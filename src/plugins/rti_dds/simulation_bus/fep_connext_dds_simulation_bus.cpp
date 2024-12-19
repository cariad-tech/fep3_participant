/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "fep_connext_dds_simulation_bus.h"

#include "bus_info/bus_info.h"
#include "converter.h"
#include "internal_topic/internal_topic.h"
#include "stream_item_topic/stream_item_topic.h"

#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/components/participant_info/participant_info_intf.h>
#include <fep3/fep3_filesystem.h>
#include <fep3/fep3_participant_version.h>

#include <a_util/result.h>
#include <a_util/system/address_info.h>

#include <rti/core/cond/AsyncWaitSet.hpp>

#ifndef WIN32
    #include <dlfcn.h>
#endif // !WIN32

using namespace fep3;
using namespace dds::domain;
using namespace dds::core;
using namespace dds::domain::qos;

const int ConnextDDSSimulationBus::_address_info = 0;

class ConnextDDSSimulationBus::Impl {
    using DataAccessColletion = base::SimulationDataAccessCollection<ReaderItemQueue>;
    using DataAccessColletionPtr = std::shared_ptr<DataAccessColletion>;

public:
    Impl(ConnextDDSSimulationBusConfiguration& config) : _configuration(config)
    {
    }

    ~Impl()
    {
        _topics.clear();
        _bus_info.reset();

        if (_qos_provider) {
            _qos_provider->extensions().unload_profiles();
        }
        if (_participant) {
            _participant->close();
        }
        _waitset_guard = nullptr;
    }

    void createDataAccessCollection()
    {
        _data_access_collection =
            std::make_shared<base::SimulationDataAccessCollection<ReaderItemQueue>>();
    }

    void releaseDataAccessCollection()
    {
        _data_access_collection.reset();
    }

    std::unique_ptr<ISimulationBus::IDataReader> getReader(const std::string& name,
                                                           const IStreamType& stream_type,
                                                           size_t queue_capacity,
                                                           std::chrono::nanoseconds timeout)
    {
        std::shared_ptr<ITopic> topic;
        bool is_new_topic;
        std::tie(topic, is_new_topic) = getOrCreateTopic(name, stream_type);
        auto data_reader = topic->createDataReader(queue_capacity, _data_access_collection);

        // unlock guard, update waitset
        _waitset_guard.trigger_value(true);

        if (is_new_topic && timeout.count() > 0ll) {
            auto stream_item_topic = dynamic_cast<StreamItemTopic*>(topic.get());
            if (stream_item_topic) {
                bool connected = stream_item_topic->waitForConnectingWriters(timeout);
                if (!connected) {
                    if (_logger) {
                        if (_logger->isErrorEnabled()) {
                            _logger->logError(a_util::strings::format(
                                "Not enough writers connected to reader %s", name.c_str()));
                        }
                    }
                    return {};
                }
            }
        }
        return data_reader;
    }

    std::unique_ptr<ISimulationBus::IDataWriter> getWriter(const std::string& name,
                                                           const IStreamType& stream_type,
                                                           size_t queue_capacity = 0)
    {
        auto topic = getOrCreateTopic(name, stream_type).first;
        return topic->createDataWriter(queue_capacity);
    }
    std::unique_ptr<ISimulationBus::IDataWriter> getWriter(const std::string& name,
                                                           size_t queue_capacity = 0)
    {
        auto topic = getOrCreateTopic(name, fep3::base::StreamTypeRaw()).first;
        return topic->createDataWriter(queue_capacity);
    }

