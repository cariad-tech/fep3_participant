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


#include <algorithm>

#include <a_util/memory/memorybuffer.h>

#include "mapping.h"
#include "fep3/base/sample/data_sample.h"
#include "fep3/native_components/data_registry/data_signal.h"
#include "fep3/native_components/data_registry/data_registry.h"

using namespace fep3;
using namespace fep3::native;

struct RawMemoryPassPointerToListener : public IRawMemory
{
public:
    explicit RawMemoryPassPointerToListener(size_t emulated_data_size, ddl::mapping::rt::ISignalListener& signal_listener)
        : _emulated_data_size(emulated_data_size), _signal_listener(signal_listener)
    {
    }

    size_t capacity() const override
    {
        return _emulated_data_size;
    }

    const void* cdata() const override
    {
        return nullptr;
    }

    size_t size() const override
    {
        return _emulated_data_size;
    }

    size_t set(const void* data, size_t data_size)
    {
        if (isFailed(_signal_listener.onSampleReceived(data, data_size)))
        {
            throw std::runtime_error("Critical error: The simulation bus passed a nullpointer to the mapping engine!");
        }
        _emulated_data_size = data_size;
        return _emulated_data_size;
    }

    size_t resize(size_t)
    {
        return this->size();
    }

private:
    size_t _emulated_data_size{};
    ddl::mapping::rt::ISignalListener& _signal_listener;
};

// Adapter between IDataRegistry::IDataReceiver and mapping::rt::ISignalListener
class DataReceiverAdapter : public IDataRegistry::IDataReceiver
{
public:
    explicit DataReceiverAdapter(ddl::mapping::rt::ISignalListener& signal_listener)
        : _signal_listener{signal_listener}{}

    void operator()(const data_read_ptr<const IStreamType>&)
    {
        throw std::runtime_error("Critical error: The simulation bus passed a stream type to the mapping engine!");
    }

    void operator()(const data_read_ptr<const IDataSample>& sample)
    {
        RawMemoryPassPointerToListener memory_wrap(sample->getSize(), _signal_listener);
        sample->read(memory_wrap);
    }

private:
    ddl::mapping::rt::ISignalListener& _signal_listener;
};

/***************************************************************/
/* SignalMapping                                               */
/***************************************************************/

SignalMapping::SignalMapping(DataRegistry& registry) : _data_registry(registry)
{
}

SignalMapping::~SignalMapping()
{
    stopAndResetMappingEngine();
}

a_util::result::Result SignalMapping::registerMappingConfiguration(const std::string& mapping_config, const std::string& path_to_config_file, const ddl::dd::DataDefinition& ddl_description)
{
    a_util::xml::DOM dom;

    if (!path_to_config_file.empty())
    {
        if (!dom.load(path_to_config_file))
        {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_FILE, std::string("Failed to load mapping configuration file: " + dom.getLastError()).c_str());
        }
    }
    else if (!mapping_config.empty())
    {
        if (!dom.fromString(mapping_config))
        {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG, std::string("Failed to load mapping configuration: " + dom.getLastError()).c_str());
        }
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_EMPTY, "Both mapping configuration parameters are empty");
    }

    a_util::result::Result res = _config.setDD(ddl_description);
    res |= _config.loadFromDOM(dom, ddl::mapping::MapConfiguration::mc_load_mapping);
    if (isFailed(res))
    {
        RETURN_ERROR_DESCRIPTION(res.getErrorCode(), a_util::strings::join(_config.getErrorList(), "\n").c_str());
    }

    _engine.setConfiguration(_config);
    return{};
}

bool SignalMapping::checkMappingConfiguration(const std::string& target_signal_name)
{
    return static_cast<bool>(_config.getTarget(target_signal_name));
}

void SignalMapping::clearMappingConfiguration() noexcept
{
    _config.reset();
    _engine.setConfiguration(_config);
}

void SignalMapping::resetSignalDescription(const ddl::dd::DataDefinition& ddl_description) noexcept
{
    _config.setDDWithoutConsistency(ddl_description);
    _engine.setConfiguration(_config);
}

a_util::result::Result SignalMapping::registerSignal(const std::string& target_signal_name)
{
    handle_t handle{ nullptr }; // The mapping engine will assign an object to the pointer
    return _engine.Map(target_signal_name, handle);
}

a_util::result::Result SignalMapping::unregisterSignal(const std::string& target_signal_name)
{
    auto found = _mapped_signals.find(target_signal_name);
    if (found != _mapped_signals.end())
    {
        return _engine.unmap(found->second);
    }
    RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG, "Signal has not been registered");
}

