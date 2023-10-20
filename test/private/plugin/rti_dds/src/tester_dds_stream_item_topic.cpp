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

#include <fep3/base/environment_variable/include/environment_variable.h>
#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/base/stream_type/mock/mock_stream_type.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <stream_item_topic.h>

using namespace ::testing;
using dds::core::QosProvider;
using dds::domain::DomainParticipant;
using dds::domain::qos::DomainParticipantQos;
using LoggerMock = ::NiceMock<fep3::mock::LoggerWithDefaultBehavior>;
using StreamTypeMock = ::NiceMock<fep3::mock::StreamType>;

struct StreamItemTopicTest : public ::Test {
    StreamItemTopicTest()
        : _domain_participant(std::make_shared<DomainParticipant>(1, _domain_participant_qos)),
          _logger_mock(std::make_shared<LoggerMock>()),
          _stream_type_mock(std::make_shared<StreamTypeMock>())
    {
    }

    void SetUp() override
    {
        _qos_provider = std::make_shared<QosProvider>(QosProvider::Default());
        EXPECT_CALL(*_stream_type_mock.get(), getMetaTypeName()).WillRepeatedly(Invoke([]() {
            return fep3::base::arya::meta_type_raw.getName();
        }));
        EXPECT_CALL(*_stream_type_mock.get(), copyTo(_)).WillRepeatedly(Return());
        EXPECT_CALL(*_stream_type_mock.get(), getProperty(_)).WillRepeatedly(Return(""));
        _stream_item_topic = std::make_unique<StreamItemTopic>(
            *_domain_participant, _topic_name, *_stream_type_mock, _qos_provider, _logger_mock);
    }

    DomainParticipantQos _domain_participant_qos;
    std::shared_ptr<DomainParticipant> _domain_participant;
    std::shared_ptr<QosProvider> _qos_provider;
    std::shared_ptr<LoggerMock> _logger_mock;
    std::shared_ptr<StreamTypeMock> _stream_type_mock;
    const std::string _topic_name{"topic_name"};
    std::unique_ptr<StreamItemTopic> _stream_item_topic;
};

TEST_F(StreamItemTopicTest, CTOR)
{
    const auto topic_name = "test_topic_name";
    EXPECT_CALL(*_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Using qos profile 'fep3::anonymous' for sample topic 'test_topic_name'.")))
        .WillOnce(::Return(fep3::Result{}));
    EXPECT_CALL(
        *_logger_mock.get(),
        logDebug(fep3::mock::LogStringRegexMatcher(
            "Using qos profile 'fep3::stream_type' for stream type topic 'test_topic_name'.")))
        .WillOnce(::Return(fep3::Result{}));
    StreamItemTopic stream_item_topic(
        *_domain_participant, topic_name, *_stream_type_mock, _qos_provider, _logger_mock);
    EXPECT_EQ(*_domain_participant, stream_item_topic.getDomainParticipant());
    EXPECT_EQ(topic_name, stream_item_topic.GetTopic());
    EXPECT_EQ(_qos_provider, stream_item_topic.getQosProvider());
    EXPECT_EQ(std::string(FEP3_QOS_PROFILE_PREFIX) + fep3::base::arya::meta_type_raw.getName(),
              stream_item_topic.getSampleQosProfile());
}

TEST_F(StreamItemTopicTest, findSampleQosProfile__successfullSmallRawType)
{
    EXPECT_EQ(std::string(FEP3_QOS_PROFILE_PREFIX) + fep3::base::arya::meta_type_raw.getName(),
              _stream_item_topic->findSampleQosProfile(*_stream_type_mock));
}

TEST_F(StreamItemTopicTest, findSampleQosProfile__successfullBigStringType)
{
    EXPECT_CALL(*_stream_type_mock.get(), getMetaTypeName())
        .WillRepeatedly(Return(fep3::base::arya::meta_type_string.getName()));
    EXPECT_CALL(*_stream_type_mock.get(),
                getProperty(fep3::base::arya::meta_type_prop_name_max_byte_size))
        .WillRepeatedly(Return("63000"));
    EXPECT_CALL((*_logger_mock.get()),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Size of sample described by stream_type 'ascii-string' for topic 'topic_name' "
                    "exceeds max transport limit of '63000'."
                    "Qos profile 'fep3::ascii-string_big' will be used.")))
        .WillOnce(::Return(fep3::Result{}));

    EXPECT_EQ(std::string(FEP3_QOS_PROFILE_PREFIX) + fep3::base::arya::meta_type_string.getName() +
                  FEP3_BIG_QOS_PROFILE_POSTFIX,
              _stream_item_topic->findSampleQosProfile(*_stream_type_mock));
}