    std::shared_ptr<QosProvider> LoadQosProfile()
    {
        const auto address_info =
            a_util::system::AddressInfo(ConnextDDSSimulationBus::_address_info);
        auto qos_file_beside_the_binary =
            address_info.getFilePath().getParent().append("USER_QOS_PROFILES.xml");

        auto qos_file_beside_the_binary_clean =
            std::regex_replace(qos_file_beside_the_binary.toString(), std::regex("\\\\"), "/");

        auto qos_libraries = QosProvider::Default().extensions().qos_profile_libraries();
        // If we have already found the fep3 qos library we use it
        if (std::find(qos_libraries.begin(), qos_libraries.end(), "fep3") != qos_libraries.end()) {
            _qos_provider = std::make_shared<QosProvider>(dds::core::QosProvider::Default());
        }
        // If not we search beside the simulation bus binary
        else if (fs::exists(qos_file_beside_the_binary_clean)) {
            _qos_provider = std::make_shared<QosProvider>(qos_file_beside_the_binary_clean);
        }
        else {
            _qos_provider = std::make_shared<QosProvider>(dds::core::QosProvider::Default());
        }
        return _qos_provider;
    }

    void initBusInfo(DomainParticipantQos& participant_qos, const std::string participant_name)
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
        _bus_info->setUpdateCallback([this, built_in_topic_businfo]() {
            if (_bus_info) {
                built_in_topic_businfo->write(_bus_info->asJson());
            }
        });
    }

    void startBlockingReception(const std::function<void()>& reception_preparation_done_callback)
    {
        // RAII ensuring invocation of callback exactly once (even in case exception is thrown)
        std::unique_ptr<bool, std::function<void(bool*)>>
            reception_preparation_done_callback_caller(
                nullptr, [reception_preparation_done_callback](bool* p) {
                    reception_preparation_done_callback();
                    delete p;
                });
        if (reception_preparation_done_callback) {
            reception_preparation_done_callback_caller.reset(new bool);
        }

        if (_data_access_collection) {
            // Run until ::stopBlockingReception was called
            runReceptionWaitSet(std::move(reception_preparation_done_callback_caller));
        }
    }

    void stopBlockingReception()
    {
        std::lock_guard<std::mutex> lk(_waitset_mtx);
        _receiving = false;
        _waitset_guard->trigger_value(true);
    }

