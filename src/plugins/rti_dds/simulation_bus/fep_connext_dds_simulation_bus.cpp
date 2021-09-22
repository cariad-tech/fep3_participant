/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */

#include "fep_connext_dds_simulation_bus.h"
#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/base/binary_info/binary_info.h>
#include <fep3/base/environment_variable/environment_variable.h>
#include <fep3/base/properties/properties.h>
#include <fep3/components/configuration/configuration_service_intf.h>
#include <fep3/components/participant_info/participant_info_intf.h>
#include <fep3/components/simulation_bus/simulation_data_access.h>
#include <fep3/fep3_participant_version.h>

#include <a_util/result.h>
#include <a_util/filesystem.h>

#include <vector>
#include <cstring>
#include <regex>

#include <dds/dds.hpp>    

#include <plugins/rti_dds/simulation_bus/converter.h>
#include <plugins/rti_dds/simulation_bus/stream_item_topic/stream_item_topic.h>
#include <plugins/rti_dds/simulation_bus/bus_info/bus_info.h>

#include <plugins/rti_dds/simulation_bus/internal_topic/internal_topic.h>
#include "reader_item_queue.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

using namespace fep3;
using namespace dds::domain;
using namespace dds::core;
using namespace dds::domain::qos;

a_util::filesystem::Path getFilePath()
{
    a_util::filesystem::Path current_binary_file_path;
#ifdef WIN32
    HMODULE hModule = nullptr;
    if(GetModuleHandleEx
        (GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
        , (LPCTSTR)getFilePath
        , &hModule
        ))
    {
        std::vector<wchar_t> file_path_buffer;
        DWORD number_of_copied_characters = 0;
        // note: to support paths with length > MAX_PATH we have do trial-and-error
        // because GetModuleFileName does not indicate if the path was truncated
        while(number_of_copied_characters >= file_path_buffer.size())
        {
            file_path_buffer.resize(file_path_buffer.size() + MAX_PATH);
            number_of_copied_characters = GetModuleFileNameW(hModule, &file_path_buffer[0], static_cast<DWORD>(file_path_buffer.size()));
        }
        file_path_buffer.resize(number_of_copied_characters);
        current_binary_file_path = std::string(file_path_buffer.cbegin(), file_path_buffer.cend());
    }
#else   // WIN32
    Dl_info dl_info;
    dladdr(reinterpret_cast<void*>(getFilePath), &dl_info);
    current_binary_file_path = dl_info.dli_fname;
#endif
    return current_binary_file_path.getParent();
}

class ConnextDDSSimulationBus::Impl
{
public:
    Impl()
    {

    }

    ~Impl()
    {
        _topics.clear();
        _bus_info.reset();
       
        if (_qos_provider)
        {
            _qos_provider->extensions().unload_profiles();
        }
        if (_participant)
        {
            _participant->close();
        }
        _guard_condition = nullptr;
    }
    
    void createDataAccessCollection()
    {
        _data_access_collection = std::make_shared<base::SimulationDataAccessCollection<ReaderItemQueue>>();
    }
    
    void releaseDataAccessCollection()
    {
        _data_access_collection.reset();
    }

     std::unique_ptr<ISimulationBus::IDataReader> getReader
        (const std::string& name
        , const IStreamType& stream_type
        , size_t queue_capacity = 0
        )
    {
        auto topic = getOrCreateTopic(name, stream_type);
        return topic->createDataReader(queue_capacity, _data_access_collection);
    }

     std::unique_ptr<ISimulationBus::IDataReader> getReader
        (const std::string& name
        , size_t queue_capacity = 0
        )
    {
        auto topic = getOrCreateTopic(name, fep3::base::arya::StreamTypeRaw());
        return topic->createDataReader(queue_capacity, _data_access_collection);
    }

    std::unique_ptr<ISimulationBus::IDataWriter> getWriter
        (const std::string& name
        , const IStreamType& stream_type
        , size_t queue_capacity = 0
        )
    {
        auto topic = getOrCreateTopic(name, stream_type);
        return topic->createDataWriter(queue_capacity);
    }
    std::unique_ptr<ISimulationBus::IDataWriter> getWriter
        (const std::string& name
        , size_t queue_capacity = 0
        )
    {
        auto topic = getOrCreateTopic(name, fep3::base::arya::StreamTypeRaw());
        return topic->createDataWriter(queue_capacity);
    }

