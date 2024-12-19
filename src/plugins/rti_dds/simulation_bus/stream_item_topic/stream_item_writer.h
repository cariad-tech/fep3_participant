/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "stream_item_topic.h"

class StreamItemDataWriter : public fep3::ISimulationBus::IDataWriter,
                             public dds::pub::DataWriterListener<dds::core::BytesTopicType> {
public:
    StreamItemDataWriter(const std::shared_ptr<StreamItemTopic>& topic,
                         size_t queue_capacity,
                         const std::shared_ptr<dds::core::QosProvider>& qos_provider);
    ~StreamItemDataWriter();

    fep3::Result write(const fep3::IDataSample& data_sample) override;
    fep3::Result write(const fep3::IStreamType& stream_type) override;
    fep3::Result transmit() override;

    void createWriter(const dds::pub::qos::DataWriterQos& qos);

protected:
    void on_offered_deadline_missed(
        dds::pub::DataWriter<dds::core::BytesTopicType>& writer,
        const dds::core::status::OfferedDeadlineMissedStatus& status) override;

    void on_offered_incompatible_qos(
        dds::pub::DataWriter<dds::core::BytesTopicType>& writer,
        const dds::core::status::OfferedIncompatibleQosStatus& status) override;

    void on_liveliness_lost(dds::pub::DataWriter<dds::core::BytesTopicType>& writer,
                            const dds::core::status::LivelinessLostStatus& status) override;

    void on_publication_matched(dds::pub::DataWriter<dds::core::BytesTopicType>& writer,
                                const dds::core::status::PublicationMatchedStatus& status) override;

    void on_reliable_writer_cache_changed(
        dds::pub::DataWriter<dds::core::BytesTopicType>& writer,
        const rti::core::status::ReliableWriterCacheChangedStatus& status) override;

    void on_reliable_reader_activity_changed(
        dds::pub::DataWriter<dds::core::BytesTopicType>& writer,
        const rti::core::status::ReliableReaderActivityChangedStatus& status) override;

    void on_instance_replaced(dds::pub::DataWriter<dds::core::BytesTopicType>& writer,
                              const dds::core::InstanceHandle& handle) override;

    void on_application_acknowledgment(
        dds::pub::DataWriter<dds::core::BytesTopicType>& writer,
        const rti::pub::AcknowledgmentInfo& acknowledgment_info) override;

    void on_service_request_accepted(
        dds::pub::DataWriter<dds::core::BytesTopicType>& writer,
        const rti::core::status::ServiceRequestAcceptedStatus& status) override;

    void on_destination_unreachable(dds::pub::DataWriter<dds::core::BytesTopicType>&,
                                    const dds::core::InstanceHandle&,
                                    const rti::core::Locator&) override;

    void* on_data_request(dds::pub::DataWriter<dds::core::BytesTopicType>&,
                          const rti::core::Cookie&) override;

    void on_data_return(dds::pub::DataWriter<dds::core::BytesTopicType>&,
                        void*,
                        const rti::core::Cookie&) override;

    void on_sample_removed(dds::pub::DataWriter<dds::core::BytesTopicType>& writer,
                           const rti::core::Cookie& cookie) override;

private:
    std::shared_ptr<StreamItemTopic> _topic;
    dds::pub::Publisher _publisher;
    std::unique_ptr<dds::pub::DataWriter<fep3::ddstypes::StreamType>> _stream_type_writer;
    std::unique_ptr<dds::pub::DataWriter<dds::core::BytesTopicType>> _sample_writer;
};