private:
    /**
     * @brief For a given topic name creates or retrieves an already existing topic object.
     * Returns the object along with a boolean signalling if the topic is newly created.
     */
    std::pair<std::shared_ptr<ITopic>, bool> getOrCreateTopic(const std::string& topic_name,
                                                              const IStreamType& stream_type)
    {
        auto entity = _topics.find(topic_name);
        if (entity != _topics.end()) {
            //@TODO Check IStreamType
            return std::make_pair(entity->second, false);
        }

        if (!_participant) {
            throw std::runtime_error(
                "RTI DDS Participant pointer empty. Simulation Bus not initialized.");
        }

        auto topic = std::make_shared<StreamItemTopic>(
            *_participant, topic_name, stream_type, _qos_provider, _logger);
        _topics[topic_name] = topic;
        return std::make_pair(topic, true);
    }

    void createWaitSet(rti::core::cond::AsyncWaitSet& waitset)
    {
        waitset =
            rti::core::cond::AsyncWaitSet(rti::core::cond::AsyncWaitSetProperty().thread_pool_size(
                _configuration._async_waitset_threads));
    };

    void createWaitSet(dds::core::cond::WaitSet& waitset)
    {
        waitset = dds::core::cond::WaitSet();
    };

    void updateWaitSet(rti::core::cond::AsyncWaitSet& waitset,
                       DataAccessColletion::const_iterator it)
    {
        // For AsyncWaitSet the reader have to be recreated here not in the dispatching thread,
        // When the StreamType is dynamically sent to reader.
        it->_item_queue->createReader();
        waitset += it->_item_queue->createSampleReadCondition(it->_receiver);
        waitset += it->_item_queue->createStreamTypeReadCondition(it->_receiver);

        it->_item_queue->setRecreateWaitSetCondition(
            [this]() { _waitset_guard->trigger_value(true); });
    };

    void updateWaitSet(dds::core::cond::WaitSet& waitset, DataAccessColletion::const_iterator it)
    {
        waitset += it->_item_queue->createSampleReadCondition(it->_receiver);
        waitset += it->_item_queue->createStreamTypeReadCondition(it->_receiver);

        it->_item_queue->setRecreateWaitSetCondition([this, &waitset]() { waitset = nullptr; });
    };

    template <typename WaitSetType>
    void setUpReceptionWaitSet(size_t& last_size_of_data_access_collection,
                               WaitSetType& waitset,
                               std::unique_ptr<bool, std::function<void(bool*)>> on_setup_callback)
    {
        std::lock_guard<std::mutex> lk(_waitset_mtx);
        // Recreate WaitSet if:
        // * waitset was never created
        // * a new reader was created (_data_access_collection->size() changed)
        // * a reader recreated internal dds reader
        const auto& data_access_collection = *_data_access_collection.get();
        if (last_size_of_data_access_collection != data_access_collection.size() ||
            waitset.is_nil()) {
            createWaitSet(waitset);
            for (auto data_access_iterator = data_access_collection.cbegin();
                 data_access_iterator != data_access_collection.cend();
                 ++data_access_iterator) {
                updateWaitSet(waitset, data_access_iterator);
            }

            waitset += _waitset_guard;
            _waitset_guard->trigger_value(false);

            // the Simulation Bus is now prepared for the reception of data and for a call to
            // stopBlockingReception
            last_size_of_data_access_collection = _data_access_collection->size();
            on_setup_callback.reset();
        }
    }

    void runReceptionWaitSet(std::unique_ptr<bool, std::function<void(bool*)>> on_setup_callback)
    {
        _configuration.updatePropertyVariables();

        _receiving = true;
        size_t last_size_of_data_access_collection = 0;

        if (_configuration._use_async_waitset) {
            // Async WaitSet for receiving data
            rti::core::cond::AsyncWaitSet async_waitset = nullptr;

            // This WaitSet is only to guard the AsyncWaitSet thread pool
            dds::core::cond::WaitSet waitset;

            // Attach guard condition, we can use it to trigger the wake up
            waitset += _waitset_guard;

            while (_receiving) {
                // Setup WaitSet
                setUpReceptionWaitSet(last_size_of_data_access_collection,
                                      async_waitset,
                                      std::move(on_setup_callback));
                // Start reception
                try {
                    // Start async thread pool to handle the received data
                    async_waitset.start();
                    // Block until _waitset_guard turns true
                    waitset.wait();
                    // Stop the async thread pool, it will be updated in next cycle
                    // Exit if _receiving is false
                    async_waitset.stop();
                    async_waitset = nullptr;
                }
                catch (const std::exception& exception) {
                    if (_logger && _logger->isWarningEnabled()) {
                        _logger->logWarning(a_util::strings::format(
                            "Caught RTI DDS exception during reception of data: '%s'",
                            exception.what()));
                    }
                }
            }
        }
        else {
            // WaitSet for receiving data
            dds::core::cond::WaitSet waitset = nullptr;

            while (_receiving) {
                // Setup WaitSet
                setUpReceptionWaitSet(
                    last_size_of_data_access_collection, waitset, std::move(on_setup_callback));
                // Start reception
                try {
                    // Block until one condition was emited:
                    // * ReadCondition for sample or stream
                    // * _waitset_guard turns to true
                    // * or timeout 100ms
                    auto conditions = waitset.wait(Duration(0, 100000000));
                    for (auto const& condition: conditions) {
                        if (condition != _waitset_guard) {
                            condition.delegate()->dispatch();
                        }
                        else {
                            _waitset_guard.trigger_value(false);
                        }
                    }
                }
                catch (const std::exception& exception) {
                    if (_logger && _logger->isWarningEnabled()) {
                        _logger->logWarning(a_util::strings::format(
                            "Caught RTI DDS exception during reception of data: '%s'",
                            exception.what()));
                    }
                }
            }
        }
    }