    std::shared_ptr<QosProvider> LoadQosProfile()
    {
        auto qos_file_beside_the_binary = getFilePath().append("USER_QOS_PROFILES.xml");
        auto qos_file_beside_the_binary_clean = std::regex_replace(qos_file_beside_the_binary.toString(), std::regex("\\\\"), "/");

        auto qos_libraries = QosProvider::Default().extensions().qos_profile_libraries();
        // If we have already found the fep3 qos library we use it
        if (std::find(qos_libraries.begin(), qos_libraries.end(), "fep3") != qos_libraries.end())
        {
            _qos_provider = std::make_shared<QosProvider>(dds::core::QosProvider::Default());
        }
        // If not we search beside the simulation bus binary 
        else if (a_util::filesystem::exists(qos_file_beside_the_binary_clean))
        {
            _qos_provider = std::make_shared<QosProvider>(qos_file_beside_the_binary_clean);
        }
        else
        {
            _qos_provider = std::make_shared<QosProvider>(dds::core::QosProvider::Default());
        }
        return _qos_provider;
    }
    
    void initBusInfo(DomainParticipantQos &participant_qos, const std::string participant_name)
    {
        // Create BusInfo to collect for bus informations
        _bus_info = std::make_unique<BusInfo>();
        _bus_info->getOwnParticipantInfo()->setParticipantName(participant_name);

        BusInfo::Version version;
        version.major = FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR;
        version.minor = FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR;
        version.patch = FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH;

        _bus_info->getOwnParticipantInfo()->setFepVersion(version);
        _bus_info->registerUserData(participant_qos);

        // Create built in topic to make bus informations available via ISimulationBus
        auto built_in_topic_businfo = std::make_shared<InternalTopic>("_built_in_topic_businfo");
        _topics["_built_in_topic_businfo"] = built_in_topic_businfo;
        _bus_info->setUpdateCallback([this, built_in_topic_businfo]()
        {
            if (_bus_info)
            {
                built_in_topic_businfo->write(_bus_info->asJson());
            }
        });
    }

    void startBlockingReception(const std::function<void()>& reception_preparation_done_callback)
    {
        // RAII ensuring invocation of callback exactly once (even in case exception is thrown)
        std::unique_ptr<bool, std::function<void(bool*)>> reception_preparation_done_callback_caller
            (nullptr, [reception_preparation_done_callback](bool* p)
            {
                reception_preparation_done_callback();
                delete p;
            });
        if(reception_preparation_done_callback)
        {
            reception_preparation_done_callback_caller.reset(new bool);
        }
        
        if(_data_access_collection)
        {
            const auto& data_access_collection = *_data_access_collection.get();
            {  
                size_t last_size_of_data_access_collection = 0;
                
                dds::core::cond::WaitSet waitset = nullptr;

                // Run until ::stop was called
                _receiving = true;
                while (_receiving)
                {
                    // Recreate WaitSet if:
                    // * waitset was never created
                    // * a new reader was created (_data_access_collection->size() changed)
                    // * a reader recreated internal dds reader
                    if (last_size_of_data_access_collection != data_access_collection.size() ||
                        waitset.is_nil())
                    {
                        waitset = dds::core::cond::WaitSet();
                        for
                            (auto data_access_iterator = data_access_collection.cbegin()
                                ; data_access_iterator != data_access_collection.cend()
                                ; ++data_access_iterator
                                )
                        {
                            waitset += data_access_iterator->_item_queue->createSampleReadCondition(data_access_iterator->_receiver);
                            waitset += data_access_iterator->_item_queue->createStreamTypeReadCondition(data_access_iterator->_receiver);

                            data_access_iterator->_item_queue->setRecreateWaitSetCondition([&waitset]()
                            {
                                waitset = nullptr;
                            });
                        }
                        waitset += _guard_condition;

                        _guard_condition->trigger_value(false);

                        // the Simulation Bus is now prepared for the reception of data and for a call to stopBlockingReception
                        reception_preparation_done_callback_caller.reset();

                        last_size_of_data_access_collection = _data_access_collection->size();
                    }

                    try 
                    {
                        // Block until one condition was emited:
                        // * ReadCondition for sample or stream
                        // * _guard_condition
                        // * or timeout 100ms
                        dds::core::cond::WaitSet::ConditionSeq conditions = waitset.wait(Duration(0, 100000000));
                        for (dds::core::cond::Condition const & condition : conditions)
                        {
                            if (condition != _guard_condition)
                            {
                                condition.delegate()->dispatch();
                            }
                        }
                    }
                    catch (const std::exception& exception) 
                    {
                        if (_logger)
                        {
                            if (_logger->isWarningEnabled())
                            {
                                _logger->logWarning(a_util::strings::format(
                                                        "Caught RTI DDS exception during reception of data: '%s'",
                                                        exception.what()));
                            }
                        }
                    }
                }                    
                return;
            }
            
        }
    }
    
