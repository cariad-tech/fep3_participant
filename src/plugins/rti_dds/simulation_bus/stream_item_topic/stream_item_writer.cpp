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


#include <plugins/rti_dds/simulation_bus/vector_raw_memory.hpp>
#include <plugins/rti_dds/simulation_bus/converter.h>

#include "stream_item_writer.h"

using namespace dds::core;
using namespace dds::pub;
using namespace dds::pub::qos;

using namespace fep3;

StreamItemDataWriter::StreamItemDataWriter(const std::shared_ptr<StreamItemTopic> & topic
    , size_t /*queue_capacity*/
    , const std::shared_ptr<dds::core::QosProvider> & qos_provider)
    : _topic(topic)
    , _publisher(topic->getDomainParticipant(), qos_provider->publisher_qos(FEP3_QOS_PARTICIPANT))
{
    const auto qos = qos_provider->datawriter_qos(topic->getQosProfile());
    createWriter(qos);

    _stream_type_writer = std::make_unique<DataWriter<fep3::ddstypes::StreamType>>(
        _publisher
        , topic->getStreamTypeTopic()
        , qos_provider->datawriter_qos(FEP3_QOS_STREAM_TYPE));
}

StreamItemDataWriter::~StreamItemDataWriter()
{
    _stream_type_writer->close();
    _sample_writer->close();
}

struct BytesTopicTypeRawMemory : public fep3::arya::IRawMemory
{

public:
    BytesTopicTypeRawMemory(dds::core::BytesTopicType & value) : _value(value)
    {
    }
    size_t capacity() const override
    {
        return _value.data().capacity();
    }
    const void* cdata() const override
    {
        return nullptr;
    }
    size_t size() const override
    {
        return _value.data().size();
    }
    size_t set(const void* data, size_t data_size) override
    {
        std::vector<uint8_t> buffer(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + data_size);
        _value.data(buffer);
        return data_size;
    }
    size_t resize(size_t data_size) override
    {
        return data_size;
    }

private:
    dds::core::BytesTopicType& _value;

};

void StreamItemDataWriter::createWriter(const dds::pub::qos::DataWriterQos & qos)
{
    if(_sample_writer)
    {
        _sample_writer->close();
    }

    _sample_writer = std::make_unique<DataWriter<dds::core::BytesTopicType>>(_publisher
        , _topic->getSampleTopic()
        , qos
        , this
        , dds::core::status::StatusMask::none());
}

fep3::Result StreamItemDataWriter::write(const IDataSample& data_sample)
{
    try
    {
        dds::core::BytesTopicType sample;
        BytesTopicTypeRawMemory raw_memory(sample);
        data_sample.read(raw_memory);

        _sample_writer->write(sample, convertTimestamp(data_sample.getTime()));
        
        return {};
    }
    catch (Exception & exception)
    {
        return convertExceptionToResult(exception);
    }

}

fep3::Result StreamItemDataWriter::write(const IStreamType& stream_type)
{
    try
    {
        fep3::ddstypes::StreamType dds_stream_type;
        dds_stream_type.metatype(stream_type.getMetaTypeName());

        for (auto property_name : stream_type.getPropertyNames())
        {
            dds_stream_type.properties().push_back(fep3::ddstypes::Property
                (property_name
                , stream_type.getPropertyType(property_name)
                , stream_type.getProperty(property_name)));
        }

        _stream_type_writer->write(dds_stream_type);

        if (_topic->updateStreamType(stream_type))
        {
            const auto qos = _topic->getQosProvider()->datawriter_qos(_topic->getQosProfile());
            createWriter(qos);
        }

        return {};
    }
    catch (Exception & exception)
    {
        return convertExceptionToResult(exception);
    }
}

fep3::Result StreamItemDataWriter::transmit()
{
    try
    {
        _stream_type_writer->extensions().flush();
        _sample_writer->extensions().flush();

        /*
        The behavior of transmit() is not finally defined. Maybe we need to block until data was send
        _stream_type_writer->wait_for_acknowledgments(dds::core::Duration(0, 1000000));
        _sample_writer->wait_for_acknowledgments(dds::core::Duration(0, 1000000));
        */

        return {};
    }
    catch (Exception & exception)
    {
        return convertExceptionToResult(exception);
    }

}

