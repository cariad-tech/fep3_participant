/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "rti_conext_dds_include.h"

#include <fep3/base/properties/propertynode.h>
#include <fep3/components/base/component.h>
#include <fep3/components/simulation_bus/simulation_bus_intf.h>

#include <dds/core/QosProvider.hpp>

using namespace fep3::arya;

/**
 * Implements a simulation bus based on the Conext DDS implementation of the Data Distribution
 * Service (DDS) standard from RTI.
 *
 * The Data Distribution Service (DDS) for real-time systems a middleware standard that aims high
 * performance, interoperable, real-time, scalable data exchange.
 *
 * This implementation requires a predefined USER_QOS_PROFILES.xml. In the QOS Profiles you can
 * define your demands on QOS for each stream type or topic. Please read the documentation of RTI on
 * https://community.rti.com/. The USER_QOS_PROFILES.xml need to be located beside the
 * fep3_connext_dds_plugin, your application or you can use the environment variable
 * NDDS_QOS_PROFILES (a list of ';' seperated paths).
 */
class ConnextDDSSimulationBus : public fep3::base::Component<fep3::arya::ISimulationBus> {
public:
    ConnextDDSSimulationBus();
    ~ConnextDDSSimulationBus();
    ConnextDDSSimulationBus(const ConnextDDSSimulationBus&) = delete;
    ConnextDDSSimulationBus(ConnextDDSSimulationBus&&) = delete;
    ConnextDDSSimulationBus& operator=(const ConnextDDSSimulationBus&) = delete;
    ConnextDDSSimulationBus& operator=(ConnextDDSSimulationBus&&) = delete;

public: // the base::Component statemachine
    fep3::Result create() override;
    fep3::Result destroy() override;
    fep3::Result initialize() override;
    fep3::Result deinitialize() override;

public: // the arya SimulationBus interface
    bool isSupported(const IStreamType& stream_type) const override;

    std::unique_ptr<IDataReader> getReader(const std::string& name,
                                           const IStreamType& stream_type) override;

    std::unique_ptr<IDataReader> getReader(const std::string& name,
                                           const IStreamType& stream_type,
                                           size_t queue_capacity) override;

    std::unique_ptr<IDataReader> getReader(const std::string& name) override;
    std::unique_ptr<IDataReader> getReader(const std::string& name, size_t queue_capacity) override;
    std::unique_ptr<IDataWriter> getWriter(const std::string& name,
                                           const IStreamType& stream_type) override;
    std::unique_ptr<IDataWriter> getWriter(const std::string& name,
                                           const IStreamType& stream_type,
                                           size_t queue_capacity) override;
    std::unique_ptr<IDataWriter> getWriter(const std::string& name) override;
    std::unique_ptr<IDataWriter> getWriter(const std::string& name, size_t queue_capacity) override;

    void startBlockingReception(
        const std::function<void()>& reception_preparation_done_callback) override;
    void stopBlockingReception() override;

private:
    class ConnextDDSSimulationBusConfiguration : public fep3::base::Configuration {
    public:
        ConnextDDSSimulationBusConfiguration();
        ~ConnextDDSSimulationBusConfiguration() = default;

    public:
        fep3::Result registerPropertyVariables() override;
        fep3::Result unregisterPropertyVariables() override;

    public:
        fep3::base::PropertyVariable<int32_t> _participant_domain{
            FEP3_SIMBUS_PARTICIPANT_DOMAIN_DEFAULT_VALUE};
        fep3::base::PropertyVariable<int64_t> _datawriter_ready_timeout{
            FEP3_SIMBUS_DATAWRITER_READY_TIMEOUT_DEFAULT_VALUE};
        fep3::base::PropertyVariable<std::vector<std::string>> _must_be_ready_signals{
            FEP3_SIMBUS_MUST_BE_READY_SIGNALS_DEFAULT_VALUE};
        fep3::base::PropertyVariable<bool> _use_async_waitset{
            FEP3_RTI_DDS_SIMBUS_ASYNC_WAITSET_DEFAULT_VALUE};
        fep3::base::PropertyVariable<int32_t> _async_waitset_threads{
            FEP3_RTI_DDS_SIMBUS_ASYNC_WAITSET_THREADS_DEFAULT_VALUE};
    };

public:
    std::shared_ptr<dds::core::QosProvider> getQOSProfile() const;

private:
    class Impl;
    std::unique_ptr<Impl> _impl;

    void logError(const fep3::Result& res);

    ConnextDDSSimulationBusConfiguration _simulation_bus_configuration;

    /// Static variable to query information about the memory address of this binary
    static const int _address_info;
};