    void stopBlockingReception()
    {
        _receiving = false;
        _guard_condition->trigger_value(true);
    }

private:
    std::shared_ptr<ITopic> getOrCreateTopic(const std::string & topic_name, const IStreamType& stream_type)
    {
        auto entity = _topics.find(topic_name);
        if (entity != _topics.end())
        {
            //@TODO Check IStreamType
            return entity->second;
        }

        if(!_participant)
        {
            throw std::runtime_error("RTI DDS Participant pointer empty. Simulation Bus not initialized.");
        }

        auto topic = std::make_shared<StreamItemTopic>(*_participant, topic_name, stream_type, _qos_provider, _logger);
        _topics[topic_name] = topic;
        return topic;
    }

public:
    std::unique_ptr<DomainParticipant> _participant;
    std::map<std::string, std::shared_ptr<ITopic>> _topics;
    std::shared_ptr<dds::core::QosProvider> _qos_provider;
    std::shared_ptr<fep3::ILogger> _logger;

    std::unique_ptr<BusInfo> _bus_info;
    IParticipantInfo* _participant_info;
    
private:
    // We need a sub object for collection data access (data triggered behavior)
    // because the simulation bus cannot be passed to the reader, as lifetime of simulation bus
    // cannot be controlled.
    std::shared_ptr<base::SimulationDataAccessCollection<ReaderItemQueue>> _data_access_collection;
    dds::core::cond::GuardCondition _guard_condition;
    std::atomic<bool> _receiving = { false };
};

ConnextDDSSimulationBus::ConnextDDSSimulationBus() : _impl(std::make_unique<ConnextDDSSimulationBus::Impl>())
{

}

ConnextDDSSimulationBus::~ConnextDDSSimulationBus()
{
}

fep3::Result ConnextDDSSimulationBus::create()
{
    std::shared_ptr<const IComponents> components = _components.lock();
    if (components)
    {
        auto logging_service = components->getComponent<ILoggingService>();
        if (logging_service)
        {
            _impl->_logger = logging_service->createLogger("connext_dds_simulation_bus.component");
        }

        auto configuration_service = components->getComponent<IConfigurationService>();
        if (configuration_service)
        {
            _simulation_bus_configuration.initConfiguration(*configuration_service);
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "Can not get configuration service interface");
        }

        auto participant_info = components->getComponent<IParticipantInfo>();
        if (participant_info)
        {
            _impl->_participant_info = participant_info;
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "Can not get participant info interface");
        }
    }
    return {};
}

fep3::Result ConnextDDSSimulationBus::destroy()
{
    _simulation_bus_configuration.deinitConfiguration();
    return {};
}

fep3::Result ConnextDDSSimulationBus::initialize()
{
    auto qos_libraries = _impl->LoadQosProfile()->extensions().qos_profile_libraries();
    if (std::find(qos_libraries.begin(), qos_libraries.end(), "fep3") == qos_libraries.end())
    {
        RETURN_ERROR_DESCRIPTION(fep3::ERR_NOT_FOUND, "Could not find fep3 library in USER_QOS_PROFILES.xml. \n"
            "Please make sure your application has access to the predefined USER_QOS_PROFILES.xml from fep3. \n"
            "See documentation for more information");
    }
    _simulation_bus_configuration.updatePropertyVariables();
    uint32_t domain_id = _simulation_bus_configuration._participant_domain;
    
    auto participant_qos = _impl->_qos_provider->participant_qos("fep3::participant");
    
    auto participant_name = _impl->_participant_info->getName();
    if (participant_name.empty())
    {
        RETURN_ERROR_DESCRIPTION(fep3::ERR_NOT_SUPPORTED, "Participant name from participant info is empty");
    }
    auto system_name = _impl->_participant_info->getSystemName();
    if (system_name.empty())
    {
        RETURN_ERROR_DESCRIPTION(fep3::ERR_NOT_SUPPORTED, "Participant name from participant info is empty");
    }
    if (system_name.size() >= 255)
    {
        RETURN_ERROR_DESCRIPTION(fep3::ERR_NOT_SUPPORTED, "RTI DDS doesn't support system name longer than 255 byte");
    }
    participant_qos.extensions().property.set({ "dds.domain_participant.domain_tag", system_name });

    _impl->initBusInfo(participant_qos, participant_name);

#ifdef WIN32
    //in windows the rtimonitoring is loaded lazy ... so we need to change working dir here for
    //creating time
    auto orig_wd = a_util::filesystem::getWorkingDirectory();
    auto res = a_util::filesystem::setWorkingDirectory(getFilePath());
    if (isFailed(res))
    {
        orig_wd = getFilePath();
    }
#endif
    try
    {
        _impl->_participant = std::make_unique<DomainParticipant>
            (domain_id
             , participant_qos);
        _impl->_bus_info->registerParticipant(*_impl->_participant);
    }
    catch (const std::exception& ex)
    {
#ifdef WIN32
        a_util::filesystem::setWorkingDirectory(orig_wd);
#endif
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, ex.what());
    }
    
