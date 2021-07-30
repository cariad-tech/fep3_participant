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


#include <future>

#include "data_registry.h"

#include <a_util/strings.h>
#include <a_util/filesystem/filesystem.h>

#include "data_io.h"
#include "data_signal.h"
#include "fep3/fep3_errors.h"
#include "fep3/components/service_bus/service_bus_intf.h"
#include <fep3/components/configuration/configuration_service_intf.h>

using namespace fep3;
using namespace fep3::native;

std::string RPCDataRegistryService::getSignalInNames()
{
    return a_util::strings::join(_data_registry.getSignalInNames(), ",");
}

std::string RPCDataRegistryService::getSignalOutNames()
{
    return a_util::strings::join(_data_registry.getSignalOutNames(), ",");
}

Json::Value RPCDataRegistryService::getStreamType(const std::string& signal_name)
{
    base::StreamType stream_type = _data_registry.getStreamType(signal_name);
    Json::Value value;
    value["meta_type"] = stream_type.getMetaTypeName();
    value["properties"]["names"] = a_util::strings::join(stream_type.getPropertyNames(), ",");
    value["properties"]["values"] = a_util::strings::join(stream_type.getPropertyValues(), ",");
    value["properties"]["types"] = a_util::strings::join(stream_type.getPropertyTypes(), ",");
    return value;
}

DataRegistryConfiguration::DataRegistryConfiguration(DataSignalRenaming& data_signal_renaming)
    : Configuration(FEP3_DATA_REGISTRY_CONFIG), _data_signal_renaming(data_signal_renaming)
{
}

fep3::Result DataRegistryConfiguration::registerPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_mapping_configuration, FEP3_MAPPING_CONFIGURATION_PROPERTY));
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_mapping_configuration_file_path, FEP3_MAPPING_CONFIGURATION_FILE_PATH_PROPERTY));
    FEP3_RETURN_IF_FAILED(_data_signal_renaming.registerPropertyVariables(*this));

    return{};
}

fep3::Result DataRegistryConfiguration::unregisterPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_mapping_configuration, FEP3_MAPPING_CONFIGURATION_PROPERTY));
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_mapping_configuration_file_path, FEP3_MAPPING_CONFIGURATION_FILE_PATH_PROPERTY));
    FEP3_RETURN_IF_FAILED(_data_signal_renaming.unregisterPropertyVariables(*this));

    return{};
}

DataRegistry::DataRegistry() : fep3::base::Component<IDataRegistry>()
{
}

DataRegistry::~DataRegistry()
{
    if (_receive_thread.joinable())
    {
        _receive_thread.join();
    }
}

fep3::Result DataRegistry::create()
{
    const auto components = _components.lock();
    if (components)
    {
        const auto service_bus = components->getComponent<IServiceBus>();
        if (service_bus)
        {
            const auto rpc_server = service_bus->getServer();
            if (rpc_server)
            {
                if (!_rpc_service)
                {
                    _rpc_service = std::make_shared<RPCDataRegistryService>(*this);
                    FEP3_RETURN_IF_FAILED(
                        rpc_server->registerService(::fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName(), _rpc_service));
                }
            }
            else
            {
                RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
            }
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Service Bus is not registered");
        }

        const auto configuration_service = components->getComponent<IConfigurationService>();
        if (configuration_service)
        {
            FEP3_RETURN_IF_FAILED(_configuration.initConfiguration(*configuration_service));
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Configuration Service is not registered");
        }
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Component pointer is invalid");
    }
    return{};
}

fep3::Result DataRegistry::destroy()
{
    _configuration.deinitConfiguration();
    auto components = _components.lock();
    if (components)
    {
        auto service_bus = components->getComponent<IServiceBus>();
        if (service_bus)
        {
            const auto rpc_server = service_bus->getServer();
            if (rpc_server)
            {
                FEP3_RETURN_IF_FAILED(
                    rpc_server->unregisterService(::fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName()));
            }
            else
            {
                RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
            }
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Service Bus is not registered");
        }
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Component pointer is invalid");
    }

    return {};
}

