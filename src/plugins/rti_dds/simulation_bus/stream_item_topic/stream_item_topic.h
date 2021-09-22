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

#include <plugins/rti_dds/simulation_bus/rti_conext_dds_include.h>
#include <plugins/rti_dds/types/stream_types.hpp>
#include <plugins/rti_dds/simulation_bus/topic_intf.h>
#include <fep3/components/logging/logger_intf.h>

#define FEP3_QOS_STREAM_TYPE "fep3::stream_type"
#define FEP3_QOS_PARTICIPANT "fep3::participant"
#define FEP3_DEFAULT_QOS_PROFILE "fep3::default_profile"
#define FEP3_LARGEDATA_QOS_PROFILE_NAME "largedata"

// Will be compute from USER_QOS_PROFILE.xml after ticket FEPSDK-2756
#define FEP3_TRANSPORT_LAYER_MAX_MESSAGE_SIZE 64000

class StreamItemTopic :
    public std::enable_shared_from_this<StreamItemTopic>,
    public ITopic
{

public:
    StreamItemTopic(dds::domain::DomainParticipant & participant
        , const std::string & topic_name
        , const fep3::IStreamType& stream_type
        , const std::shared_ptr<dds::core::QosProvider> & qos_provider
        , const std::shared_ptr<fep3::ILogger> logger);

    std::string GetTopic();

    std::string findQosProfile(const fep3::IStreamType& stream_type);

    std::unique_ptr<fep3::ISimulationBus::IDataReader> createDataReader
        (size_t queue_capacity
        , const std::weak_ptr<fep3::base::SimulationDataAccessCollection<ReaderItemQueue>>& data_access_collection
        );
    std::unique_ptr<fep3::ISimulationBus::IDataWriter> createDataWriter(size_t queue_capacity);

    dds::domain::DomainParticipant & getDomainParticipant();
    dds::topic::Topic<dds::core::BytesTopicType> getSampleTopic();
    dds::topic::Topic<fep3::ddstypes::StreamType> getStreamTypeTopic();
    std::shared_ptr<dds::core::QosProvider> getQosProvider();
    std::string getQosProfile();

    bool updateStreamType(const fep3::IStreamType& stream_type);

private:
    bool isBigStreamType(const fep3::IStreamType& stream_type);
    bool containsProfile(const std::string & profile_name);

private:
    dds::domain::DomainParticipant & _participant;
    std::unique_ptr<dds::topic::Topic<dds::core::BytesTopicType>> _sample_topic;
    std::unique_ptr<dds::topic::Topic<fep3::ddstypes::StreamType>> _stream_type_topic;
    std::string _topic_name;
    std::string _qos_profile;
    fep3::base::StreamType  _stream_type;
    std::shared_ptr<dds::core::QosProvider> _qos_provider;
    std::shared_ptr<fep3::ILogger> _logger;
};