#ifdef WIN32
    a_util::filesystem::setWorkingDirectory(orig_wd);
#endif
    
    _impl->createDataAccessCollection();
    
    return {};
}

fep3::Result ConnextDDSSimulationBus::deinitialize()
{
    _impl->releaseDataAccessCollection();
    
    _impl->_bus_info->unregisterParticipant(*_impl->_participant);
    _impl->_bus_info = nullptr;

    _impl->_topics.clear();
    if (_impl->_qos_provider)
    {
        _impl->_qos_provider->extensions().unload_profiles();
    }
    
    if (_impl->_participant)
    {
        _impl->_participant->close();
    }

    _impl->_participant.reset();

    return {};
}

bool ConnextDDSSimulationBus::isSupported(const IStreamType& stream_type) const
{
    using namespace fep3::base::arya;
    return
        (meta_type_raw == stream_type
        || (meta_type_audio == stream_type)
        || (meta_type_ddl == stream_type)
        || (meta_type_plain == stream_type)
        || (meta_type_string == stream_type)
        || (meta_type_plain == stream_type)
        );
}

std::unique_ptr<ISimulationBus::IDataReader> ConnextDDSSimulationBus::getReader
    (const std::string& name
    , const IStreamType& stream_type
)
{
    try
    {
        return _impl->getReader(name, stream_type);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}

std::unique_ptr<ISimulationBus::IDataReader> ConnextDDSSimulationBus::getReader
    (const std::string& name
    , const IStreamType& stream_type
    , size_t queue_capacity
)
{
    try
    {
        return _impl->getReader(name, stream_type, queue_capacity);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    catch (std::exception& exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}

std::unique_ptr<ISimulationBus::IDataReader> ConnextDDSSimulationBus::getReader(const std::string& name)
{
    try
    {
        return _impl->getReader(name);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}
std::unique_ptr<ISimulationBus::IDataReader> ConnextDDSSimulationBus::getReader(const std::string& name, size_t queue_capacity)
{
    try
    {
        return _impl->getReader(name, queue_capacity);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}
std::unique_ptr<ISimulationBus::IDataWriter> ConnextDDSSimulationBus::getWriter
    (const std::string& name
    , const IStreamType& stream_type
)
{
    try
    {
        return _impl->getWriter(name, stream_type, 0);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}
std::unique_ptr<ISimulationBus::IDataWriter> ConnextDDSSimulationBus::getWriter
    (const std::string& name
    , const IStreamType& stream_type
    , size_t queue_capacity
)
{
    try
    {
        return _impl->getWriter(name, stream_type, queue_capacity);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}
std::unique_ptr<ISimulationBus::IDataWriter> ConnextDDSSimulationBus::getWriter(const std::string& name)
{
    try
    {
        return _impl->getWriter(name);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}
std::unique_ptr<ISimulationBus::IDataWriter> ConnextDDSSimulationBus::getWriter(const std::string& name, size_t queue_capacity)
{
    try
    {
        auto topic = _impl->getWriter(name, queue_capacity);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}
void ConnextDDSSimulationBus::startBlockingReception(const std::function<void()>& reception_preparation_done_callback)
{
    try
    {
        _impl->startBlockingReception(reception_preparation_done_callback);
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
}

void ConnextDDSSimulationBus::stopBlockingReception()
{
    _impl->stopBlockingReception();
}

std::shared_ptr<dds::core::QosProvider> ConnextDDSSimulationBus::getQOSProfile() const
{
    return _impl->_qos_provider;
}

void ConnextDDSSimulationBus::logError(const fep3::Result& res)
{
    if (_impl->_logger)
    {
        if (_impl->_logger->isErrorEnabled())
        {
            _impl->_logger->logError(a_util::result::toString(res));
        }
    }
}

ConnextDDSSimulationBus::ConnextDDSSimulationBusConfiguration::ConnextDDSSimulationBusConfiguration()
    : Configuration("rti_dds_simulation_bus")
{
}

fep3::Result ConnextDDSSimulationBus::ConnextDDSSimulationBusConfiguration::registerPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_participant_domain, "participant_domain"));
    
    return {};
}

fep3::Result ConnextDDSSimulationBus::ConnextDDSSimulationBusConfiguration::unregisterPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_participant_domain, "participant_domain"));
    
    return {};
}
