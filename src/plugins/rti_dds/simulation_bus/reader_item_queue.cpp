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

#include "converter.h"
#include "stream_item_topic/stream_item_topic.h"

#include <fep3/base/sample/data_sample.h>

#include <a_util/result.h>

std::shared_ptr<fep3::IStreamType> createStreamType(
    const fep3::ddstypes::StreamType& dds_streamtype, const dds::sub::SampleInfo& /*sample_info*/)
{
    auto streamtype = std::make_shared<fep3::base::StreamType>(dds_streamtype.metatype());
    for (auto dds_property: dds_streamtype.properties()) {
        streamtype->setProperty(dds_property.name(), dds_property.value(), dds_property.type());
    }
    return streamtype;
}

std::shared_ptr<fep3::IDataSample> createSample(const dds::core::BytesTopicType& dds_sample,
                                                const dds::sub::SampleInfo& sample_info)
{
    auto sample = std::make_shared<fep3::base::DataSample>();
    sample->set(dds_sample.data().data(), dds_sample.data().size());
    sample->setTime(convertTimestamp(sample_info.source_timestamp()));
    sample->setCounter(
        static_cast<uint32_t>(sample_info.extensions().publication_sequence_number().value()));
    return sample;
}

ReaderItemQueue::ReaderItemQueue(const std::shared_ptr<fep3::ILogger>& logger,
                                 const std::shared_ptr<StreamItemTopic>& topic)
    : _logger(logger),
      _topic(topic),
      _subscriber(topic->getDomainParticipant(),
                  topic->getQosProvider()->subscriber_qos(FEP3_QOS_PARTICIPANT))
{
    createReader(topic->getQosProvider()->datareader_qos(topic->getSampleQosProfile()));

    _streamtype_reader = std::make_unique<dds::sub::DataReader<fep3::ddstypes::StreamType>>(
        _subscriber,
        topic->getStreamTypeTopic(),
        topic->getQosProvider()->datareader_qos(topic->getStreamTypeQosProfile()));
}

ReaderItemQueue::~ReaderItemQueue()
{
    _sample_reader->close();
    _streamtype_reader->close();
    _subscriber.close();
}

void ReaderItemQueue::createReader(const dds::sub::qos::DataReaderQos& qos)
{
    if (_sample_reader) {
        // We are calling into ConnextDDSSimulationBus::startBlockingReception to be released from
        // the waitset no synchronization needed because we are in the same thread
        if (_release_reader_conditions) {
            _release_reader_conditions();
        }

        // Now we need to close the old reader
        // If not closable, we have to return
        // AsyncWaitSet will close it while updating waitset
        if (!closeReader()) {
            return;
        }
    }

    // At this point we are overriding the existing reader, but it will not be deleted
    // because it's still part of the ReadCondition. But we are not affected from the old
    // reader because we have closed him.
    _sample_reader = std::make_unique<dds::sub::DataReader<dds::core::BytesTopicType>>(
        _subscriber,
        _topic->getSampleTopic(),
        qos,
        this,
        dds::core::status::StatusMask::subscription_matched());
}

bool ReaderItemQueue::closeReader()
{
    try {
        if (_sample_reader) {
            // AsyncWaitSet may not close it while dispatching.
            // It will close it while updating waitset next time.
            _sample_reader->close();
        }
        return true;
    }
    catch (const std::exception& exception) {
        if (_logger && _logger->isWarningEnabled()) {
            _logger->logWarning(
                a_util::strings::format("RTI DDS can not close reader: '%s'", exception.what()));
        }
        return false;
    }
}

void ReaderItemQueue::createReader()
{
    createReader(_topic->getQosProvider()->datareader_qos(_topic->getSampleQosProfile()));
}

void ReaderItemQueue::setRecreateWaitSetCondition(
    const std::function<void()>& release_reader_conditions)
{
    _release_reader_conditions = release_reader_conditions;
}

size_t ReaderItemQueue::size() const
{
    try {
        return _sample_reader->extensions().datareader_cache_status().sample_count();
    }
    catch (dds::core::Exception& exception) {
        logError(convertExceptionToResult(exception));
    }
    return 0;
}

size_t ReaderItemQueue::capacity() const
{
    try {
        return _sample_reader->qos().delegate().history.depth();
    }
    catch (dds::core::Exception& exception) {
        logError(convertExceptionToResult(exception));
    }
    return 0;
}

void ReaderItemQueue::logError(const fep3::Result& res) const
{
    if (_logger) {
        if (_logger->isErrorEnabled()) {
            _logger->logError(a_util::result::toString(res));
        }
    }
}

bool ReaderItemQueue::pop(fep3::ISimulationBus::IDataReceiver& receiver)
{
    try {
        return popFrom(*_sample_reader.get(), *_streamtype_reader.get(), _subscriber, receiver);
    }
    catch (dds::core::Exception& exception) {
        logError(convertExceptionToResult(exception));
    }
    return false;
}