fep3::Result DataRegistry::initialize()
{
    _configuration.updatePropertyVariables();
    FEP3_RETURN_IF_FAILED(_data_signal_renaming.parseProperties());

    // Apply alias naming from renaming configuration to all already registered signals
    for (auto in : _ins)
    {
        const auto alias_name = _data_signal_renaming.getAliasInputName(in.second->getName());

        // Check that we not collide with another alias name
        for (const auto& signal : _ins)
        {
            if (signal.first != in.first
                && signal.second->getAlias() == alias_name)
            {
                RETURN_ERROR_DESCRIPTION(ERR_NOT_SUPPORTED
                    , "The input signal name '%s' alias '%s' is already registered as signal with same alias name."
                    , signal.first.c_str()
                    , alias_name.c_str());
            }
        }

        in.second->setAlias(alias_name);
    }

    for (auto out : _outs)
    {
        const auto alias_name = _data_signal_renaming.getAliasOutputName(out.second->getName());

        // Check that we not collide with another alias name
        for (const auto& signal : _outs)
        {
            if (signal.first != out.first
                && signal.second->getAlias() == alias_name)
            {
                RETURN_ERROR_DESCRIPTION(ERR_NOT_SUPPORTED
                    , "The output signal name '%s' alias '%s' is already registered as signal with same alias name."
                    , signal.first.c_str()
                    , alias_name.c_str());
            }
        }

        out.second->setAlias(alias_name);
    }
    return {};
}

fep3::Result DataRegistry::tense()
{
    // Get simulation bus connection
    ISimulationBus* simulation_bus{ nullptr };
    auto components = _components.lock();
    if (components)
    {
        simulation_bus = components->getComponent<ISimulationBus>();
    }
    if (!simulation_bus || !components)
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Simulation Bus is not registered");
    }

    // Register input signals without a mapping at the simulation bus
    for (const auto& current_in : _ins)
    {
        FEP3_RETURN_IF_FAILED(current_in.second->registerAtSimulationBus(*simulation_bus));
    }
    std::promise<void> blocking_reception_prepared;
    auto blocking_reception_prepared_result = blocking_reception_prepared.get_future();
    _receive_thread = std::thread([simulation_bus, &blocking_reception_prepared]()
    {
        simulation_bus->startBlockingReception([&blocking_reception_prepared]()
        {
            blocking_reception_prepared.set_value();
        });
    });
    // wait for the blocking reception to be prepared
    blocking_reception_prepared_result.get();

    // Register input signals with a mapping at the signal mapping subcomponent
    for (const auto& current_in : _mapped_ins)
    {
        FEP3_RETURN_IF_FAILED(current_in.second->registerAtSignalMapping(_mapping));
    }

    // Register output signals without a mapping at the simulation bus
    for (const auto& current_out : _outs)
    {
        FEP3_RETURN_IF_FAILED(current_out.second->registerAtSimulationBus(*simulation_bus));
    }

    signals_registered_at_simulation_bus = true;

    return{};
}

fep3::Result DataRegistry::start()
{
    return _mapping.startMappingEngine();
}

fep3::Result DataRegistry::stop()
{
    return _mapping.stopAndResetMappingEngine();
}

fep3::Result DataRegistry::relax()
{
    ISimulationBus* simulation_bus{ nullptr };
    auto components = _components.lock();
    if (components)
    {
        simulation_bus = components->getComponent<ISimulationBus>();
    }
    if (!simulation_bus || !components)
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Simulation Bus is not registered");
    }

    simulation_bus->stopBlockingReception();
    if (_receive_thread.joinable())
    {
        _receive_thread.join();
    }

    // Unregister ALL signals OUT
    for (auto& current_out : _outs)
    {
        current_out.second->unregisterFromSimulationBus();
    }

    // Unregister ALL mapped signals IN
    for (auto& current_in : _mapped_ins)
    {
        _mapping.unregisterDataReceiver(current_in.second->getName());
    }

    // Unregister ALL signals IN
    for (auto& current_in : _ins)
    {
        current_in.second->unregisterFromSimulationBus();
    }
    signals_registered_at_simulation_bus = false;
    return{};
}

fep3::Result DataRegistry::unregisterDataIn(const std::string& name)
{
    if (removeDataIn(name))
    {
        return{};
    }
    else
    {
        std::string description = "Data Registry does not have an input signal named " + name + " registered";
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, description.c_str());
    }
}

fep3::Result DataRegistry::unregisterDataOut(const std::string& name)
{
    if (removeDataOut(name))
    {
        return{};
    }
    else
    {
        std::string description = "Data Registry does not have an output signal named " + name + " registered";
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, description.c_str());
    }
}