public:
    std::unique_ptr<DomainParticipant> _participant;
    std::map<std::string, std::shared_ptr<ITopic>> _topics;
    std::shared_ptr<dds::core::QosProvider> _qos_provider;
    std::shared_ptr<fep3::ILogger> _logger;

    std::unique_ptr<BusInfo> _bus_info;
    IParticipantInfo* _participant_info;

private:
    ConnextDDSSimulationBusConfiguration& _configuration;

    // We need a sub object for collection data access (data triggered behavior)
    // because the simulation bus cannot be passed to the reader, as lifetime of simulation bus
    // cannot be controlled.
    DataAccessColletionPtr _data_access_collection;
    // this condition guard the waitset
    // if set true, the waitset will update in the next cycle, based on the change of the
    // _data_access_collection
    dds::core::cond::GuardCondition _waitset_guard;
    std::atomic<bool> _receiving = {false};
    std::mutex _waitset_mtx;
};

ConnextDDSSimulationBus::ConnextDDSSimulationBus()
    : _impl(std::make_unique<ConnextDDSSimulationBus::Impl>(_simulation_bus_configuration))
{
}

ConnextDDSSimulationBus::~ConnextDDSSimulationBus()
{
}

fep3::Result ConnextDDSSimulationBus::create()
{
    std::shared_ptr<const IComponents> components = _components.lock();
    if (components) {
        auto logging_service = components->getComponent<ILoggingService>();
        if (logging_service) {
            _impl->_logger = logging_service->createLogger("connext_dds_simulation_bus.component");
        }

        auto configuration_service = components->getComponent<IConfigurationService>();
        if (configuration_service) {
            _simulation_bus_configuration.initConfiguration(*configuration_service);
        }
        else {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE,
                                     "Can not get configuration service interface");
        }

        auto participant_info = components->getComponent<IParticipantInfo>();
        if (participant_info) {
            _impl->_participant_info = participant_info;
        }
        else {
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
    // the mutex ensures that qos profile is not read while the working dir is changed and
    // ensures there are not parallel calls to the non thread safe constructor of DomainParticipant
    static std::mutex domain_participant_constructor_mutex;
    dds::core::StringSeq qos_libraries;
    {
        std::unique_lock<std::mutex> constrLock(domain_participant_constructor_mutex);
        qos_libraries = _impl->LoadQosProfile()->extensions().qos_profile_libraries();
    }

    if (std::find(qos_libraries.begin(), qos_libraries.end(), "fep3") == qos_libraries.end()) {
        RETURN_ERROR_DESCRIPTION(fep3::ERR_NOT_FOUND,
                                 "Could not find fep3 library in USER_QOS_PROFILES.xml. \n"
                                 "Please make sure your application has access to the predefined "
                                 "USER_QOS_PROFILES.xml from fep3. \n"
                                 "See documentation for more information");
    }
    _simulation_bus_configuration.updatePropertyVariables();
    uint32_t domain_id = _simulation_bus_configuration._participant_domain;

    auto participant_qos = _impl->_qos_provider->participant_qos("fep3::participant");

    auto participant_name = _impl->_participant_info->getName();
    if (participant_name.empty()) {
        RETURN_ERROR_DESCRIPTION(fep3::ERR_NOT_SUPPORTED,
                                 "Participant name from FEP Component '%s' is empty",
                                 _impl->_participant_info->getComponentIID());
    }
    auto system_name = _impl->_participant_info->getSystemName();
    if (system_name.empty()) {
        RETURN_ERROR_DESCRIPTION(fep3::ERR_NOT_SUPPORTED,
                                 "System name from FEP Component '%s' is empty",
                                 _impl->_participant_info->getComponentIID());
    }
    if (system_name.size() >= 255) {
        RETURN_ERROR_DESCRIPTION(fep3::ERR_NOT_SUPPORTED,
                                 "RTI DDS doesn't support system name longer than 255 byte");
    }
    participant_qos.extensions().property.set({"dds.domain_participant.domain_tag", system_name});
    participant_qos.extensions().participant_name.name(participant_name);

    _impl->initBusInfo(participant_qos, participant_name);

    {
        // call DomainParticipant thread safe and not LoadQosProfile while changing the working
        // directories
        std::unique_lock<std::mutex> constr_lock(domain_participant_constructor_mutex);

#ifdef WIN32
        // in windows the rtimonitoring is loaded lazy ... so we need to change working dir here for
        // creating time
        auto orig_wd = fs::current_path();

        const auto address_info =
            a_util::system::AddressInfo(ConnextDDSSimulationBus::_address_info);
        auto wd = fs::path{address_info.getFilePath().getParent().toString()};

        try {
            fs::current_path(wd);
        }
        catch (fs::filesystem_error const&) {
            orig_wd = wd;
        }
#endif
        try {
            _impl->_participant = std::make_unique<DomainParticipant>(domain_id, participant_qos);
            _impl->_bus_info->registerParticipant(*_impl->_participant);
        }
        catch (const std::exception& ex) {
#ifdef WIN32
            fs::current_path(orig_wd);
#endif
            RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, ex.what());
        }

#ifdef WIN32
        fs::current_path(orig_wd);
#endif
    }
    _impl->createDataAccessCollection();

    return {};
}

