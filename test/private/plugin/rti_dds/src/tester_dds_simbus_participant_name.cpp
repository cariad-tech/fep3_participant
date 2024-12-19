/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "detail/test_connext_dds_simulation_bus.hpp"

#include <dds/dds.hpp>
#include <dds/topic/BuiltinTopic.hpp>

#include <notification_waiting.h>

class BuiltinParticipantListener
    : public dds::sub::NoOpDataReaderListener<dds::topic::ParticipantBuiltinTopicData> {
public:
    BuiltinParticipantListener(fep3::native::NotificationWaiting& wait_for_builtin_topic_reception)
        : _wait_for_builtin_topic_reception(wait_for_builtin_topic_reception)
    {
    }
    // This gets called when a subscriber has been discovered
    void on_data_available(dds::sub::DataReader<dds::topic::ParticipantBuiltinTopicData>& reader)
    {
        // We only process newly seen subscribers
        dds::sub::LoanedSamples<dds::topic::ParticipantBuiltinTopicData> samples =
            reader.select().state(dds::sub::status::DataState::new_instance()).take();

        for (const auto& sample: samples) {
            if (!sample.info().valid()) {
                continue;
            }

            auto part_name = sample.data()->participant_name().name();
            if (part_name.has_value()) {
                _received_dds_participant_names.emplace_back(part_name.value());
            }
        }
        _wait_for_builtin_topic_reception.notify();
    }

    std::vector<std::string> getReceivedParticipantNames() const
    {
        return _received_dds_participant_names;
    }

private:
    fep3::native::NotificationWaiting& _wait_for_builtin_topic_reception;
    std::vector<std::string> _received_dds_participant_names;
};

struct DdsParticipantWithBuiltInTopicListener {
    DdsParticipantWithBuiltInTopicListener(const std::string& system_name, uint32_t domain_id)
    {
        /// @brief taken from
        /// https://github.com/rticommunity/rticonnextdds-examples/blob/master/examples/connext_dds/builtin_topics/c%2B%2B11/msg_publisher.cxx
        /// with some adaptations
        /// Copyright https://github.com/rticommunity/rticonnextdds-examples/blob/master/LICENSE
        auto participant_qos = dds::core::QosProvider::Default().participant_qos();
        participant_qos.extensions().property.set(
            {"dds.domain_participant.domain_tag", system_name});
        _test_participant =
            std::make_unique<dds::all::DomainParticipant>(domain_id, participant_qos);

        _built_in_subscriber = std::make_unique<dds::sub::Subscriber>(
            dds::sub::builtin_subscriber(*_test_participant));

        _participant_listener =
            std::make_shared<BuiltinParticipantListener>(_wait_for_builtin_topic_reception);

        dds::sub::find<dds::sub::DataReader<dds::topic::ParticipantBuiltinTopicData>>(
            *_built_in_subscriber,
            dds::topic::participant_topic_name(),
            std::back_inserter(_participant_reader));

        if (_participant_reader.empty())
            throw std::runtime_error("No reader for built in topic found");

        _participant_reader[0].set_listener(_participant_listener);

        _test_participant->enable();
    }

    std::vector<std::string> waitForBuiltInTopics()
    {
        _wait_for_builtin_topic_reception.waitForNotification();
        return _participant_listener->getReceivedParticipantNames();
    }

    std::unique_ptr<dds::all::DomainParticipant> _test_participant;
    std::unique_ptr<dds::sub::Subscriber> _built_in_subscriber;
    std::vector<dds::sub::DataReader<dds::topic::ParticipantBuiltinTopicData>> _participant_reader;
    std::shared_ptr<BuiltinParticipantListener> _participant_listener;
    fep3::native::NotificationWaiting _wait_for_builtin_topic_reception;
};

class DDSParticipantNameTest : public TestConnextDDSSimulationBus {
protected:
    DDSParticipantNameTest()
        : _system_name(makePlatformDepName("testSystem")),
          _participant_name(makePlatformDepName("testParticipant")),
          _domain_id(randomDomainId()),
          _dds_participant(_system_name, _domain_id)
    {
    }

    void SetUp() override
    {
        _sim_bus = createSimulationBus(_domain_id, _participant_name, _system_name);

        startReception(dynamic_cast<ISimulationBus*>(_sim_bus.get()));
    }

    void TearDown() override
    {
        stopReception(dynamic_cast<ISimulationBus*>(_sim_bus.get()));
        TearDownComponent(*(_sim_bus.get()));
    }

    std::shared_ptr<fep3::IComponent> _sim_bus;
    const std::string _system_name;
    const std::string _participant_name;
    const uint32_t _domain_id;
    DdsParticipantWithBuiltInTopicListener _dds_participant;
};

TEST_F(DDSParticipantNameTest, DomainParticipantName)
{
    std::vector<std::string> received_dds_participant_names =
        _dds_participant.waitForBuiltInTopics();

    ASSERT_THAT(received_dds_participant_names, ::testing::Contains(_participant_name));
}