fep3::Result DataRegistry::registerDataReceiveListener(const std::string& name,
    const std::shared_ptr<IDataRegistry::IDataReceiver>& listener)
{
    DataSignalIn* found = getAnyDataIn(name);
    if (found)
    {
        found->registerDataListener(listener);
        return{};
    }
    std::string description = "Data Registry does not have an input signal named " + name + " registered";
    RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, description.c_str());
}

fep3::Result DataRegistry::unregisterDataReceiveListener(const std::string& name,
    const std::shared_ptr<IDataRegistry::IDataReceiver>& listener)
{
    DataSignalIn* found = getAnyDataIn(name);
    if (found)
    {
        found->unregisterDataListener(listener);
        return{};
    }
    std::string description = "Data Registry does not have an input signal named " + name + " registered";
    RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, description.c_str());
}

std::unique_ptr<IDataRegistry::IDataReader> DataRegistry::getReader(const std::string& name)
{
    return getReader(name, size_t(1));
}

std::unique_ptr<IDataRegistry::IDataReader> DataRegistry::getReader(const std::string& name,
    size_t queue_capacity)
{
    std::unique_ptr<IDataRegistry::IDataReader> reader{ nullptr };
    DataSignalIn* found = getAnyDataIn(name);
    if (found)
    {
        reader = found->getReader(queue_capacity);
    }
    return reader;
}

std::unique_ptr<IDataRegistry::IDataWriter> DataRegistry::getWriter(const std::string& name)
{
    return getWriter(name, size_t(0));
}

std::unique_ptr<IDataRegistry::IDataWriter> DataRegistry::getWriter(const std::string& name, size_t queue_capacity)
{
    std::unique_ptr<IDataRegistry::IDataWriter> writer{ nullptr };
    DataSignalOut* found = getDataOut(name);
    if (found)
    {
        writer = found->getWriter(queue_capacity);
    }
    return writer;
}

std::vector<std::string> DataRegistry::getSignalInNames()
{
    std::vector<std::string> retval;
    for (const auto& signal : _ins)
    {
        retval.push_back(signal.second->getAlias());
    }
    return retval;
}

std::vector<std::string> DataRegistry::getSignalOutNames()
{
    std::vector<std::string> retval;
    for (const auto& signal : _outs)
    {
        retval.push_back(signal.second->getAlias());
    }
    return retval;
}

base::StreamType DataRegistry::getStreamType(const std::string& name)
{
    DataSignal* signal{nullptr};

    signal = getDataInByAlias(name);
    if (signal)
    {
        return signal->getType();
    }

    signal = getDataOutByAlias(name);
    if (signal)
    {
        return signal->getType();
    }

    return base::StreamType{ base::StreamMetaType{"hook"} };
}

handle_t DataRegistry::getSignalInHandle(const std::string& name)
{
    return static_cast<handle_t>(getDataIn(name));
}

fep3::Result DataRegistry::registerDDL(const IStreamType& ddl_type, Action action)
{
    // Cancel if the DDL Struct of the stream type is already registered
    {
        std::string type_struct = ddl_type.getProperty(fep3::base::arya::meta_type_prop_name_ddlstruct);
        if (_type_descriptions.find(type_struct) != _type_descriptions.end())
        {
            return{};
        }
    }

    std::string desc = ddl_type.getProperty(fep3::base::arya::meta_type_prop_name_ddldescription);
    if (desc.empty())
    {
        std::string desc_file = ddl_type.getProperty(fep3::base::arya::meta_type_prop_name_ddlfileref);
        if (desc_file.empty())
        {
            RETURN_ERROR_DESCRIPTION(ERR_EMPTY, "A DDL StreamType has no description or file reference defined");
        }
        if (a_util::filesystem::readTextFile(desc_file, desc) != a_util::filesystem::OK)
        {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_FILE, std::string("Failed to read DDL description file " + desc_file).c_str());
        }
    }

    if (action == Action::Replace)
    {
        FEP3_RETURN_IF_FAILED(_description_mgr.loadDDL(desc));
        _type_descriptions.clear();
    }
    else if (action == Action::Merge)
    {
        FEP3_RETURN_IF_FAILED(_description_mgr.mergeDDL(desc));
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNKNOWN, "Unable to register DDL because of unknown action type");
    }

    _mapping.resetSignalDescription(_description_mgr.getDDL());

    return{};
}