fep3::Result ConnextDDSSimulationBus::deinitialize()
{
    _impl->releaseDataAccessCollection();

    _impl->_bus_info->unregisterParticipant(*_impl->_participant);
    _impl->_bus_info = nullptr;

    _impl->_topics.clear();
    if (_impl->_qos_provider) {
        _impl->_qos_provider->extensions().unload_profiles();
    }

    if (_impl->_participant) {
        _impl->_participant->close();
    }

    _impl->_participant.reset();

    return {};
}

bool ConnextDDSSimulationBus::isSupported(const IStreamType& stream_type) const
{
    using namespace fep3::base::arya;
    return (meta_type_raw == stream_type || (meta_type_audio == stream_type) ||
            (meta_type_ddl == stream_type) || (meta_type_plain == stream_type) ||
            (meta_type_string == stream_type));
}

std::unique_ptr<ISimulationBus::IDataReader> ConnextDDSSimulationBus::getReader(
    const std::string& name, const IStreamType& stream_type, size_t queue_capacity)
{
    try {
        _simulation_bus_configuration.updatePropertyVariables();
        int64_t timeout = _simulation_bus_configuration._datawriter_ready_timeout;
        if (timeout < 0ll) {
            if (_impl->_logger) {
                if (_impl->_logger->isWarningEnabled()) {
                    _impl->_logger->logWarning(
                        a_util::strings::format("Negative timeout value (%lld), disabling the "
                                                "waiting for connecting writers",
                                                timeout));
                }
            }

            timeout = 0ll;
        }
        if (timeout != 0ll) {
            const std::vector<std::string>& must_be_ready_signals =
                _simulation_bus_configuration._must_be_ready_signals;
            if (must_be_ready_signals.size() != 1u || must_be_ready_signals.front() != "*") {
                if (std::find(must_be_ready_signals.cbegin(), must_be_ready_signals.cend(), name) ==
                    must_be_ready_signals.cend()) {
                    timeout = 0ll;
                }
            }
        }

        return _impl->getReader(
            name, stream_type, queue_capacity, std::chrono::nanoseconds(timeout));
    }
    catch (Exception& exception) {
        logError(convertExceptionToResult(exception));
    }
    catch (std::exception& exception) {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}

std::unique_ptr<ISimulationBus::IDataReader> ConnextDDSSimulationBus::getReader(
    const std::string& name)
{
    return getReader(name, fep3::base::StreamTypeRaw(), 0u);
}

std::unique_ptr<ISimulationBus::IDataReader> ConnextDDSSimulationBus::getReader(
    const std::string& name, size_t queue_capacity)
{
    return getReader(name, fep3::base::StreamTypeRaw(), queue_capacity);
}

std::unique_ptr<ISimulationBus::IDataReader> ConnextDDSSimulationBus::getReader(
    const std::string& name, const IStreamType& stream_type)
{
    return getReader(name, stream_type, 0u);
}

std::unique_ptr<ISimulationBus::IDataWriter> ConnextDDSSimulationBus::getWriter(
    const std::string& name, const IStreamType& stream_type)
{
    try {
        return _impl->getWriter(name, stream_type, 0);
    }
    catch (Exception& exception) {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}
std::unique_ptr<ISimulationBus::IDataWriter> ConnextDDSSimulationBus::getWriter(
    const std::string& name, const IStreamType& stream_type, size_t queue_capacity)
{
    try {
        return _impl->getWriter(name, stream_type, queue_capacity);
    }
    catch (Exception& exception) {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}
std::unique_ptr<ISimulationBus::IDataWriter> ConnextDDSSimulationBus::getWriter(
    const std::string& name)
{
    try {
        return _impl->getWriter(name);
    }
    catch (Exception& exception) {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}
std::unique_ptr<ISimulationBus::IDataWriter> ConnextDDSSimulationBus::getWriter(
    const std::string& name, size_t queue_capacity)
{
    try {
        auto topic = _impl->getWriter(name, queue_capacity);
    }
    catch (Exception& exception) {
        logError(convertExceptionToResult(exception));
    }
    return nullptr;
}
void ConnextDDSSimulationBus::startBlockingReception(
    const std::function<void()>& reception_preparation_done_callback)
{
    try {
        _impl->startBlockingReception(reception_preparation_done_callback);
    }
    catch (Exception& exception) {
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
    if (_impl->_logger) {
        if (_impl->_logger->isErrorEnabled()) {
            _impl->_logger->logError(a_util::result::toString(res));
        }
    }
}

ConnextDDSSimulationBus::ConnextDDSSimulationBusConfiguration::
    ConnextDDSSimulationBusConfiguration()
    : Configuration(FEP3_RTI_DDS_SIMBUS_CONFIG)
{
}

fep3::Result ConnextDDSSimulationBus::ConnextDDSSimulationBusConfiguration::
    registerPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(
        registerPropertyVariable(_participant_domain, FEP3_SIMBUS_PARTICIPANT_DOMAIN_PROPERTY));
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_datawriter_ready_timeout,
                                                   FEP3_SIMBUS_DATAWRITER_READY_TIMEOUT_PROPERTY));
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_must_be_ready_signals,
                                                   FEP3_SIMBUS_MUST_BE_READY_SIGNALS_PROPERTY));
    FEP3_RETURN_IF_FAILED(
        registerPropertyVariable(_use_async_waitset, FEP3_RTI_DDS_SIMBUS_ASYNC_WAITSET_PROPERTY));
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(
        _async_waitset_threads, FEP3_RTI_DDS_SIMBUS_ASYNC_WAITSET_THREADS_PROPERTY));
    return {};
}

fep3::Result ConnextDDSSimulationBus::ConnextDDSSimulationBusConfiguration::
    unregisterPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_must_be_ready_signals,
                                                     FEP3_SIMBUS_MUST_BE_READY_SIGNALS_PROPERTY));
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(
        _datawriter_ready_timeout, FEP3_SIMBUS_DATAWRITER_READY_TIMEOUT_PROPERTY));
    FEP3_RETURN_IF_FAILED(
        unregisterPropertyVariable(_participant_domain, FEP3_SIMBUS_PARTICIPANT_DOMAIN_PROPERTY));
    FEP3_RETURN_IF_FAILED(
        unregisterPropertyVariable(_use_async_waitset, FEP3_RTI_DDS_SIMBUS_ASYNC_WAITSET_PROPERTY));
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(
        _async_waitset_threads, FEP3_RTI_DDS_SIMBUS_ASYNC_WAITSET_THREADS_PROPERTY));

    return {};
}
