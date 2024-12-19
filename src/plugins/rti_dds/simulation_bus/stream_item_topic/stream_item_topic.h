/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "../topic_intf.h"

#include <future>

#define FEP3_QOS_PROFILE_PREFIX "fep3::"
#define FEP3_BIG_QOS_PROFILE_POSTFIX "_big"
#define FEP3_QOS_STREAM_TYPE FEP3_QOS_PROFILE_PREFIX "stream_type"
#define FEP3_QOS_PARTICIPANT FEP3_QOS_PROFILE_PREFIX "participant"
#define FEP3_DEFAULT_QOS_PROFILE FEP3_QOS_PROFILE_PREFIX "default_profile"
#define FEP3_LARGEDATA_QOS_PROFILE_NAME "largedata"

// RTI DDS uses UDP by default and requires samples to fit into an UDP package (~64KB).
// To provide buffer for extra overhead (e.g. RTI DDS adds message size overhead) the threshold
// to switch the QOS profile used is less than the maximum size.
// If the threshold is reached or exceeded, RTI DDS requires asynchronous transmission and
// therefore specific QOS profiles.
// The max message size shall be computed from USER_QOS_PROFILE.xml after ticket FEPSDK-2756.
#define FEP3_TRANSPORT_LAYER_MAX_MESSAGE_SIZE 63000

class StreamItemTopic : public std::enable_shared_from_this<StreamItemTopic>, public ITopic {
public:
    StreamItemTopic(dds::domain::DomainParticipant& participant,
                    const std::string& topic_name,
                    const fep3::IStreamType& stream_type,
                    const std::shared_ptr<dds::core::QosProvider>& qos_provider,
                    const std::shared_ptr<fep3::ILogger> logger);

    std::string GetTopic() override;

    std::string findStreamTypeQosProfile(const fep3::IStreamType& stream_type);
    std::string findSampleQosProfile(const fep3::IStreamType& stream_type);

    std::unique_ptr<fep3::ISimulationBus::IDataReader> createDataReader(
        size_t queue_capacity,
        const std::weak_ptr<fep3::base::SimulationDataAccessCollection<ReaderItemQueue>>&
            data_access_collection) override;
    std::unique_ptr<fep3::ISimulationBus::IDataWriter> createDataWriter(
        size_t queue_capacity) override;

    dds::domain::DomainParticipant& getDomainParticipant();
    dds::topic::Topic<dds::core::BytesTopicType> getSampleTopic();
    dds::topic::Topic<fep3::ddstypes::StreamType> getStreamTypeTopic();
    std::shared_ptr<dds::core::QosProvider> getQosProvider();
    std::string getStreamTypeQosProfile();
    std::string getSampleQosProfile();

    bool updateStreamType(const fep3::IStreamType& stream_type);
    bool waitForConnectingWriters(std::chrono::nanoseconds timeout);
    void onSubscription(int count);

private:
    bool containsProfile(const std::string& profile_name);

private:
    dds::domain::DomainParticipant& _participant;
    std::unique_ptr<dds::topic::Topic<dds::core::BytesTopicType>> _sample_topic;
    std::unique_ptr<dds::topic::Topic<fep3::ddstypes::StreamType>> _stream_type_topic;
    std::string _topic_name;
    std::string _sample_qos_profile;
    std::string _stream_type_qos_profile;
    fep3::base::StreamType _stream_type;
    std::shared_ptr<dds::core::QosProvider> _qos_provider;
    std::shared_ptr<fep3::ILogger> _logger;
    std::promise<void> _writers_num_promise;
    std::atomic_bool _writers_num_should_notify = {true};
};