fep3::Result DataRegistry::resolveSignalType(const std::string& type_name, std::string*& type_description)
{
    auto it_desc = _type_descriptions.find(type_name);
    if (it_desc == _type_descriptions.end())
    {
        std::string description;
        fep3::Result res = _description_mgr.resolveType(type_name, description);
        if (isOk(res))
        {
            type_description = &(_type_descriptions.emplace(std::make_pair(type_name, description)).first->second);
        }
        return res;
    }
    else
    {
        type_description = &(it_desc->second);
    }
    return{};
}

fep3::Result DataRegistry::updateMappingConfiguration()
{
    std::string old_config = static_cast<std::string>(_configuration._mapping_configuration);
    std::string old_config_file = static_cast<std::string>(_configuration._mapping_configuration_file_path);

    _configuration.updatePropertyVariables();

    std::string config = static_cast<std::string>(_configuration._mapping_configuration);
    std::string config_file = static_cast<std::string>(_configuration._mapping_configuration_file_path);
    // Compare to old property values to prevent unnecessary registration
    if (old_config != config || old_config_file != config_file)
    {
        FEP3_RETURN_IF_FAILED(_mapping.registerMappingConfiguration(config, config_file, _description_mgr.getDDL()));
    }
    return{};
}

fep3::Result DataRegistry::registerDataIn(const std::string& name,
    const IStreamType& type,
    bool is_dynamic_meta_type)
{
    using fep3::base::arya::meta_type_ddl;
    using fep3::base::arya::meta_type_ddl_fileref;
    using fep3::base::arya::meta_type_ddl_array;
    using fep3::base::arya::meta_type_ddl_array_fileref;
    using fep3::base::arya::meta_type_prop_name_ddlstruct;

    FEP3_RETURN_IF_FAILED(DataSignalRenaming::checkName(name));

    auto found_data_in = getAnyDataIn(name);
    if (found_data_in)
    {
        std::string found_meta_type_name = found_data_in->getType().getMetaTypeName();
        std::string meta_type_name = type.getMetaTypeName();
        // Check if both StreamTypes are ddl or ddl-arrays but ignore if its a fileref or not
        if (((meta_type_name == meta_type_ddl.getName() || meta_type_name == meta_type_ddl_fileref.getName()) 
            && (found_meta_type_name == meta_type_ddl.getName() || found_meta_type_name == meta_type_ddl_fileref.getName()))
            || ((meta_type_name == meta_type_ddl_array.getName() || meta_type_name == meta_type_ddl_array_fileref.getName()) 
            && (found_meta_type_name == meta_type_ddl_array.getName() || found_meta_type_name == meta_type_ddl_array_fileref.getName())))
        {
            if (type.getProperty(meta_type_prop_name_ddlstruct) == found_data_in->getType().getProperty(meta_type_prop_name_ddlstruct))
            {
                return{};
            }
            else
            {
                std::string description = "The input signal " + name + " does already exist, but with a different type: Passed ddl type with struct " +
                    type.getProperty(meta_type_prop_name_ddlstruct) + " but found ddl type with struct " + 
                    found_data_in->getType().getProperty(meta_type_prop_name_ddlstruct);
                RETURN_ERROR_DESCRIPTION(ERR_INVALID_TYPE, description.c_str());
            }
        }
        if (found_data_in->getType() == type)
        {
            return{};
        }
        else
        {
            std::string description = "The input signal " + name + " does already exist, but with a different type: Passed type " +
                type.getMetaTypeName() + " but found type " + found_data_in->getType().getMetaTypeName();
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_TYPE, description.c_str());
        }
    }

    if (type.getMetaTypeName() == fep3::base::arya::meta_type_ddl.getName() ||
        type.getMetaTypeName() == fep3::base::arya::meta_type_ddl_fileref.getName())
    {
        FEP3_RETURN_IF_FAILED(registerDDL(type, Action::Merge));
    }

    // Properties don't have passive listeners so we need to check for an update
    FEP3_RETURN_IF_FAILED(updateMappingConfiguration());

    if (_mapping.checkMappingConfiguration(name))
    {
        FEP3_RETURN_IF_FAILED(_mapping.registerSignal(name));
        _mapped_ins.emplace(name, std::make_shared<DataSignalIn>(name, name, type, is_dynamic_meta_type));
    }
    else
    {
        const auto alias_name = _data_signal_renaming.getAliasInputName(name);
        if (checkFreeAliasName(alias_name))
        {
            auto data_signal_in = std::make_shared<DataSignalIn>(name, alias_name, type, is_dynamic_meta_type);
            _ins.emplace(name, data_signal_in);

            if (signals_registered_at_simulation_bus)
            {
                ISimulationBus* simulation_bus{ nullptr };
                auto components = _components.lock();
                if (components)
                {
                    simulation_bus = components->getComponent<ISimulationBus>();
                }
                if (!simulation_bus || !components)
                {
                    RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Simulation Bus is not registered");
                }
                return data_signal_in->registerAtSimulationBus(*simulation_bus);
            }
            
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(ERR_NOT_SUPPORTED, "The input signal name '%s' is already registered as signal with same alias name.", alias_name.c_str());
        }
        
    }
    return {};
}

