/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/base/component_registry/include/component_factory/component_factory_cpp_plugins.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/components/logging/mock_logging_service.h>
#include <fep3/components/participant_info/participant_info_intf.h>
#include <fep3/components/simulation_bus/simulation_bus_intf.h>
#include <fep3/native_components/configuration/configuration_service.h>

#include <common/gtest_asserts.h>
#include <helper/platform_dep_name.h>

// indentation comment for clang-format
#include <future>
#include <random>

using namespace fep3;

static const std::string default_test_name = "default_test_system";

class TestConnextDDSSimulationBus : public ::testing::Test {
public:
    static uint32_t randomDomainId()
    {
        std::random_device rd;
        std::mt19937 mt(rd());
        // The actual property is int32, so we cannot set the full value span of uint32.
        // The Domain ID can only be in range 1 - 167 on Windows
        // https://community.rti.com/static/documentation/connext-dds/6.0.1/doc/manuals/connext_dds/html_files/RTI_ConnextDDS_CoreLibraries_UsersManual/Content/UsersManual/ChoosingDomainID.htm
        std::uniform_int_distribution<uint32_t> dist(1, 167u);

        return dist(mt);
    }

    void TearDownComponent(fep3::IComponent& component)
    {
        EXPECT_EQ(fep3::Result(), component.stop());
        EXPECT_EQ(fep3::Result(), component.relax());
        EXPECT_EQ(fep3::Result(), component.deinitialize());
    }

    std::shared_ptr<fep3::IComponent> createSimulationBusDep(
        uint32_t domain_id,
        const std::string& participant_name,
        const std::string& system_name = default_test_name) const
    {
        return createSimulationBus(
            domain_id, makePlatformDepName(participant_name), makePlatformDepName(system_name));
    }

    std::shared_ptr<fep3::IComponent> createSimulationBus(
        uint32_t domain_id,
        const std::string& participant_name,
        const std::string& system_name = default_test_name) const
    {
        auto simulation_bus =
            fep3::ComponentFactoryCPPPlugin(FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB)
                .createComponent(fep3::ISimulationBus::getComponentIID(), nullptr);
        if (!simulation_bus) {
            return nullptr;
        }

        std::shared_ptr<Components> components =
            std::make_shared<Components>(participant_name, system_name);
        EXPECT_EQ(fep3::Result(), simulation_bus->createComponent(components));

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

        EXPECT_FEP3_NOERROR(simulation_bus->initialize());
        EXPECT_FEP3_NOERROR(simulation_bus->tense());
        EXPECT_FEP3_NOERROR(simulation_bus->start());

        return simulation_bus;
    }

    void startReception(fep3::ISimulationBus* simulation_bus)
    {
        // stop any potentially running reception
        stopReception(simulation_bus);

        auto receiver_thread_iterator = _receiver_threads.cbegin();
        for (; receiver_thread_iterator != _receiver_threads.cend(); ++receiver_thread_iterator) {
            if (receiver_thread_iterator->first == simulation_bus) {
                break;
            }
        }
        if (receiver_thread_iterator == _receiver_threads.cend()) {
            std::promise<void> blocking_reception_prepared;
            auto blocking_reception_prepared_result = blocking_reception_prepared.get_future();
            receiver_thread_iterator = _receiver_threads.emplace(
                _receiver_threads.cend(),
                simulation_bus,
                std::make_unique<std::thread>([simulation_bus, &blocking_reception_prepared]() {
                    try {
                        simulation_bus->startBlockingReception([&blocking_reception_prepared]() {
                            blocking_reception_prepared.set_value();
                        });
                    }
                    catch (std::exception exception) {
                        GTEST_FAIL();
                    }
                }));
            // wait for the blocking reception to be prepared
            blocking_reception_prepared_result.get();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    void stopReception(fep3::ISimulationBus* simulation_bus)
    {
        // stop the reception
        simulation_bus->stopBlockingReception();
        auto it = _receiver_threads.begin();
        // wait for the end and delte it from list
        while (it != _receiver_threads.end()) {
            if ((*it).first == simulation_bus && (*it).second) {
                (*it).second->join();
                (*it).second.reset();
                it = _receiver_threads.erase(it);
                break;
            }
            else {
                it++;
            }
        }
    }

public:
    class ParticipantInfo : public fep3::base::Component<fep3::IParticipantInfo> {
    public:
        ParticipantInfo(const std::string& participant_name, const std::string& system_name)
            : _participant_name(participant_name), _system_name(system_name)
        {
        }
        std::string getName() const override
        {
            return _participant_name;
        }

        std::string getSystemName() const override
        {
            return _system_name;
        }

    private:
        std::string _participant_name;
        std::string _system_name;
    };

    class Components : public fep3::IComponents {
    public:
        std::unique_ptr<fep3::native::ConfigurationService> _configuration_service =
            std::make_unique<fep3::native::ConfigurationService>();
        std::unique_ptr<ParticipantInfo> _participant_info;
        std::unique_ptr<fep3::mock::LoggingService> _logging_service;

    public:
        Components(const std::string participant_name,
                   const std::string system_name = default_test_name,
                   std::shared_ptr<fep3::mock::Logger> logging_mock =
                       std::make_shared<::testing::NiceMock<fep3::mock::Logger>>())
            : _participant_info(std::make_unique<ParticipantInfo>(participant_name, system_name)),
              _logging_service(std::make_unique<fep3::mock::LoggingService>())
        {
            EXPECT_CALL(*_logging_service, createLogger(::testing::_))
                .WillRepeatedly(::testing::Return(logging_mock));

            _configuration_service->create();
            _configuration_service->initialize();
            _configuration_service->tense();
            _configuration_service->start();
        }

        fep3::IComponent* findComponent(const std::string& fep_iid) const
        {
            if (fep_iid == fep3::IConfigurationService::getComponentIID()) {
                return _configuration_service.get();
            }
            else if (fep_iid == fep3::IParticipantInfo::getComponentIID()) {
                return _participant_info.get();
            }
            else if (fep_iid == fep3::ILoggingService::getComponentIID()) {
                return _logging_service.get();
            }
            return nullptr;
        }
    };

protected:
    std::list<std::pair<fep3::ISimulationBus*, std::unique_ptr<std::thread>>> _receiver_threads;
};
