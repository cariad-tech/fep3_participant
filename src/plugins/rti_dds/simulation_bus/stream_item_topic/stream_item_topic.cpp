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

#include "stream_item_reader.h"
#include "stream_item_writer.h"

#include <fep3/base/stream_type/default_stream_type.h>

#include <a_util/filesystem/filesystem.h>
#include <ddl/dd/ddstring.h>

using namespace fep3;
using namespace dds::domain;
using a_util::strings::format;

namespace {

bool isBigStreamTypePlain(const fep3::IStreamType& stream_type)
{
    const auto max_byte_size_prop =
        stream_type.getProperty(fep3::base::arya::meta_type_prop_name_max_byte_size);
    if (!max_byte_size_prop.empty()) {
        try {
            if (std::stoi(max_byte_size_prop) >= FEP3_TRANSPORT_LAYER_MAX_MESSAGE_SIZE) {
                return true;
            }
        }
        catch (const std::exception&) {
        }
    }

    // Need to compute size of all possible array types here. See FEPSDK-2934
    // Remove lines below in the future. Just an approximation.
    if (stream_type.getMetaTypeName() == fep3::base::arya::meta_type_plain_array.getName()) {
        const auto max_array_size_prop =
            stream_type.getProperty(fep3::base::arya::meta_type_prop_name_max_array_size);

        if (!max_array_size_prop.empty()) {
            // Assume 8 Bytes for the biggest plain type
            try {
                if (std::stoi(max_array_size_prop) * 8 >= FEP3_TRANSPORT_LAYER_MAX_MESSAGE_SIZE) {
                    return true;
                }
            }
            catch (const std::exception&) {
                return false;
            }
        }
    }
    return false;
}

bool isBigStreamTypeDDL(const fep3::IStreamType& stream_type)
{
    const auto stream_meta_type_name = stream_type.getMetaTypeName();
    const auto struct_name =
        stream_type.getProperty(fep3::base::arya::meta_type_prop_name_ddlstruct);
    if (struct_name.empty()) {
        return false;
    }
    std::string ddl_description;

    ddl_description = stream_type.getProperty(fep3::base::arya::meta_type_prop_name_ddldescription);

    if (ddl_description.empty()) {
        const auto ddl_fileref =
            stream_type.getProperty(fep3::base::arya::meta_type_prop_name_ddlfileref);

        if (a_util::filesystem::readTextFile(ddl_fileref, ddl_description) !=
            a_util::filesystem::OK) {
            return false;
        }
    }

    size_t struct_size_bit;

    try {
        struct_size_bit = ddl::DDString::fromXMLString(ddl_description)
                              .getStructTypeAccess(struct_name)
                              .getStaticSerializedBitSize();
    }
    catch (const ddl::dd::Error& /*error*/) {
        return false;
    }

    const auto max_message_size_bit = FEP3_TRANSPORT_LAYER_MAX_MESSAGE_SIZE * 8;

    if (struct_size_bit >= max_message_size_bit) {
        return true;
    }

    const auto max_arraysize_prop =
        stream_type.getProperty(fep3::base::arya::meta_type_prop_name_max_array_size);
    if (!max_arraysize_prop.empty()) {
        try {
            if (std::stoi(max_arraysize_prop) * struct_size_bit >= max_message_size_bit) {
                return true;
            }
        }
        catch (const std::exception&) {
            return false;
        }
    }

    return false;
}

} // namespace

StreamItemTopic::StreamItemTopic(DomainParticipant& participant,
                                 const std::string& topic_name,
                                 const IStreamType& stream_type,
                                 const std::shared_ptr<dds::core::QosProvider>& qos_provider,
                                 const std::shared_ptr<fep3::ILogger> logger)
    : _participant(participant),
      _topic_name(topic_name),
      _stream_type(stream_type),
      _qos_provider(qos_provider),
      _logger(logger)
{
    _stream_type_qos_profile = findStreamTypeQosProfile(stream_type);
    _sample_qos_profile = findSampleQosProfile(stream_type);

    if (logger->isDebugEnabled()) {
        logger->logDebug(format("Using qos profile '%s' for stream type topic '%s'.",
                                _stream_type_qos_profile.c_str(),
                                _topic_name.c_str()));
        logger->logDebug(format("Using qos profile '%s' for sample topic '%s'.",
                                _sample_qos_profile.c_str(),
                                _topic_name.c_str()));
    }

    _sample_topic =
        std::make_unique<dds::topic::Topic<dds::core::BytesTopicType>>(participant, topic_name);
    _stream_type_topic = std::make_unique<dds::topic::Topic<fep3::ddstypes::StreamType>>(
        participant, topic_name + "_stream_type");
}

std::string StreamItemTopic::GetTopic()
{
    return _topic_name;
}

bool StreamItemTopic::updateStreamType(const fep3::IStreamType& stream_type)
{
    if (!(stream_type == _stream_type)) {
        auto qos_profile = findSampleQosProfile(stream_type);
        if (qos_profile != _sample_qos_profile) {
            if (_logger->isDebugEnabled()) {
                _logger->logDebug(format("Update qos profile for topic '%s' from '%s' to '%s'.",
                                         _topic_name.c_str(),
                                         _sample_qos_profile.c_str(),
                                         qos_profile.c_str()));
            }

            _sample_qos_profile = qos_profile;
            return true;
        }
    }
    return false;
}