fep3::Result DataRegistry::registerDataOut(const std::string& name,
    const IStreamType& type,
    bool is_dynamic_meta_type)
{
    FEP3_RETURN_IF_FAILED(DataSignalRenaming::checkName(name));

    auto found_data_out = getDataOut(name);
    if (found_data_out)
    {
        if (found_data_out->getType() == type)
        {
            return{};
        }
        else
        {
            std::string description = "The output signal " + name + " does already exist, but with a different type: Passed type " +
                type.getMetaTypeName() + " but found type " + found_data_out->getType().getMetaTypeName();
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_TYPE, description.c_str());
        }
    }

    if (type.getMetaTypeName() == fep3::base::arya::meta_type_ddl.getName() ||
        type.getMetaTypeName() == fep3::base::arya::meta_type_ddl_fileref.getName())
    {
        FEP3_RETURN_IF_FAILED(registerDDL(type, Action::Merge));
    }

    const auto alias_name = _data_signal_renaming.getAliasOutputName(name);
    if (checkFreeAliasName(alias_name, true))
    {
        auto data_signal_out = std::make_shared<DataSignalOut>(name, alias_name, type, is_dynamic_meta_type);
        _outs.emplace(name, data_signal_out);

        if (signals_registered_at_simulation_bus)
        {
            ISimulationBus* simulation_bus{ nullptr };
            auto components = _components.lock();
            if (components)
            {
                simulation_bus = components->getComponent<ISimulationBus>();
            }
            if (!simulation_bus || !components)
            {
                RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Simulation Bus is not registered");
            }
            return data_signal_out->registerAtSimulationBus(*simulation_bus);
        }
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_SUPPORTED, "The output signal name '%s' is already registered as signal with same alias name.", alias_name.c_str());
    }

    return{};
}

DataRegistry::DataSignalIn* DataRegistry::getDataIn(const std::string& name)
{
    auto found = _ins.find(name);
    if (found != _ins.end())
    {
        return found->second.get();
    }
    return nullptr;
}

DataRegistry::DataSignalIn* DataRegistry::getMappedDataIn(const std::string& name)
{
    auto found = _mapped_ins.find(name);
    if (found != _mapped_ins.end())
    {
        return found->second.get();
    }
    return nullptr;
}

DataRegistry::DataSignalIn* DataRegistry::getAnyDataIn(const std::string& name)
{
    auto found = getDataIn(name);
    if (!found)
    {
        found = getMappedDataIn(name);
    }
    return found;
}

DataRegistry::DataSignalOut* DataRegistry::getDataOut(const std::string& name)
{
    auto found = _outs.find(name);
    if (found != _outs.end())
    {
        return found->second.get();
    }
    return nullptr;
}

DataRegistry::DataSignalIn* DataRegistry::getDataInByAlias(const std::string& name)
{
    for (auto in : _ins)
    {
        if (in.second->getAlias() == name) return in.second.get();
    }
    return nullptr;
}

DataRegistry::DataSignalOut* DataRegistry::getDataOutByAlias(const std::string& name)
{
    for (auto out : _outs)
    {
        if (out.second->getAlias() == name) return out.second.get();
    }
    return nullptr;
}

bool DataRegistry::removeDataIn(const std::string& name)
{
    return (_ins.erase(name) > 0);
}

bool DataRegistry::removeDataOut(const std::string& name)
{
    return (_outs.erase(name) > 0);
}

bool DataRegistry::checkFreeAliasName(const std::string& name, bool output)
{
    if(output)
    {
        for (const auto& signal : _outs)
        {
            if(signal.second->getAlias() == name)
            {
                return false;
            }
        }
    }
    else
    {
        for (const auto& signal : _ins)
        {
            if (signal.second->getAlias() == name)
            {
                return false;
            }
        }
    }
    return true;
}
