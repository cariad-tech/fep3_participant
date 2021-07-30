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

#include <fep3/components/base/component.h>
#include <fep3/components/simulation_bus/simulation_bus_intf.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/base/properties/propertynode.h>
#include <plugins/rti_dds/simulation_bus/rti_conext_dds_include.h>

using namespace fep3::arya;

/**
* Implements a simulation bus based on the Conext DDS implementation of the Data Distribution Service (DDS) standard from RTI.
*
* The Data Distribution Service (DDS) for real-time systems a middleware standard that aims high performance, interoperable,
* real-time, scalable data exchange.
*
* This implementation requires a predefined USER_QOS_PROFILES.xml. In the QOS Profiles you can define your demands on QOS for
* each stream type or topic. Please read the documentation of RTI on https://community.rti.com/.
* The USER_QOS_PROFILES.xml need to be located beside the fep3_connext_dds_plugin, your application or you can use the
* environment variable NDDS_QOS_PROFILES (a list of ';' seperated paths).
*
*/
class ConnextDDSSimulationBus : public fep3::base::Component<fep3::arya::ISimulationBus>
{
    public:
        ConnextDDSSimulationBus();
        ~ConnextDDSSimulationBus();
        ConnextDDSSimulationBus(const ConnextDDSSimulationBus&) = delete;
        ConnextDDSSimulationBus(ConnextDDSSimulationBus&&) = delete;
        ConnextDDSSimulationBus& operator=(const ConnextDDSSimulationBus&) = delete;
        ConnextDDSSimulationBus& operator=(ConnextDDSSimulationBus&&) = delete;

    public: //the base::Component statemachine
        fep3::Result create() override;
        fep3::Result destroy() override;
        fep3::Result initialize() override;
        fep3::Result deinitialize() override;

    public: //the arya SimulationBus interface
        bool isSupported(const IStreamType& stream_type) const override;

        std::unique_ptr<IDataReader> getReader
            (const std::string& name
            , const IStreamType& stream_type
            ) override;

        std::unique_ptr<IDataReader> getReader
            (const std::string& name
            , const IStreamType& stream_type
            , size_t queue_capacity
            ) override;

        std::unique_ptr<IDataReader> getReader(const std::string& name) override;
        std::unique_ptr<IDataReader> getReader(const std::string& name, size_t queue_capacity) override;
        std::unique_ptr<IDataWriter> getWriter
            (const std::string& name
            , const IStreamType& stream_type
            ) override;
        std::unique_ptr<IDataWriter> getWriter
            (const std::string& name
            , const IStreamType& stream_type
            , size_t queue_capacity
            ) override;
        std::unique_ptr<IDataWriter> getWriter(const std::string& name) override;
        std::unique_ptr<IDataWriter> getWriter(const std::string& name, size_t queue_capacity) override;

        void startBlockingReception(const std::function<void()>& reception_preparation_done_callback) override;
        void stopBlockingReception() override;

    public:
        std::shared_ptr<dds::core::QosProvider> getQOSProfile() const;

    private:
        class ConnextDDSSimulationBusConfiguration : public fep3::base::Configuration
        {
        public:
            ConnextDDSSimulationBusConfiguration();
            ~ConnextDDSSimulationBusConfiguration() = default;

        public:
            fep3::Result registerPropertyVariables() override;
            fep3::Result unregisterPropertyVariables() override;

        public:
            fep3::base::PropertyVariable<int32_t>     _participant_domain{ 5 };
        };

    private:
        class Impl;
        std::unique_ptr<Impl> _impl;

        void logError(const fep3::Result& res);

        ConnextDDSSimulationBusConfiguration _simulation_bus_configuration;
};
