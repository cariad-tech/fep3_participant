/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "test_connext_dds_simulation_bus.hpp"

#include <fep3/base/stream_type/default_stream_type.h>

class AsyncReaderWriterTestClassType {
};
class ReaderWriterTestClassType {
};
class SignalWaitingTestClassType {
};

using SimulationBusTypes =
    ::testing::Types<AsyncReaderWriterTestClassType, ReaderWriterTestClassType>;

template <typename T>
class ReaderWriterTestSimulationBus : public TestConnextDDSSimulationBus {
protected:
    virtual void SetUp()
    {
        TestConnextDDSSimulationBus::SetUp();

        _domain_id = randomDomainId();
        _sim_participant_name_1 = makePlatformDepName("simbus_participant_1");
        _sim_participant_name_2 = makePlatformDepName("simbus_participant_2");
        _sim_participant_name_3 = makePlatformDepName("simbus_participant_3");
        _sim_test_system_name = makePlatformDepName("default_test_system");
        _logger_mock = std::make_shared<::testing::NiceMock<fep3::mock::Logger>>();

        ASSERT_NO_THROW(_simulation_bus =
                            createSimulationBus(_sim_participant_name_1, _sim_test_system_name);
                        _simulation_bus_2 =
                            createSimulationBus(_sim_participant_name_2, _sim_test_system_name););

        std::string topic = findFreeTopic();
        _writer = getSimulationBus2()->getWriter(topic, fep3::base::StreamTypePlain<uint32_t>());
        _reader = getSimulationBus()->getReader(topic, fep3::base::StreamTypePlain<uint32_t>());

        ASSERT_TRUE(_writer);
        ASSERT_TRUE(_reader);
    }

    std::shared_ptr<IComponent> createSimulationBus(std::shared_ptr<Components> components)
    {
        auto simulation_bus =
            fep3::ComponentFactoryCPPPlugin(FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB)
                .createComponent(ISimulationBus::getComponentIID(), nullptr);
        if (!simulation_bus) {
            return nullptr;
        }

        EXPECT_FEP3_NOERROR(simulation_bus->createComponent(components));

        auto property_node =
            components->_configuration_service->getNode(FEP3_RTI_DDS_SIMBUS_CONFIG);
        if (!property_node) {
            return nullptr;
        }

        if (auto property =
                std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>(
                    property_node->getChild(FEP3_SIMBUS_PARTICIPANT_DOMAIN_PROPERTY))) {
            property->setValue(std::to_string(_domain_id));
        }

        if (std::is_same<T, AsyncReaderWriterTestClassType>::value) {
            if (auto property =
                    std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>(
                        property_node->getChild(FEP3_RTI_DDS_SIMBUS_ASYNC_WAITSET_PROPERTY))) {
                property->setValue("true");
            }
        }

        EXPECT_FEP3_NOERROR(simulation_bus->initialize());
        EXPECT_FEP3_NOERROR(simulation_bus->tense());
        EXPECT_FEP3_NOERROR(simulation_bus->start());

        return simulation_bus;
    }

    std::shared_ptr<IComponent> createSimulationBus(std::string participant_name,
                                                    std::string system_name = "default_test_system")
    {
        std::shared_ptr<Components> components =
            std::make_shared<Components>(participant_name, system_name, _logger_mock);
        return createSimulationBus(components);
    }

    void TearDownComponent(IComponent& component)
    {
        EXPECT_FEP3_NOERROR(component.stop());
        EXPECT_FEP3_NOERROR(component.relax());
        EXPECT_FEP3_NOERROR(component.deinitialize());
    }

    void TearDown()
    {
        _writer.reset();
        _reader.reset();

        // stop all potentially running receptions
        stopReception(getSimulationBus());
        stopReception(getSimulationBus2());

        if (_simulation_bus) {
            TearDownComponent(*_simulation_bus);
            _simulation_bus.reset();
        }

        if (_simulation_bus_2) {
            TearDownComponent(*_simulation_bus_2);
            _simulation_bus_2.reset();
        }
    }

    fep3::ISimulationBus* getSimulationBus()
    {
        return dynamic_cast<ISimulationBus*>(_simulation_bus.get());
    }
    fep3::ISimulationBus* getSimulationBus2()
    {
        return dynamic_cast<ISimulationBus*>(_simulation_bus_2.get());
    }

    std::string findFreeTopic()
    {
        return makePlatformDepName("test");
    }

    uint32_t getDomainId() const
    {
        return _domain_id;
    }

protected:
    std::unique_ptr<fep3::ISimulationBus::IDataWriter> _writer;
    std::unique_ptr<fep3::ISimulationBus::IDataReader> _reader;
    std::shared_ptr<::testing::NiceMock<fep3::mock::Logger>> _logger_mock;
    std::shared_ptr<IComponent> _simulation_bus;
    std::shared_ptr<IComponent> _simulation_bus_2;
    std::string _sim_participant_name_1;
    std::string _sim_participant_name_2;
    std::string _sim_participant_name_3;
    std::string _sim_test_system_name;

private:
    uint32_t _domain_id;
};

using ReaderWriterTestClass = ReaderWriterTestSimulationBus<ReaderWriterTestClassType>;
using AsyncReaderWriterTestClass = ReaderWriterTestSimulationBus<AsyncReaderWriterTestClassType>;

class SignalWaitingTestClass : public TestConnextDDSSimulationBus {
protected:
    std::shared_ptr<fep3::IComponent> createSimulationBus(
        uint32_t domain_id,
        const std::string& participant_name,
        const std::string& system_name = "default_test_system",
        int64_t datawriter_ready_timeout = 0ll,
        const std::string& must_be_ready_signals = "",
        std::shared_ptr<fep3::mock::Logger> logger_mock =
            std::make_shared<::testing::NiceMock<fep3::mock::Logger>>()) const
    {
        std::shared_ptr<Components> components =
            std::make_shared<Components>(participant_name, system_name, logger_mock);
        auto simulation_bus =
            fep3::ComponentFactoryCPPPlugin(FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB)
                .createComponent(ISimulationBus::getComponentIID(), nullptr);
        if (!simulation_bus) {
            return nullptr;
        }

        EXPECT_FEP3_NOERROR(simulation_bus->createComponent(components));

        auto property_node =
            components->_configuration_service->getNode(FEP3_RTI_DDS_SIMBUS_CONFIG);
        if (!property_node) {
            return nullptr;
        }

        if (auto property =
                std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>(
                    property_node->getChild(FEP3_SIMBUS_PARTICIPANT_DOMAIN_PROPERTY))) {
            property->setValue(std::to_string(domain_id));
            property->updateObservers();
        }

        if (datawriter_ready_timeout != 0ll) {
            if (auto property =
                    std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>(
                        property_node->getChild(FEP3_SIMBUS_DATAWRITER_READY_TIMEOUT_PROPERTY))) {
                property->setValue(std::to_string(datawriter_ready_timeout));
                property->updateObservers();
            }
        }

        if (!must_be_ready_signals.empty()) {
            if (auto property =
                    std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>(
                        property_node->getChild(FEP3_SIMBUS_MUST_BE_READY_SIGNALS_PROPERTY))) {
                property->setValue(must_be_ready_signals);
                property->updateObservers();
            }
        }

        EXPECT_FEP3_NOERROR(simulation_bus->initialize());
        EXPECT_FEP3_NOERROR(simulation_bus->tense());
        EXPECT_FEP3_NOERROR(simulation_bus->start());

        return simulation_bus;
    }
};
