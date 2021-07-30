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

#include "test_connext_dds_simulation_bus.hpp"
#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/components/logging/mock/mock_logging_service.h>

class ReaderWriterTestClass : public TestConnextDDSSimulationBus
{
public:
    class Components : public IComponents
    {
    public:
        std::unique_ptr<fep3::native::ConfigurationService> _configuration_service = std::make_unique<fep3::native::ConfigurationService>();
        std::unique_ptr<ParticipantInfo> _participant_info;
        std::unique_ptr<fep3::mock::LoggingService> _logging_service;

    public:
        Components(const std::string participant_name
            , const std::string system_name = "default_test_system"
            , std::shared_ptr<fep3::mock::LoggerMock> logging_mock = std::make_shared<::testing::NiceMock<fep3::mock::LoggerMock>>())
            : _participant_info(std::make_unique<ParticipantInfo>(participant_name, system_name))
            , _logging_service(std::make_unique<fep3::mock::LoggingService>(logging_mock))
        {
            _configuration_service->create();
            _configuration_service->initialize();
            _configuration_service->tense();
            _configuration_service->start();
        }

        IComponent* findComponent(const std::string& fep_iid) const
        {
            if (fep_iid == IConfigurationService::getComponentIID())
            {
                return _configuration_service.get();
            }
            else if (fep_iid == IParticipantInfo::getComponentIID())
            {
                return _participant_info.get();
            }
            else if (fep_iid == ILoggingService::getComponentIID())
            {
                return _logging_service.get();
            }
            return nullptr;
        }
    };
protected:

    virtual void SetUp()
    {
        TestConnextDDSSimulationBus::SetUp();

        _domain_id = randomDomainId();

        _simulation_bus = createSimulationBus(_domain_id, "simbus_participant_1");
        _simulation_bus_2 = createSimulationBus(_domain_id, "simbus_participant_2");

        std::string topic = findFreeTopic();
        _writer = getSimulationBus2()->getWriter(topic, fep3::base::StreamTypePlain<uint32_t>());
        _reader = getSimulationBus()->getReader(topic, fep3::base::StreamTypePlain<uint32_t>());

        ASSERT_TRUE(_writer);
        ASSERT_TRUE(_reader);
    }

    std::unique_ptr<IComponent> createSimulationBus(uint32_t domain_id, std::shared_ptr<Components> components)
    {
        auto simulation_bus = _factory->createComponent(ISimulationBus::getComponentIID(), nullptr);
        if (!simulation_bus)
        {
            return nullptr;
        }

        EXPECT_EQ(fep3::Result(), simulation_bus->createComponent(components));

        auto property_node = components->_configuration_service->getNode("rti_dds_simulation_bus");
        if (!property_node)
        {
            return nullptr;
        }

        if (auto property = std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>(property_node->getChild("participant_domain")))
        {
            property->setValue(std::to_string(domain_id));
            property->updateObservers();
        }


        EXPECT_EQ(fep3::Result(), simulation_bus->initialize());
        EXPECT_EQ(fep3::Result(), simulation_bus->tense());
        EXPECT_EQ(fep3::Result(), simulation_bus->start());

        return simulation_bus;
    }

    std::unique_ptr<IComponent> createSimulationBus(uint32_t domain_id, std::string participant_name, std::string system_name = "default_test_system")
    {
        std::shared_ptr<Components> components = std::make_shared<Components>(participant_name, system_name);
        return createSimulationBus(domain_id, components);
    }

    uint32_t randomDomainId()
    {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_real_distribution<float> dist(1.0, 200.0);

        return static_cast<unsigned int>(dist(mt)) % 200;
    }

    void TearDownComponent(IComponent & component)
    {
        EXPECT_EQ(fep3::Result(), component.stop());
        EXPECT_EQ(fep3::Result(), component.relax());
        EXPECT_EQ(fep3::Result(), component.deinitialize());
    }

    void TearDown()
    {
        _writer.reset();
        _reader.reset();

        // stop all potentially running receptions
        stopReception(getSimulationBus());
        stopReception(getSimulationBus2());

        if (_simulation_bus)
        {
            TearDownComponent(*_simulation_bus);
            _simulation_bus.reset();
        }

        if (_simulation_bus_2)
        {
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
        return std::string("test_") + std::to_string(1);
    }

    uint32_t getDomainId() const
    {
        return _domain_id;
    }

protected:
    std::unique_ptr<fep3::ISimulationBus::IDataWriter> _writer;
    std::unique_ptr<fep3::ISimulationBus::IDataReader> _reader;

    std::unique_ptr<IComponent> _simulation_bus;
    std::unique_ptr<IComponent> _simulation_bus_2;

private:
    uint32_t _domain_id;
};

