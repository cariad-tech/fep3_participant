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

#pragma once

#include <cstddef>
#include <list>
#include <memory>
#include <unordered_map>

#include <a_util/result.h>

#include "fep3/base/stream_type/default_stream_type.h"
#include "fep3/base/stream_type/stream_type_intf.h"
#include "fep3/base/properties/propertynode.h"
#include "fep3/components/base/component.h"
#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep3/rpc_services/data_registry/data_registry_rpc_intf_def.h"
#include "fep3/rpc_services/data_registry/data_registry_service_stub.h"
#include "fep3/components/service_bus/rpc/fep_rpc.h"
#include "fep3/native_components/data_registry/mapping.h"
#include "fep3/native_components/data_registry/ddl_manager.h"
#include "fep3/components/service_bus/service_bus_intf.h"

#include "data_signal_renaming.h"

namespace fep3
{
namespace native
{
namespace arya
{
class DataRegistry;

class RPCDataRegistryService : public rpc::RPCService<fep3::rpc_stubs::RPCDataRegistryServiceStub, fep3::rpc::IRPCDataRegistryDef>
{
public:
    explicit RPCDataRegistryService(DataRegistry& data_registry) : _data_registry(data_registry) {}

public:
    std::string getSignalInNames() override;
    std::string getSignalOutNames() override;
    Json::Value getStreamType(const std::string& signal_name) override;

private:
    DataRegistry& _data_registry;
};

struct DataRegistryConfiguration : public base::Configuration
{
    explicit DataRegistryConfiguration(DataSignalRenaming& data_signal_renaming);
    ~DataRegistryConfiguration() = default;

    fep3::Result registerPropertyVariables() override;
    fep3::Result unregisterPropertyVariables() override;

    /// Property for a full mapping configuration as a single string
    base::PropertyVariable<std::string> _mapping_configuration{ "" };
    /// Property for a file path to the mapping configuration file
    base::PropertyVariable<std::string> _mapping_configuration_file_path{""};

    DataSignalRenaming& _data_signal_renaming;
};

/**
 * Native implementation of the data registry. Manages an internal list of
 * input and output signals which will be registered to the simulation bus
 * all at once during initialization.
 *
 * This class also provides getter functions for readers and writers to these signals.
 */
class DataRegistry : public base::Component<IDataRegistry>
{
public: // Types
    /// Description flags for signal description registration
    enum class Action : bool { Replace, Merge };

private:
    typedef std::map<std::string, std::string> tDescriptionMap;

public: // base::Component
    DataRegistry();
    DataRegistry(const DataRegistry&) = delete;
    DataRegistry(DataRegistry&&) = delete;
    DataRegistry& operator=(const DataRegistry&) = delete;
    DataRegistry& operator=(DataRegistry&&) = delete;
    ~DataRegistry() override;

    fep3::Result create() override;
    fep3::Result destroy() override;
    fep3::Result initialize() override;
    fep3::Result tense() override;
    fep3::Result relax() override;
    fep3::Result start() override;
    fep3::Result stop() override;

public: // IDataRegistry
    fep3::Result registerDataIn(const std::string& name,
                                const IStreamType& type,
                                bool is_dynamic_meta_type=false) override;
    fep3::Result registerDataOut(const std::string& name,
                                 const IStreamType& type,
                                 bool is_dynamic_meta_type=false) override;
    fep3::Result unregisterDataIn(const std::string& name) override;
    fep3::Result unregisterDataOut(const std::string& name) override;

    fep3::Result registerDataReceiveListener(const std::string& name,
        const std::shared_ptr<IDataReceiver>& listener) override;
    fep3::Result unregisterDataReceiveListener(const std::string& name,
        const std::shared_ptr<IDataReceiver>& listener) override;