bool StreamItemTopic::waitForConnectingWriters(std::chrono::nanoseconds timeout)
{
    auto future = _writers_num_promise.get_future();
    return future.wait_for(std::chrono::nanoseconds(timeout)) == std::future_status::ready;
}

void StreamItemTopic::onSubscription(int count)
{
    if (count > 0 && _writers_num_should_notify.exchange(false)) {
        _writers_num_promise.set_value();
    }
}

std::string StreamItemTopic::findStreamTypeQosProfile(const fep3::IStreamType& stream_type)
{
    size_t stream_type_size{0};
    for (const auto& property_name: stream_type.getPropertyNames()) {
        stream_type_size += stream_type.getProperty(property_name).size();
    }

    if (stream_type_size >= FEP3_TRANSPORT_LAYER_MAX_MESSAGE_SIZE) {
        const auto big_stream_type_qos_profile_name =
            std::string(FEP3_QOS_STREAM_TYPE) + FEP3_BIG_QOS_PROFILE_POSTFIX;
        if (_logger->isDebugEnabled()) {
            _logger->logDebug(format(
                "Content of stream type '%s' of topic '%s' exceeds max transport limit of '%d'."
                "Qos profile '%s' will be used.",
                stream_type.getMetaTypeName().c_str(),
                _topic_name.c_str(),
                FEP3_TRANSPORT_LAYER_MAX_MESSAGE_SIZE,
                big_stream_type_qos_profile_name.c_str()));
        }

        return big_stream_type_qos_profile_name;
    }

    return FEP3_QOS_STREAM_TYPE;
}

std::string StreamItemTopic::findSampleQosProfile(const fep3::IStreamType& stream_type)
{
    auto stream_meta_type_name = stream_type.getMetaTypeName();

    if (isBigStreamTypePlain(stream_type) || isBigStreamTypeDDL(stream_type)) {
        const auto big_qos_profile_name =
            stream_meta_type_name + std::string(FEP3_BIG_QOS_PROFILE_POSTFIX);

        if (containsProfile(big_qos_profile_name)) {
            const auto qos_profile_name =
                std::string(FEP3_QOS_PROFILE_PREFIX) + big_qos_profile_name;
            if (_logger->isDebugEnabled()) {
                _logger->logDebug(format("Size of sample described by stream_type '%s' for "
                                         "topic '%s' exceeds max transport limit of '%d'."
                                         "Qos profile '%s' will be used.",
                                         stream_meta_type_name.c_str(),
                                         _topic_name.c_str(),
                                         FEP3_TRANSPORT_LAYER_MAX_MESSAGE_SIZE,
                                         qos_profile_name.c_str()));
            }

            return qos_profile_name;
        }
    }

    if (containsProfile(stream_meta_type_name)) {
        return std::string(FEP3_QOS_PROFILE_PREFIX) + stream_meta_type_name;
    }
    else {
        if (_logger->isWarningEnabled()) {
            _logger->logWarning(format("MetaType '%s' not defined in USER_QOS_PROFILES.xml. Using "
                                       "'" FEP3_DEFAULT_QOS_PROFILE "'.",
                                       stream_meta_type_name.c_str()));
        }
    }

    return FEP3_DEFAULT_QOS_PROFILE;
}

bool StreamItemTopic::containsProfile(const std::string& profile_name)
{
    auto qos_profiles = _qos_provider->extensions().qos_profiles("fep3");
    return std::find(qos_profiles.begin(), qos_profiles.end(), profile_name) != qos_profiles.end();
}

std::unique_ptr<ISimulationBus::IDataReader> StreamItemTopic::createDataReader(
    size_t queue_capacity,
    const std::weak_ptr<fep3::base::SimulationDataAccessCollection<ReaderItemQueue>>&
        data_access_collection)
{
    return std::make_unique<StreamItemDataReader>(
        this->shared_from_this(), queue_capacity, data_access_collection, _logger);
}

std::unique_ptr<ISimulationBus::IDataWriter> StreamItemTopic::createDataWriter(
    size_t queue_capacity)
{
    auto writer = std::make_unique<StreamItemDataWriter>(
        this->shared_from_this(), queue_capacity, _qos_provider);
    writer->write(_stream_type);
    return writer;
}

dds::domain::DomainParticipant& StreamItemTopic::getDomainParticipant()
{
    return _participant;
}

dds::topic::Topic<dds::core::BytesTopicType> StreamItemTopic::getSampleTopic()
{
    return *_sample_topic;
}

dds::topic::Topic<fep3::ddstypes::StreamType> StreamItemTopic::getStreamTypeTopic()
{
    return *_stream_type_topic;
}

std::shared_ptr<dds::core::QosProvider> StreamItemTopic::getQosProvider()
{
    return _qos_provider;
}

std::string StreamItemTopic::getStreamTypeQosProfile()
{
    return _stream_type_qos_profile;
}

std::string StreamItemTopic::getSampleQosProfile()
{
    return _sample_qos_profile;
}
