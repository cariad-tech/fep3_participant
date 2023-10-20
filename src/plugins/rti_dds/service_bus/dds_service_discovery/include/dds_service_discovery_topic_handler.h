/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */
#pragma once

#include "DdsServiceDiscoveryTopic.hpp"
#include "dds_service_discovery_participant.h"

namespace fep3 {
namespace native {

template <typename TopicType>
std::unique_ptr<dds::sub::DataReader<TopicType>> createReader(
    const dds::sub::Subscriber& subscriber,
    std::string topic_name,
    const DdsSdParticipant& dds_sd_participant)
{
    dds::sub::qos::DataReaderQos reader_qos = dds::core::QosProvider::Default().datareader_qos();
    reader_qos << dds::core::policy::Durability(
        dds::core::policy::DurabilityKind_def::TRANSIENT_LOCAL);
    reader_qos << dds::core::policy::Reliability(dds::core::policy::ReliabilityKind::RELIABLE);

    reader_qos << dds::core::policy::History(dds::core::policy::HistoryKind::KEEP_LAST);

    return std::make_unique<dds::sub::DataReader<TopicType>>(
        subscriber, dds_sd_participant.getTopic<TopicType>(topic_name), reader_qos);
}

class IDSTopicHandler {
public:
    virtual ~IDSTopicHandler() = default;
    virtual void sampleReceived(const DdsServiceDiscovery& participant_data) = 0;
};

template <typename T>
class DdsSdTopicListener : public dds::sub::NoOpDataReaderListener<T> {
public:
    DdsSdTopicListener(IDSTopicHandler* pIDSTopicHandler) : _pIDSTopicHandler(pIDSTopicHandler)
    {
    }

    void on_data_available(dds::sub::DataReader<T>& reader) override
    {
        rti::sub::LoanedSamples<T> samples = reader.take();
        for (rti::sub::LoanedSample<T> sample: samples) {
            if (sample.info().state().instance_state() ==
                dds::sub::status::InstanceState::alive()) {
                _pIDSTopicHandler->sampleReceived(sample.data());
            }
            else {
                T my_sample;
                reader.key_value(my_sample, sample.info().instance_handle());
                // TODO: inform about error condition?
            }
        }
    }

private:
    IDSTopicHandler* _pIDSTopicHandler;
};

template <typename T>
class DDSTopicHandlerImpl : public IDSTopicHandler {
public:
    DDSTopicHandlerImpl(std::function<void(const DdsServiceDiscovery&)> callback_function,
                        std::string topic_name,
                        const DdsSdParticipant& dds_sd_participant)
        : _subscriber(dds_sd_participant.getDomainParticipant()),
          _reader(createReader<T>(_subscriber, std::move(topic_name), dds_sd_participant)),
          _callback(callback_function)
    {
        _listener = std::move(std::make_unique<DdsSdTopicListener<T>>(this));
        _reader->listener(_listener.get(), dds::core::status::StatusMask::data_available());
    }

    ~DDSTopicHandlerImpl()
    {
        // otherwise reader will try to report to a dangling listener
        _reader->listener(nullptr, dds::core::status::StatusMask::all());
        _reader->close();
        _reader.reset();
        _listener.reset();
        _subscriber.close();
    }

    void sampleReceived(const DdsServiceDiscovery& participant_data)
    {
        _callback(participant_data);
    }

private:
    dds::sub::Subscriber _subscriber;
    std::unique_ptr<DdsSdTopicListener<T>> _listener;
    std::unique_ptr<dds::sub::DataReader<T>> _reader;

    std::function<void(const DdsServiceDiscovery&)> _callback;
    static uint32_t _creation_counter;
};

class DDSTopicHandler {
public:
    template <typename T>
    DDSTopicHandler(std::function<void(const DdsServiceDiscovery&)> callback_function,
                    std::string topic_name,
                    const DdsSdParticipant& dds_sd_participant,
                    T&&)
        : _dds_topic_handler(std::make_unique<DDSTopicHandlerImpl<T>>(
              callback_function, topic_name, dds_sd_participant))
    {
    }

private:
    std::unique_ptr<IDSTopicHandler> _dds_topic_handler;
};

} // namespace native
} // namespace fep3