    std::unique_ptr<IDataRegistry::IDataReader> getReader(const std::string& name) override;
    std::unique_ptr<IDataRegistry::IDataReader> getReader(const std::string& name,
        size_t queue_capacity) override;
    std::unique_ptr<IDataRegistry::IDataWriter> getWriter(const std::string& name) override;
    std::unique_ptr<IDataRegistry::IDataWriter> getWriter(const std::string& name,
        size_t queue_capacity) override;

public: // Member functions
    // Implementation functions of the RPC service
    std::vector<std::string> getSignalInNames();
    std::vector<std::string> getSignalOutNames();
    base::StreamType getStreamType(const std::string& name);

    // Handle getter used by the mapping engine
    handle_t getSignalInHandle(const std::string& name);

    /**
     * @brief Registers a ddl description at the mapping engine for the given signal direction.
     *
     * @param [in] ddl_type DDL StreamType containing the DDL description or file path
     * @param [in] direction Signal direction flag
     * @param [in] action Flag if the current description should be replaced or merged with the new ddl
     *
     * @retval ERR_INVALID_FILE Failed to read file or description does not contain a valid file path
     * @retval ERR_EMPTY The @p ddl_type does not contain any description or file path to a ddl
     * @retval ERR_INVALID_ARG The description is invalid
     * @retval ERR_INVALID_TYPE A data type is in conflict with an already existing data type (merge only)
     */
    fep3::Result registerDDL(const IStreamType& ddl_type, Action action = Action::Replace);

    /**
     * @brief Gets a complete but minimal DDL description that only contains the given type
     *
     * @param [in] type The DDL datatype the description should contain
     * @param [out] description A DDL description that only contains the given type
     *
     * @retval ERR_NOT_FOUND The type was not found in the registered ddl description
     */
    fep3::Result resolveSignalType(const std::string& type, std::string*& description);

public: // Data registry implementation detail classes
    class DataSignal; // public because it's used as a handle in the mapping engine
private:
    class DataSignalIn;
    class DataSignalOut;

    class DataReader;
    class DataWriter;
    class DataReaderProxy;
    class DataWriterProxy;

private: // Helper functions
    DataSignalIn* getDataIn(const std::string& name);
    DataSignalOut* getDataOut(const std::string& name);

    DataSignalIn* getDataInByAlias(const std::string& name);
    DataSignalOut* getDataOutByAlias(const std::string& name);

    bool removeDataIn(const std::string& name);
    bool removeDataOut(const std::string& name);
    DataSignalIn* getMappedDataIn(const std::string& name);
    DataSignalIn* getAnyDataIn(const std::string& name);
    fep3::Result updateMappingConfiguration();

    bool checkFreeAliasName(const std::string& name, bool output = false);

private: // Member variables
    /// Internal list of all input signals coming from the simulation bus
    std::unordered_map<std::string, std::shared_ptr<DataSignalIn>> _ins{};
    /// Internal list of all output signals going to the simualtion bus
    std::unordered_map<std::string, std::shared_ptr<DataSignalOut>> _outs{};
    std::thread _receive_thread;
    std::shared_ptr<IRPCServer::IRPCService> _rpc_service{ nullptr };

    /// Will be set in tense and unset in relax
    bool signals_registered_at_simulation_bus = false;

    // Mapping
    /// Internal list of all input signals coming from the mapping engine
    std::unordered_map<std::string, std::shared_ptr<DataSignalIn>> _mapped_ins{};
    /// Mapping engine adaptor for all mapped incoming signals
    SignalMapping _mapping{ *this };
    /// Data description manager managing the ddl description for mapped input signals
    DDLManager _description_mgr{};
    /// Type descriptions of all mapped input signals
    tDescriptionMap _type_descriptions{};

    // Signal Renaming
    DataSignalRenaming _data_signal_renaming{};

    /// Data Registry properties
    DataRegistryConfiguration _configuration{ _data_signal_renaming };
};
} // namespace arya
using arya::RPCDataRegistryService;
using arya::DataRegistry;
using arya::DataRegistryConfiguration;
} // namespace native
} // namespace fep3