TEST_F(StreamItemTopicTest, findSampleQosProfile__successfullBigPlainArrayType)
{
    EXPECT_CALL(*_stream_type_mock.get(), getMetaTypeName())
        .WillRepeatedly(Return(fep3::base::arya::meta_type_plain_array.getName()));
    EXPECT_CALL(*_stream_type_mock.get(),
                getProperty(fep3::base::arya::meta_type_prop_name_max_array_size))
        .WillRepeatedly(Return("1000000"));
    EXPECT_CALL(*_stream_type_mock.get(),
                getProperty(fep3::base::arya::meta_type_prop_name_datatype))
        .WillRepeatedly(Return("uint32_t"));
    EXPECT_CALL(*_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Size of sample described by stream_type 'plain-array-ctype' for topic "
                    "'topic_name' exceeds max transport limit of '63000'."
                    "Qos profile 'fep3::plain-array-ctype_big' will be used.")))
        .WillOnce(::Return(fep3::Result{}));

    EXPECT_EQ(std::string(FEP3_QOS_PROFILE_PREFIX) +
                  fep3::base::arya::meta_type_plain_array.getName() + FEP3_BIG_QOS_PROFILE_POSTFIX,
              _stream_item_topic->findSampleQosProfile(*_stream_type_mock));
}

const auto struct_name_small = "tSmallStruct";
const auto struct_name_big = "tBigStruct";
const auto ddl_description = R"(<?xml version="1.0" encoding="utf-8" standalone="no"?>
<adtf:ddl xmlns:adtf="adtf">
<header>
<language_version>3.00</language_version>
<author>fep_team</author>
<date_creation>20.02.2020</date_creation>
<date_change>20.02.2020</date_change>
<description>Simplistic DDL for testing purposes</description>
</header>
<units />
<datatypes>
<datatype description="theoretical datatype for testing purposes that is smaller than FEP3_TRANSPORT_LAYER_MAX_MESSAGE_SIZE" name="smallDatatype" size="256000" />
<datatype description="theoretical datatype for testing purposes that is bigger than FEP3_TRANSPORT_LAYER_MAX_MESSAGE_SIZE" name="bigDatatype" size="512001" />
</datatypes>
<enums>
</enums>
<structs>
<struct alignment="1" name="tSmallStruct" version="1">
<element alignment="1" arraysize="1" byteorder="LE" bytepos="0" name="smallElement" type="smallDatatype" default="0"/>
</struct>
<struct alignment="1" name="tBigStruct" version="1">
<element alignment="1" arraysize="1" byteorder="LE" bytepos="0" name="bigElement" type="bigDatatype" default="0"/>
</struct>
</structs>
<streams />
</adtf:ddl>)";
const auto ddl_description_filename = TEST_FILES_DIR "test_max_signal_size.description";

TEST_F(StreamItemTopicTest, findStreamTypeQosProfile__successfullSmallStreamType)
{
    const std::string property_content(2, ' ');
    const auto property_name = "test_property";
    EXPECT_CALL(*_stream_type_mock.get(), getPropertyNames())
        .WillOnce(Return(std::vector<std::string>{property_name}));
    EXPECT_CALL(*_stream_type_mock.get(), getProperty(property_name))
        .WillRepeatedly(Return(property_content));
    EXPECT_CALL(*_logger_mock.get(), logDebug(_)).WillRepeatedly(::Return(fep3::Result{}));

    EXPECT_EQ(FEP3_QOS_STREAM_TYPE,
              _stream_item_topic->findStreamTypeQosProfile(*_stream_type_mock));
}