void StreamItemDataWriter::on_offered_deadline_missed(
    dds::pub::DataWriter<dds::core::BytesTopicType>& /*writer*/,
    const dds::core::status::OfferedDeadlineMissedStatus& /*status*/)
{
    //std::cout << "           on_offered_deadline_missed" << std::endl;
}
void StreamItemDataWriter::on_offered_incompatible_qos(
    dds::pub::DataWriter<dds::core::BytesTopicType>& /*writer*/,
    const dds::core::status::OfferedIncompatibleQosStatus& /*status*/)
{
    //std::cout << "           on_offered_incompatible_qos" << std::endl;
}
void StreamItemDataWriter::on_liveliness_lost(
    dds::pub::DataWriter<dds::core::BytesTopicType>& /*writer*/,
    const dds::core::status::LivelinessLostStatus& /*status*/)
{
    //std::cout << "           on_liveliness_lost" << std::endl;
}
void StreamItemDataWriter::on_publication_matched(
    dds::pub::DataWriter<dds::core::BytesTopicType>& /*writer*/,
    const dds::core::status::PublicationMatchedStatus& /*status*/)
{
    //std::cout << "           on_publication_matched" << std::endl;
}
void StreamItemDataWriter::on_reliable_writer_cache_changed(
    dds::pub::DataWriter<dds::core::BytesTopicType>& /*writer*/,
    const rti::core::status::ReliableWriterCacheChangedStatus& /*status*/)
{
    //std::cout << "           on_reliable_writer_cache_changed" << std::endl;
}
void StreamItemDataWriter::on_reliable_reader_activity_changed(
    dds::pub::DataWriter<dds::core::BytesTopicType>& /*writer*/,
    const rti::core::status::ReliableReaderActivityChangedStatus& /*status*/)
{
    //std::cout << "           on_reliable_reader_activity_changed" << std::endl;
}
void StreamItemDataWriter::on_instance_replaced(
    dds::pub::DataWriter<dds::core::BytesTopicType>& /*writer*/,
    const dds::core::InstanceHandle& /*handle*/)
{
    //std::cout << "           on_instance_replaced" << std::endl;
}
void StreamItemDataWriter::on_application_acknowledgment(
    dds::pub::DataWriter<dds::core::BytesTopicType>& /*writer*/,
    const rti::pub::AcknowledgmentInfo& /*acknowledgment_info*/)
{
    //std::cout << "on_application_acknowledgment" << std::endl;
}
void StreamItemDataWriter::on_service_request_accepted(
    dds::pub::DataWriter<dds::core::BytesTopicType>& /*writer*/,
    const rti::core::status::ServiceRequestAcceptedStatus& /*status*/)
{
    //std::cout << "           on_service_request_accepted" << std::endl;
}
void StreamItemDataWriter::on_destination_unreachable(
    dds::pub::DataWriter<dds::core::BytesTopicType>&,
    const dds::core::InstanceHandle&,
    const rti::core::Locator&)
{
    //std::cout << "           on_destination_unreachable" << std::endl;
}
void* StreamItemDataWriter::on_data_request(
    dds::pub::DataWriter<dds::core::BytesTopicType>&,
    const rti::core::Cookie&)
{
    //std::cout << "           on_data_request" << std::endl;
    return nullptr;
}
void StreamItemDataWriter::on_data_return(
    dds::pub::DataWriter<dds::core::BytesTopicType>&,
    void*,
    const rti::core::Cookie&)
{
    //std::cout << "           on_data_return" << std::endl;
}
void StreamItemDataWriter::on_sample_removed(
    dds::pub::DataWriter<dds::core::BytesTopicType>& /*writer*/,
    const rti::core::Cookie& /*cookie*/)
{
    //std::cout << "           on_sample_removed" << std::endl;
}