bool ReaderItemQueue::popFrom(dds::sub::DataReader<dds::core::BytesTopicType>& sample_reader,
                              dds::sub::DataReader<fep3::ddstypes::StreamType>& streamtype_reader,
                              const dds::all::Subscriber& subscriber,
                              fep3::ISimulationBus::IDataReceiver& receiver)
{
    if (!sample_reader.is_nil()) {
        dds::sub::CoherentAccess coherent_access(subscriber);

        std::vector<dds::sub::AnyDataReader> readers;
        const int num_readers =
            find(subscriber, dds::sub::status::DataState::any(), std::back_inserter(readers));

        // readers contains a list of reader, i.e.:
        // _sample_reader
        // _sample_reader
        // _streamtype_reader
        // _sample_reader
        // _sample_reader
        // depending on the recieve order

        if (num_readers > 0) {
            if (readers[0] == sample_reader) {
                for (auto sample: sample_reader.select().max_samples(1).take()) {
                    receiver(createSample(sample, sample.info()));
                }
            }
            else {
                for (auto streamtype: streamtype_reader.select().max_samples(1).take()) {
                    auto stream_type = createStreamType(streamtype, streamtype.info());
                    receiver(stream_type);
                    if (_topic->updateStreamType(*stream_type)) {
                        // first read all samples
                        while (pop(receiver))
                            ;

                        // now recreate reader
                        createReader();
                    }
                }
            }
            return true;
        }
    }

    return false;
}

fep3::Optional<fep3::Timestamp> ReaderItemQueue::getFrontTime() const
{
    dds::sub::CoherentAccess coherent_access(_subscriber);

    std::vector<dds::sub::AnyDataReader> readers;
    int num_readers =
        find(_subscriber, dds::sub::status::DataState::new_data(), std::back_inserter(readers));

    if (num_readers > 0) {
        if (readers[0] == *_sample_reader) {
            auto sample = *_sample_reader->select().max_samples(1).read().begin();

            return convertTimestamp(sample.info().source_timestamp());
        }
        else {
            auto streamtype = *_streamtype_reader->select().max_samples(1).read().begin();

            return convertTimestamp(streamtype.info().source_timestamp());
        }
    }
    return {};
}

dds::core::cond::Condition ReaderItemQueue::createSampleReadCondition(
    const std::shared_ptr<fep3::ISimulationBus::IDataReceiver>& receiver)
{
    return dds::sub::cond::ReadCondition(
        *_sample_reader,
        dds::sub::status::SampleState::not_read(),
        [sample_reader = *_sample_reader.get(),
         streamtype_reader = *_streamtype_reader.get(),
         subscriber = _subscriber,
         receiver,
         this]() mutable {
            while (popFrom(sample_reader, streamtype_reader, subscriber, *receiver.get()))
                ;
        });
}

dds::core::cond::Condition ReaderItemQueue::createStreamTypeReadCondition(
    const std::shared_ptr<fep3::ISimulationBus::IDataReceiver>& receiver)
{
    return dds::sub::cond::ReadCondition(
        *_streamtype_reader,
        dds::sub::status::SampleState::not_read(),
        [sample_reader = *_sample_reader.get(),
         streamtype_reader = *_streamtype_reader.get(),
         subscriber = _subscriber,
         receiver,
         this]() mutable {
            while (popFrom(sample_reader, streamtype_reader, subscriber, *receiver.get()))
                ;
        });
}

void ReaderItemQueue::on_data_available(dds::sub::DataReader<dds::core::BytesTopicType>& /*reader*/)
{
}

void ReaderItemQueue::on_requested_deadline_missed(
    dds::sub::DataReader<dds::core::BytesTopicType>& /*reader*/,
    const dds::core::status::RequestedDeadlineMissedStatus& /*status*/)
{
    // std::cout << "           on_requested_deadline_missed" << std::endl;
}

void ReaderItemQueue::on_requested_incompatible_qos(
    dds::sub::DataReader<dds::core::BytesTopicType>& /*reader*/,
    const dds::core::status::RequestedIncompatibleQosStatus& /*status*/)
{
    // std::cout << "           on_requested_incompatible_qos" << std::endl;
}

void ReaderItemQueue::on_sample_rejected(
    dds::sub::DataReader<dds::core::BytesTopicType>& /*reader*/,
    const dds::core::status::SampleRejectedStatus& /*status*/)
{
    // std::cout << "           on_requested_incompatible_qos" << std::endl;
}

void ReaderItemQueue::on_liveliness_changed(
    dds::sub::DataReader<dds::core::BytesTopicType>& /*reader*/,
    const dds::core::status::LivelinessChangedStatus& /*status*/)
{
    // std::cout << "           on_requested_incompatible_qos" << std::endl;
}

void ReaderItemQueue::on_subscription_matched(
    dds::sub::DataReader<dds::core::BytesTopicType>& /*reader*/,
    const dds::core::status::SubscriptionMatchedStatus& status)
{
    // std::cout << "           on_subscription_matched" << std::endl;
    _topic->onSubscription(status.current_count());
}

void ReaderItemQueue::on_sample_lost(dds::sub::DataReader<dds::core::BytesTopicType>& /*reader*/,
                                     const dds::core::status::SampleLostStatus& /*status*/)
{
    // std::cout << "           on_sample_lost" << std::endl;
}