TEST_F(StreamItemTopicTest, findStreamTypeQosProfile__successfullBigStreamType)
{
    const std::string property_content(FEP3_TRANSPORT_LAYER_MAX_MESSAGE_SIZE, ' ');
    const auto property_name = "test_property";
    EXPECT_CALL(*_stream_type_mock.get(), getPropertyNames())
        .WillOnce(Return(std::vector<std::string>{property_name}));
    EXPECT_CALL(*_stream_type_mock.get(), getProperty(property_name))
        .WillRepeatedly(Return(property_content));
    EXPECT_CALL(*_logger_mock.get(), logDebug(_)).WillRepeatedly(::Return(fep3::Result{}));
    EXPECT_CALL(*_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Content of stream type 'anonymous' of topic 'topic_name' exceeds max "
                    "transport limit of '63000'."
                    "Qos profile 'fep3::stream_type_big' will be used.")))
        .WillOnce(::Return(fep3::Result{}));

    EXPECT_EQ(std::string(FEP3_QOS_STREAM_TYPE) + FEP3_BIG_QOS_PROFILE_POSTFIX,
              _stream_item_topic->findStreamTypeQosProfile(*_stream_type_mock));
}

TEST_F(StreamItemTopicTest, findSampleQosProfile__successfullBigDDLType)
{
    EXPECT_CALL(*_stream_type_mock.get(), getMetaTypeName())
        .WillRepeatedly(Return(fep3::base::arya::meta_type_ddl.getName()));
    EXPECT_CALL(*_stream_type_mock.get(),
                getProperty(fep3::base::arya::meta_type_prop_name_ddlstruct))
        .WillRepeatedly(Return(struct_name_big));
    EXPECT_CALL(*_stream_type_mock.get(),
                getProperty(fep3::base::arya::meta_type_prop_name_ddldescription))
        .WillRepeatedly(Return(ddl_description));
    EXPECT_CALL(*_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Size of sample described by stream_type 'ddl' for topic 'topic_name' exceeds "
                    "max transport limit of '63000'."
                    "Qos profile 'fep3::ddl_big' will be used.")))
        .WillOnce(::Return(fep3::Result{}));

    EXPECT_EQ(std::string(FEP3_QOS_PROFILE_PREFIX) + fep3::base::arya::meta_type_ddl.getName() +
                  FEP3_BIG_QOS_PROFILE_POSTFIX,
              _stream_item_topic->findSampleQosProfile(*_stream_type_mock));
}

TEST_F(StreamItemTopicTest, findSampleQosProfile__successfullBigDDLArrayType)
{
    EXPECT_CALL(*_stream_type_mock.get(), getMetaTypeName())
        .WillRepeatedly(Return(fep3::base::arya::meta_type_ddl_array.getName()));
    EXPECT_CALL(*_stream_type_mock.get(),
                getProperty(fep3::base::arya::meta_type_prop_name_ddlstruct))
        .WillRepeatedly(Return(struct_name_small));
    EXPECT_CALL(*_stream_type_mock.get(),
                getProperty(fep3::base::arya::meta_type_prop_name_ddldescription))
        .WillRepeatedly(Return(ddl_description));
    EXPECT_CALL(*_stream_type_mock.get(),
                getProperty(fep3::base::arya::meta_type_prop_name_max_array_size))
        .WillRepeatedly(Return("2"));
    EXPECT_CALL(*_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Size of sample described by stream_type 'ddl-array' for topic 'topic_name' "
                    "exceeds max transport limit of '63000'."
                    "Qos profile 'fep3::ddl-array_big' will be used.")))
        .WillOnce(::Return(fep3::Result{}));

    EXPECT_EQ(std::string(FEP3_QOS_PROFILE_PREFIX) +
                  fep3::base::arya::meta_type_ddl_array.getName() + FEP3_BIG_QOS_PROFILE_POSTFIX,
              _stream_item_topic->findSampleQosProfile(*_stream_type_mock));
}

