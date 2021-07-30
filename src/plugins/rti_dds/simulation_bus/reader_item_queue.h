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

#include <fep3/base/sample/data_sample_intf.h>
#include <fep3/base/stream_type/stream_type_intf.h>
#include <fep3/components/simulation_bus/simulation_bus_intf.h>
#include <fep3/components/logging/logger_intf.h>

#include <dds/dds.hpp>
#include <plugins/rti_dds/types/stream_types.hpp>

std::shared_ptr<fep3::arya::IStreamType> createStreamType(const fep3::ddstypes::StreamType& dds_streamtype, const dds::sub::SampleInfo& sample_info);
std::shared_ptr<fep3::arya::IDataSample> createSample(const dds::core::BytesTopicType& dds_sample, const dds::sub::SampleInfo& sample_info);

class StreamItemTopic;

class ReaderItemQueue final
    : public dds::sub::DataReaderListener<dds::core::BytesTopicType>
{
public:
    ReaderItemQueue
        (const std::shared_ptr<fep3::ILogger> & logger
		, const std::shared_ptr<StreamItemTopic> & topic
        );
    ~ReaderItemQueue();
    size_t size() const;
    size_t capacity() const;
    bool pop(fep3::ISimulationBus::IDataReceiver& receiver);
    
    void setRecreateWaitSetCondition(const std::function<void()> & release_reader_conditions);

    fep3::Optional<fep3::Timestamp> getFrontTime() const;

    void createReader(const dds::sub::qos::DataReaderQos & qos);
    dds::core::cond::Condition createSampleReadCondition(const std::shared_ptr<fep3::ISimulationBus::IDataReceiver>& receiver);
    dds::core::cond::Condition createStreamTypeReadCondition(const std::shared_ptr<fep3::ISimulationBus::IDataReceiver>& receiver);

protected:
    void on_data_available(
        dds::sub::DataReader<dds::core::BytesTopicType>& reader);

    void on_requested_deadline_missed(
        dds::sub::DataReader<dds::core::BytesTopicType>& reader,
        const dds::core::status::RequestedDeadlineMissedStatus& status);

    void on_requested_incompatible_qos(
        dds::sub::DataReader<dds::core::BytesTopicType>& reader,
        const dds::core::status::RequestedIncompatibleQosStatus& status);

    void on_sample_rejected(
        dds::sub::DataReader<dds::core::BytesTopicType>& reader,
        const dds::core::status::SampleRejectedStatus& status);

    void on_liveliness_changed(
        dds::sub::DataReader<dds::core::BytesTopicType>& reader,
        const dds::core::status::LivelinessChangedStatus& status);

    void on_subscription_matched(
        dds::sub::DataReader<dds::core::BytesTopicType>& reader,
        const dds::core::status::SubscriptionMatchedStatus& status);

    void on_sample_lost(
        dds::sub::DataReader<dds::core::BytesTopicType>& reader,
        const dds::core::status::SampleLostStatus& status);

private:
    bool popFrom
        (dds::sub::DataReader<dds::core::BytesTopicType>& sample_reader
        , dds::sub::DataReader<fep3::ddstypes::StreamType>& streamtype_reader
        , const dds::all::Subscriber& subscriber
        , fep3::ISimulationBus::IDataReceiver& receiver
        );
    void logError(const fep3::Result& res) const;

private:
    std::shared_ptr<fep3::ILogger> _logger;
    std::shared_ptr<StreamItemTopic> _topic;
    dds::all::Subscriber _subscriber;

    std::function<void()> _release_reader_conditions;
    std::unique_ptr<dds::sub::DataReader<dds::core::BytesTopicType>> _sample_reader;
    std::unique_ptr<dds::sub::DataReader<fep3::ddstypes::StreamType>> _streamtype_reader;
    
};