a_util::result::Result SignalMapping::registerDataReceiver(std::shared_ptr<IDataRegistry::IDataReceiver> data_receiver, const std::string& target_signal_name)
{
    auto signal_handle_it = _mapped_signals.find(target_signal_name);
    if (signal_handle_it != _mapped_signals.end())
    {
        auto data_receiver_it = _data_receivers.find(signal_handle_it->second);
        if (data_receiver_it != _data_receivers.end())
        {
            RETURN_ERROR_DESCRIPTION(ERR_RESOURCE_IN_USE, "A data receiver is already registered for this target signal");
        }
        else
        {
            _data_receivers.emplace(std::make_pair(signal_handle_it->second, data_receiver));
            return{};
        }
    }
    RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG, std::string("The signal " + target_signal_name + " is not registered").c_str());
}

a_util::result::Result SignalMapping::unregisterDataReceiver(const std::string& target_signal_name)
{
    auto signal_handle_it = _mapped_signals.find(target_signal_name);
    if (signal_handle_it != _mapped_signals.end())
    {
        auto data_receiver_it = _data_receivers.find(signal_handle_it->second);
        if (data_receiver_it == _data_receivers.end())
        {
            RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "No data receiver is registered for this target signal");
        }
        else
        {
            _data_receivers.erase(data_receiver_it);
            return{};
        }
    }
    RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG, std::string("The signal " + target_signal_name + " is not registered").c_str());
}

fep3::Result SignalMapping::startMappingEngine()
{
    return _engine.start();
}

fep3::Result SignalMapping::stopAndResetMappingEngine()
{
    FEP3_RETURN_IF_FAILED(_engine.stop());
    return _engine.reset();
}

a_util::result::Result SignalMapping::registerSource(const char* source_name,
    const char* type_name, ddl::mapping::rt::ISignalListener* listener, handle_t& handle)
{
    const char* type_description; // Resource allocation done by data registry through resolveType()
    if (isFailed(resolveType(type_name, type_description)))
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "Source signal type not found in type description");
    }
    FEP3_RETURN_IF_FAILED(_data_registry.registerDataIn(source_name, base::StreamTypeDDL{ type_name, type_description } ));
    if (listener)
    {
        FEP3_RETURN_IF_FAILED(_data_registry.registerDataReceiveListener(source_name, std::make_shared<DataReceiverAdapter>(*listener)));
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Signal listener pointer is null");
    }
    handle = _data_registry.getSignalInHandle(source_name);
    return{};
}

a_util::result::Result SignalMapping::unregisterSource(handle_t handle)
{
    // The corresponding data listener will be destroyed automatically with the signal destruction
    return _data_registry.unregisterDataIn(reinterpret_cast<DataRegistry::DataSignal*>(handle)->getName());
}

a_util::result::Result SignalMapping::sendTarget(handle_t target, const void* data,
    size_t size, timestamp_t time_stamp)
{
    auto it_buffer = _sample_buffers.find(target);
    if (it_buffer != _sample_buffers.end())
    {
        std::shared_ptr<IDataSample> sample = it_buffer->second;
        base::RawMemoryRef memory{ data, size };
        sample->write(memory);
        sample->setTime(std::chrono::duration<timestamp_t>(time_stamp));

        auto data_receiver_it = _data_receivers.find(target);
        if (data_receiver_it != _data_receivers.end())
        {
            (*(data_receiver_it->second))(sample);
            return{};
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "Unable to send target data: No data receiver registered at signal mapping");
        }
    }
    RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "No preallocated sample buffer found.");
}

a_util::result::Result SignalMapping::targetMapped(const char* target_name, const char*,
    handle_t target, size_t target_size)
{
    _mapped_signals.emplace(std::make_pair(target_name, target));
    _sample_buffers.emplace(std::make_pair(target, std::make_shared<base::DataSample>(target_size, false)));
    return{};
}

a_util::result::Result SignalMapping::targetUnmapped(const char* target_name, handle_t target)
{
    _data_receivers.erase(target);
    _mapped_signals.erase(target_name);
    return{};
}

fep3::Result SignalMapping::resolveType(const char* type_name, const char*& type_description)
{
    std::string* description = {};
    auto res = _data_registry.resolveSignalType(type_name, description);
    a_util::result::isOk(res) ? type_description = description->c_str() : nullptr;
    return res;
}

timestamp_t SignalMapping::getTime() const
{
    // TODO: Get time from clock service?
    return{};
}

a_util::result::Result SignalMapping::registerPeriodicTimer(timestamp_t, ddl::mapping::rt::IPeriodicListener*)
{
    // This feature is not needed
    RETURN_ERROR_DESCRIPTION(ERR_NOT_IMPL,"Periodic Timers to send mapped signals are not supported");
}

a_util::result::Result SignalMapping::unregisterPeriodicTimer(timestamp_t, ddl::mapping::rt::IPeriodicListener*)
{
    RETURN_ERROR_DESCRIPTION(ERR_NOT_IMPL, "Periodic Timers to send mapped signals are not supported");
}