TEST_F(StreamItemTopicTest, findSampleQosProfile__successfullBigDDLFilerefType)
{
    EXPECT_CALL(*_stream_type_mock.get(), getMetaTypeName())
        .WillRepeatedly(Return(fep3::base::arya::meta_type_ddl_fileref.getName()));
    EXPECT_CALL(*_stream_type_mock.get(),
                getProperty(fep3::base::arya::meta_type_prop_name_ddlstruct))
        .WillRepeatedly(Return(struct_name_big));
    EXPECT_CALL(*_stream_type_mock.get(),
                getProperty(fep3::base::arya::meta_type_prop_name_ddlfileref))
        .WillRepeatedly(Return(ddl_description_filename));
    EXPECT_CALL(*_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Size of sample described by stream_type 'ddl-fileref' for topic 'topic_name' "
                    "exceeds max transport limit of '63000'."
                    "Qos profile 'fep3::ddl-fileref_big' will be used.")))
        .WillOnce(::Return(fep3::Result{}));

    EXPECT_EQ(std::string(FEP3_QOS_PROFILE_PREFIX) +
                  fep3::base::arya::meta_type_ddl_fileref.getName() + FEP3_BIG_QOS_PROFILE_POSTFIX,
              _stream_item_topic->findSampleQosProfile(*_stream_type_mock));
}

TEST_F(StreamItemTopicTest, findSampleQosProfile__successfullBigDDLArrayFilerefType)
{
    EXPECT_CALL(*_stream_type_mock.get(), getMetaTypeName())
        .WillRepeatedly(Return(fep3::base::arya::meta_type_ddl_array_fileref.getName()));
    EXPECT_CALL(*_stream_type_mock.get(),
                getProperty(fep3::base::arya::meta_type_prop_name_ddlstruct))
        .WillRepeatedly(Return(struct_name_small));
    EXPECT_CALL(*_stream_type_mock.get(),
                getProperty(fep3::base::arya::meta_type_prop_name_ddlfileref))
        .WillRepeatedly(Return(ddl_description_filename));
    EXPECT_CALL(*_stream_type_mock.get(),
                getProperty(fep3::base::arya::meta_type_prop_name_max_array_size))
        .WillRepeatedly(Return("2"));
    EXPECT_CALL(*_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Size of sample described by stream_type 'ddl-fileref-array' for topic "
                    "'topic_name' exceeds max transport limit of '63000'."
                    "Qos profile 'fep3::ddl-fileref-array_big' will be used.")))
        .WillOnce(::Return(fep3::Result{}));

    EXPECT_EQ(std::string(FEP3_QOS_PROFILE_PREFIX) +
                  fep3::base::arya::meta_type_ddl_array_fileref.getName() +
                  FEP3_BIG_QOS_PROFILE_POSTFIX,
              _stream_item_topic->findSampleQosProfile(*_stream_type_mock));
}

TEST_F(StreamItemTopicTest, updateStreamType__successfull)
{
    const auto stream_type_mock = std::make_shared<StreamTypeMock>();
    EXPECT_CALL(*stream_type_mock.get(), getMetaTypeName())
        .WillRepeatedly(Return(fep3::base::arya::meta_type_audio.getName()));
    EXPECT_CALL(*stream_type_mock.get(), copyTo(_)).WillRepeatedly(Return());
    EXPECT_CALL(*stream_type_mock.get(), getProperty(_)).WillRepeatedly(Return(""));
    EXPECT_CALL(
        *_logger_mock.get(),
        logDebug(fep3::mock::LogStringRegexMatcher(
            "Update qos profile for topic 'topic_name' from 'fep3::anonymous' to 'fep3::audio'.")))
        .WillOnce(::Return(fep3::Result{}));
    EXPECT_TRUE(_stream_item_topic->updateStreamType(*stream_type_mock));
    EXPECT_EQ(std::string(FEP3_QOS_PROFILE_PREFIX) + fep3::base::arya::meta_type_audio.getName(),
              _stream_item_topic->getSampleQosProfile());
}
