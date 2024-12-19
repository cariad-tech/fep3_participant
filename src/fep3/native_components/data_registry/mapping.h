/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/data_registry/data_registry_intf.h>

#include <ddl/mapping/engine/mapping_engine.h>

namespace fep3 {
namespace native {
namespace arya {
class DataRegistry;
/**
 * @brief Base class for the signal mapping. Functions as an adapter between the data registry and
 * the DDL mapping.
 */
class SignalMapping : public ddl::mapping::rt::IMappingEnvironment {
public:
    /**
     * @brief CTOR
     *
     * @param [in] registry The data registry object holding this signal mapping object
     */
    explicit SignalMapping(DataRegistry& registry);

    /// DTOR
    ~SignalMapping();

public:
    /*
     * @brief Registers a mapping configuration file at the participant. This has no effect on
     * currently registered signals and will only be considered during signal registration.
     *
     * @param [in] path_to_config_file A file path to a mapping configuration file
     * @param [in] ddl_description A description containing all ddl types used in the mapping
     * configuration
     *
     * @retval ERR_INVALID_FILE Could not read configuration file
     * @retval ERR_FAILED Configuration is invalid and cannot be parsed
     */
    fep3::Result registerMappingConfiguration(const std::string& path_to_config_file,
                                              const ddl::dd::DataDefinition& ddl_description);

    /*
     * @brief Checks if the given target signal name is being mapped in the current mapping
     * configuration
     *
     * @param [in] target_signal_name The name of the mapped target signal
     * @return true if it is being mapped
     */
    bool checkMappingConfiguration(const std::string& target_signal_name);

    /*
     * @brief Clears the mapping configuration of the participant.
     * This has no effect on currently registered signals or instantiated mappings.
     */
    void clearMappingConfiguration() noexcept;

    /*
     * @brief Resets the DDL signal description of the mapping engine
     *
     * @param [in] ddl_description The new DDL to use
     */
    void resetSignalDescription(const ddl::dd::DataDefinition& ddl_description) noexcept;

    /*
     * @brief Tries to register a target signal at the mapping engine using the current mapping
     * configuration. This will also register all source signals at the data registry.
     *
     * @param [in] target_signal_name The name of the target signal
     *
     * @retval ERR_INVALID_ARG No signal with @p signal_name could be found in the mapping
     * configuration or target is already mapped
     * @retval ERR_FAILED Creation of mapping failed
     */
    fep3::Result registerSignal(const std::string& target_signal_name);

    /*
     * @brief Unregisters a mapped target signal
     *
     * @param [in] target_signal_name The name of the target signal
     *
     * @retval ERR_NOT_FOUND No signal with @p signal_name could be found
     * @retval ERR_INVALID_ARG  Error target not found in mapping structure
     * @retval ERR_FAILED Removal of mapping failed
     */
    fep3::Result unregisterSignal(const std::string& target_signal_name);

    /*
     * @brief Registers a data receiver for a mapped signal, which will be called upon a configured
     * trigger condition.
     *
     * @param [in] data_receiver A shared pointer to the data receiver instance
     * @param [in] target_signal_name The name of the target signal
     *
     * @retval ERR_NOT_FOUND No signal with @p signal_name has been registered
     * @retval ERR_RESOURCE_IN_USE The @p data_receiver is already registered to this signal
     */
    fep3::Result registerDataReceiver(std::shared_ptr<IDataRegistry::IDataReceiver> data_receiver,
                                      const std::string& target_signal_name);

    /*
     * @brief Unregisters a data receiver for a mapped signal.
     *
     * @param [in] target_signal_name The name of the target signal
     *
     * @retval ERR_INVALID_ARG No signal with @p signal_name has been registered
     * @retval ERR_NOT_FOUND No data receiver is registered to this signal
     */
    fep3::Result unregisterDataReceiver(const std::string& target_signal_name);

    /*
     * @brief Starts the mapping engine
     *
     * @retval ERR_INVALID_STATE The mapping engine is already in running
     */
    fep3::Result startMappingEngine();

    /*
     * @brief Stops and resets the mapping engine
     *
     * @retval ERR_INVALID_STATE The mapping engine is not in running
     */
    fep3::Result stopAndResetMappingEngine();

    /*
     * Implementation of the IMappingEnvironment interface
     */

public:
    a_util::result::Result registerSource(const char* source_name,
                                          const char* type_name,
                                          ::ddl::mapping::rt::ISignalListener* listener,
                                          handle_t& handle) override;
    a_util::result::Result unregisterSource(handle_t handle) override;

    a_util::result::Result sendTarget(handle_t target,
                                      const void* data,
                                      size_t size,
                                      timestamp_t time_stamp) override;
    a_util::result::Result targetMapped(const char* target_name,
                                        const char* target_type,
                                        handle_t target,
                                        size_t target_size) override;
    a_util::result::Result targetUnmapped(const char* target_name, handle_t target) override;
    a_util::result::Result resolveType(const char* type_name,
                                       const char*& type_description) override;
    timestamp_t getTime() const override;
    a_util::result::Result registerPeriodicTimer(
        timestamp_t period_us, ::ddl::mapping::rt::IPeriodicListener* listener) override;
    a_util::result::Result unregisterPeriodicTimer(
        timestamp_t period_us, ::ddl::mapping::rt::IPeriodicListener* listener) override;

private:
    /// Mapping configuration from the mapping configuration file
    ddl::mapping::MapConfiguration _config{};
    /// Reference to the data registry to register mapped signals
    DataRegistry& _data_registry;
    /// Name to handle relation of the target signals
    std::map<std::string, handle_t> _mapped_signals{};
    /// Preallocated sample buffers for every registered target
    std::map<handle_t, std::shared_ptr<IDataSample>> _sample_buffers{};
    /// List of data receivers for every signal to be called during target signal trigger
    std::map<handle_t, std::shared_ptr<IDataRegistry::IDataReceiver>> _data_receivers{};
    /// Mapping engine doing the actual mapping
    ddl::mapping::rt::MappingEngine _engine{*this};
};

} // namespace arya
using arya::SignalMapping;
} // namespace native
} // namespace fep3