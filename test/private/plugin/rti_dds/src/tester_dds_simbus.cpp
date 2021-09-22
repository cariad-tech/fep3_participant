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


#include "detail/test_read_write_test_class.hpp"
#include "detail/test_receiver.hpp"
#include "detail/test_samples.hpp"

#include <fep3/base/sample/data_sample.h>
#include <fep3/base/sample/mock/mock_data_sample.h>
#include <fep3/base/sample/raw_memory.h>
#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/base/stream_type/mock/mock_stream_type.h>
#include <fep3/components/simulation_bus/mock/mock_simulation_bus.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <helper/gmock_async_helper.h>

/**
 * @detail Test send and receive of one sample
 * @req_id FEPSDK-Sample
 */
TEST_F(ReaderWriterTestClass, SendAndReceiveSample)
{
    test::helper::Notification all_items_received;
    const auto& mock_receiver = std::make_shared<::testing::StrictMock<fep3::mock::DataReceiver>>();

    uint32_t test_sample_value = 6;
    const data_read_ptr<const IDataSample> test_sample = std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value);

    { // setting of expectations
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            // DataSampleType currently doesn't handle timestamp and counter correctly (see FEPSDK-2668)
            // thus we only check the value -> DataSampleSmartPtr*Value*Matcher
            // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
            (fep3::mock::DataSampleSmartPtrValueMatcher(test_sample)))).WillOnce(Notify(&all_items_received));
        // ignore stream types
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IStreamType>&>
            (::testing::_))).WillRepeatedly(::testing::Return());
    }

    _reader->reset(mock_receiver);

    startReception(getSimulationBus());
    _writer->write(*test_sample.get());
    _writer->transmit();
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(5)));
    stopReception(getSimulationBus());
}


/**
 * @detail Test send and receive of one stream type change
 * @req_id FEPSDK-Sample
 */
TEST_F(ReaderWriterTestClass, SendAndReceiveStreamType)
{
    test::helper::Notification all_items_received;
    const auto& mock_receiver = std::make_shared<::testing::StrictMock<fep3::mock::DataReceiver>>();

    const data_read_ptr<const IStreamType> test_stream_type = std::make_shared<base::StreamTypeDDL>("tStruct", "ddl_description");

    { // setting of expectations
        // ignore the initial stream type
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IStreamType>&>
            (::testing::_))).WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IStreamType>&>
            (fep3::mock::StreamTypeSmartPtrMatcher(test_stream_type))))
            .WillOnce(Notify(&all_items_received)) // stream type as sent by transmitter
            ;
    }

    _reader->reset(mock_receiver);

    startReception(getSimulationBus());
    _writer->write(*test_stream_type.get());
    _writer->transmit();
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(5)));
    stopReception(getSimulationBus());
}


/**
 * @detail Test change of stream type during sample transmition
 * @req_id FEPSDK-Sample
 */
TEST_F(ReaderWriterTestClass, ChangeStreamType)
{
    test::helper::Notification all_items_received;
    const auto& mock_receiver = std::make_shared<::testing::StrictMock<fep3::mock::DataReceiver>>();

    uint8_t test_sample_value_1 = 6;
    const data_read_ptr<const IDataSample> test_sample_1 = std::make_shared<fep3::base::DataSampleType<uint8_t>>(test_sample_value_1);

    const data_read_ptr<const IStreamType> test_stream_type = std::make_shared<base::StreamTypePlain<uint64_t>>();

    uint64_t test_sample_value_2 = 600000000;
    const data_read_ptr<const IDataSample> test_sample_2 = std::make_shared<fep3::base::DataSampleType<uint64_t>>(test_sample_value_2);

    { // setting of expectations
        ::testing::InSequence sequence;
        // ignore the initial stream type
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IStreamType>&>
            (::testing::_))).WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            // DataSampleType currently doesn't handle timestamp and counter correctly (see FEPSDK-2668)
            // thus we only check the value -> DataSampleSmartPtr*Value*Matcher
            // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
            (fep3::mock::DataSampleSmartPtrValueMatcher(test_sample_1)))).WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IStreamType>&>
            (fep3::mock::StreamTypeSmartPtrMatcher(test_stream_type))))
            .WillOnce(::testing::Return()) // stream type as sent by transmitter
            ;
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            // DataSampleType currently doesn't handle timestamp and counter correctly (see FEPSDK-2668)
            // thus we only check the value -> DataSampleSmartPtr*Value*Matcher
            // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
            (fep3::mock::DataSampleSmartPtrValueMatcher(test_sample_2)))).WillOnce(Notify(&all_items_received));
    }

    _reader->reset(mock_receiver);

    startReception(getSimulationBus());
    _writer->write(*test_sample_1.get());
    _writer->write(*test_stream_type.get());
    _writer->write(*test_sample_2.get());
    _writer->transmit();
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(5)));
    stopReception(getSimulationBus());
}


/**
 * @detail Test send and receive from sample with timestamp
 * @req_id FEPSDK-Sample
 */
TEST_F(ReaderWriterTestClass, SampleTimestamp)
{
    test::helper::Notification all_items_received;
    const auto& mock_receiver = std::make_shared<::testing::StrictMock<fep3::mock::DataReceiver>>();

    uint32_t test_sample_value = 6;
    const data_read_ptr<const IDataSample> test_sample = std::make_shared<TimeDataSampleType<uint32_t>>
        (test_sample_value, Timestamp(3));

    { // setting of expectations
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            (fep3::mock::DataSampleSmartPtrTimestampAndValueMatcher(test_sample)))).WillOnce(Notify(&all_items_received));
        // ignore stream types
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IStreamType>&>
            (::testing::_))).WillRepeatedly(::testing::Return());
    }

    _reader->reset(mock_receiver);

    startReception(getSimulationBus());
    _writer->write(*test_sample.get());
    _writer->transmit();
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(5)));
    stopReception(getSimulationBus());
}

/**
 * @detail Test send and receive from sample with timestamp
 * @req_id FEPSDK-Sample
 */
TEST_F(ReaderWriterTestClass, getFrontTime)
{
    startReception(getSimulationBus());

    uint32_t value = 6;
    _writer->write(TimeDataSampleType<uint32_t>(value, Timestamp(3)));
    _writer->transmit();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Pop stream_type
    CountSampleTestReceiver receiver;
    _reader->pop(receiver);

    // Now pop the real sample
    EXPECT_EQ(_reader->getFrontTime().value_or(Timestamp(0)), Timestamp(3));

    TestReceiver sample_receiver;
    _reader->pop(sample_receiver);

    ASSERT_EQ(sample_receiver._samples.size(), 1);
    EXPECT_EQ(sample_receiver._samples.at(0)->getTime(), Timestamp(3));
}

MATCHER_P(DataSampleSmartPtrCounterMatcher, pointer_to_expected_counter, "Matcher for counter of smart pointer to IDataSample")
{
    return arg->getCounter() == *pointer_to_expected_counter;
}
ACTION_P(AssignDataSampleSmartPtrCounter, pointer_to_destination)
{
    *pointer_to_destination = arg0->getCounter();
}
ACTION_P(Increment, pointer_to_value)
{
    (*pointer_to_value)++;
}

/**
 * @detail Test the sample counter
 * @req_id FEPSDK-Sample
 */
TEST_F(ReaderWriterTestClass, SampleCounter)
{
    test::helper::Notification all_items_received;
    const auto& mock_receiver = std::make_shared<::testing::StrictMock<fep3::mock::DataReceiver>>();

    uint32_t test_sample_value_1 = 6;
    const data_read_ptr<const IDataSample> test_sample_1 = std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value_1);
    uint32_t test_sample_value_2 = 7;
    const data_read_ptr<const IDataSample> test_sample_2 = std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value_2);
    uint32_t test_sample_value_3 = 8;
    const data_read_ptr<const IDataSample> test_sample_3 = std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value_3);

    uint32_t reference_counter = 0;

    { // setting of expectations
        ::testing::InSequence sequence;
        // ignore initial stream type
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IStreamType>&>
            (::testing::_))).WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            (fep3::mock::DataSampleSmartPtrValueMatcher(test_sample_1))))
            .WillOnce(::testing::DoAll(AssignDataSampleSmartPtrCounter(&reference_counter), Increment(&reference_counter))) // first sample sets the reference_counter
            ;
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            (DataSampleSmartPtrCounterMatcher(&reference_counter))))
            .WillOnce(Increment(&reference_counter))
            .WillOnce(Notify(&all_items_received))
            ;
    }

    _reader->reset(mock_receiver);

    startReception(getSimulationBus());
    _writer->write(*test_sample_1.get());
    _writer->write(*test_sample_2.get());
    _writer->write(*test_sample_3.get());
    _writer->transmit();
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(5)));
    stopReception(getSimulationBus());
}


/**
 * @detail Test a large sample (video)
 * @req_id FEPSDK-Sample
 */
TEST_F(ReaderWriterTestClass, VideoSample)
{
    auto video_type = base::StreamType(fep3::base::arya::meta_type_video);
    video_type.setProperty("height", "3840", "uint32_t");
    video_type.setProperty("width", "2160", "uint32_t");
    video_type.setProperty("pixelformat", "R(8)G(9)B(8)", "string");
    video_type.setProperty(fep3::base::arya::meta_type_prop_name_max_byte_size, "24883200", "uint32_t");

    auto writer = getSimulationBus()->getWriter("video", video_type);
    auto reader = getSimulationBus()->getReader("video", video_type);

    EXPECT_TRUE(writer);
    EXPECT_TRUE(reader);

    //std::this_thread::sleep_for(std::chrono::seconds(1));
    test::helper::Notification all_items_received;
    const auto& mock_receiver = std::make_shared<::testing::StrictMock<fep3::mock::DataReceiver>>();

    const data_read_ptr<const IDataSample> image_sample1 = std::make_shared<RandomSample>(24883200);
    const data_read_ptr<const IDataSample> image_sample2 = std::make_shared<RandomSample>(24883200);
    const data_read_ptr<const IDataSample> image_sample3 = std::make_shared<RandomSample>(24883200);

    { // setting of expectations
        ::testing::InSequence sequence;
        // ignore initial stream type
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IStreamType>&>
            (::testing::_))).WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            (fep3::mock::DataSampleSmartPtrValueMatcher(image_sample1))))
            .WillOnce(::testing::Return())
            ;
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            (fep3::mock::DataSampleSmartPtrValueMatcher(image_sample2))))
            .WillOnce(::testing::Return())
            ;
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            (fep3::mock::DataSampleSmartPtrValueMatcher(image_sample3))))
            .WillOnce(Notify(&all_items_received))
            ;
    }

    reader->reset(mock_receiver);

    startReception(getSimulationBus());
    writer->write(*image_sample1.get());
    writer->write(*image_sample2.get());
    writer->write(*image_sample3.get());
    writer->transmit();
    // For ARMv8 with limited resource, 1 Core and 1 GB RAM, the running time can exceed 13s.
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(15)));
    stopReception(getSimulationBus());
}

TEST_F(ReaderWriterTestClass, TestCorrectOrderOfSampleAndStreamType)
{
    OrderTestReceiver sample_receiver;

    std::thread sender_thread([&]()
    {
        for (uint8_t i = 0; i < 100; i++)
        {
            _writer->write(base::DataSampleType<uint8_t>(i));
            _writer->write(base::StreamTypePlain<uint32_t>());
            _writer->transmit();
        }
    });

    for (int i = 0; i < 100; i++)
    {
        _reader->pop(sample_receiver);
        _reader->pop(sample_receiver);
    }

    sender_thread.join();
}

TEST_F(ReaderWriterTestClass, TestPopOfDataReader)
{
    startReception(getSimulationBus());

    // Wait for Initalisation and discover
    CountSampleTestReceiver sample_receiver;

    _reader->pop(sample_receiver);

    for (int i = 0; i < 10; i++)
    {
        uint32_t value = 6;
        _writer->write(base::DataSampleType<uint32_t>(value));
        _writer->transmit();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        EXPECT_EQ(_reader->size(), 1);
        _reader->pop(sample_receiver);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_EQ(sample_receiver.getSampleCount(), 1u);
    }
}

/**
 * @detail Test mapping of meta stream type to the associated qos profile in QOS_USER_PROFILE.xml.
 *         All fep defined meta stream type should be covered by a qos profile.
 *         Self-defined types are tried to map if nothing is found, the 'fep3::default_profile' type is used.
 *
 * @req_id FEPSDK-
 */
TEST_F(ReaderWriterTestClass, TestMetaTypeToQOSProfile)
{

    auto logger_mock = std::make_shared<::testing::StrictMock<fep3::mock::LoggerMock>>();
    EXPECT_CALL((*logger_mock.get()), isDebugEnabled()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL((*logger_mock.get()), isWarningEnabled()).WillRepeatedly(::testing::Return(true));

    std::shared_ptr<Components> components = std::make_shared<Components>("test_meta_type_tp_qos_profile", "default_test_system", logger_mock);

    auto simulation_bus = createSimulationBus(getDomainId(), components);

    EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::ddl' for topic 'topic_ddl'."))).WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(dynamic_cast<ISimulationBus*>(simulation_bus.get())->getWriter("topic_ddl", fep3::base::StreamTypeDDL("", "")));

    EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::plain-ctype' for topic 'topic_plain_c'."))).WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(dynamic_cast<ISimulationBus*>(simulation_bus.get())->getWriter("topic_plain_c", fep3::base::StreamTypePlain<int16_t>()));

    EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::anonymous' for topic 'topic_raw'."))).WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(dynamic_cast<ISimulationBus*>(simulation_bus.get())->getWriter("topic_raw", fep3::base::StreamTypeRaw()));

    EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::video' for topic 'topic_video'."))).WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(dynamic_cast<ISimulationBus*>(simulation_bus.get())->getWriter("topic_video", fep3::base::StreamType(fep3::base::arya::meta_type_video)));

    EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::audio' for topic 'topic_audio'."))).WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(dynamic_cast<ISimulationBus*>(simulation_bus.get())->getWriter("topic_audio", fep3::base::StreamType(fep3::base::arya::meta_type_audio)));

    EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::ascii-string' for topic 'topic_ascii-string'."))).WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(dynamic_cast<ISimulationBus*>(simulation_bus.get())->getWriter("topic_ascii-string", fep3::base::StreamTypeString()));

    EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::ddl-array' for topic 'topic_ddl-array'."))).WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(dynamic_cast<ISimulationBus*>(simulation_bus.get())->getWriter("topic_ddl-array", fep3::base::StreamTypeDDLArray("", "", 10)));

    EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::ddl-fileref-array' for topic 'topic_ddl-fileref-array'."))).WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(dynamic_cast<ISimulationBus*>(simulation_bus.get())->getWriter("topic_ddl-fileref-array", fep3::base::StreamTypeDDLArrayFileRef("", "", 10)));

    EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::ddl-fileref' for topic 'topic_ddl-fileref'."))).WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(dynamic_cast<ISimulationBus*>(simulation_bus.get())->getWriter("topic_ddl-fileref", fep3::base::StreamTypeDDLFileRef("", "")));

    EXPECT_CALL((*logger_mock.get()), logWarning(fep3::mock::LogStringRegexMatcher("MetaType 'my_fancy_stream_meta_type' not defined in USER_QOS_PROFILES.xml. Using 'fep3::default_profile'."))).WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::default_profile' for topic 'topic_my_fancy'."))).WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(dynamic_cast<ISimulationBus*>(simulation_bus.get())->getWriter("topic_my_fancy", fep3::base::StreamType(base::StreamMetaType("my_fancy_stream_meta_type"))));

    EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::user_stream_meta_type' for topic 'topic_user_stream_meta_type'."))).WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(dynamic_cast<ISimulationBus*>(simulation_bus.get())->getWriter("topic_user_stream_meta_type", fep3::base::StreamType({ "user_stream_meta_type", std::list<std::string>{} })));

}

/*
* @detail For plain-array-ctype we expecting a small data qos profile,
* but we need to make sure that if max_array_size is bigger that we switch to a big data qos.
* This test will fail without StreamItemTopic::isBigStreamType()
* @req_id FEPSDK-2675
*/
TEST_F(ReaderWriterTestClass, TestOverSizedPlainArray)
{

    auto logger_mock = std::make_shared<::testing::StrictMock<fep3::mock::LoggerMock>>();
    EXPECT_CALL((*logger_mock.get()), isDebugEnabled()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL((*logger_mock.get()), isWarningEnabled()).WillRepeatedly(::testing::Return(true));

    auto simulation_bus = createSimulationBus(getDomainId(), std::make_shared<Components>("participant_1", "default_test_system", logger_mock));
    auto simulation_bus2 = createSimulationBus(getDomainId(), std::make_shared<Components>("participant_2", "default_test_system", logger_mock));
    {
        auto plain_array_type = base::StreamType(fep3::base::arya::meta_type_plain_array);
        plain_array_type.setProperty("max_array_size", "1000000", "uint32_t");
        plain_array_type.setProperty("datatype", "uint32_t", "string");

        EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::plain-array-ctype_big' for topic 'plain_array'."))).WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("stream_type 'plain-array-ctype' for topic 'plain_array' is too big and needs to be fragmented. Using profile 'plain-array-ctype_big'."))).WillOnce(::testing::Return(fep3::Result{}));
        auto writer = dynamic_cast<ISimulationBus*>(simulation_bus.get())->getWriter("plain_array", plain_array_type);

        EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("stream_type 'plain-array-ctype' for topic 'plain_array' is too big and needs to be fragmented. Using profile 'plain-array-ctype_big'."))).WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::plain-array-ctype_big' for topic 'plain_array'."))).WillOnce(::testing::Return(fep3::Result{}));
        auto reader = dynamic_cast<ISimulationBus*>(simulation_bus2.get())->getReader("plain_array", plain_array_type);

        EXPECT_TRUE(writer);
        EXPECT_TRUE(reader);

        startReception(dynamic_cast<ISimulationBus*>(simulation_bus2.get()));

        CountSampleTestReceiver sample_receiver;

        // filter first streamtype
        reader->pop(sample_receiver);

        {
            std::vector<uint32_t> values(1000000);
            std::fill(values.begin(), values.end(), 7);
            writer->write(base::DataSampleType<std::vector<uint32_t>>(values));
            writer->transmit();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            EXPECT_EQ(reader->size(), 1);
            reader->pop(sample_receiver);
            EXPECT_EQ(sample_receiver.getSampleCount(), 1u);
        }

        EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::anonymous' for topic 'plain_array_dynamic'."))).WillRepeatedly(::testing::Return(fep3::Result{}));

        // Creating dynamic reader and writer
        writer = dynamic_cast<ISimulationBus*>(simulation_bus.get())->getWriter("plain_array_dynamic");
        reader = dynamic_cast<ISimulationBus*>(simulation_bus2.get())->getReader("plain_array_dynamic");

        EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Update qos profile for topic 'plain_array_dynamic' from 'fep3::anonymous' to 'fep3::plain-array-ctype_big'."))).WillRepeatedly(::testing::Return(fep3::Result{}));
        EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("stream_type 'plain-array-ctype' for topic 'plain_array_dynamic' is too big and needs to be fragmented. Using profile 'plain-array-ctype_big'."))).WillRepeatedly(::testing::Return(fep3::Result{}));

        writer->write(plain_array_type);
        writer->transmit();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // filter first streamtype and dynamic streamtype
        reader->pop(sample_receiver);
        reader->pop(sample_receiver);

        std::vector<uint32_t> values(1000000);
        std::fill(values.begin(), values.end(), 7);

        {
            writer->write(base::DataSampleType<std::vector<uint32_t>>(values));
            writer->transmit();

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            EXPECT_EQ(reader->size(), 1);
            reader->pop(sample_receiver);
            EXPECT_EQ(sample_receiver.getSampleCount(), 1u);
        }

        // Check that 63.000 bytes are still small
        plain_array_type = base::StreamType(fep3::base::arya::meta_type_plain_array);
        plain_array_type.setProperty("max_array_size", "7875", "uint64_t");
        plain_array_type.setProperty("datatype", "uint32_t", "string");

        EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::plain-array-ctype' for topic 'plain_array_63000'."))).WillRepeatedly(::testing::Return(fep3::Result{}));
        writer = dynamic_cast<ISimulationBus*>(simulation_bus.get())->getWriter("plain_array_63000", plain_array_type);
        reader = dynamic_cast<ISimulationBus*>(simulation_bus2.get())->getReader("plain_array_63000", plain_array_type);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // filter first streamtype
        reader->pop(sample_receiver);

        std::vector<uint64_t> values64(7875);
        std::fill(values.begin(), values.end(), 7);

        {
            writer->write(base::DataSampleType<std::vector<uint64_t>>(values64));
            writer->transmit();

            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            EXPECT_EQ(reader->size(), 1);
            reader->pop(sample_receiver);
            EXPECT_EQ(sample_receiver.getSampleCount(), 1u);
        }

        stopReception(dynamic_cast<ISimulationBus*>(simulation_bus2.get()));
    }
}
/*
* @detail Test logger is correct distributed in RTI DDS Simulation Bus
*
* @req_id FEPSDK-2839
*/
TEST_F(ReaderWriterTestClass, TestLogging)
{
    auto logger_mock = std::make_shared<::testing::StrictMock<fep3::mock::LoggerMock>>();
    EXPECT_CALL((*logger_mock.get()), isDebugEnabled()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL((*logger_mock.get()), isWarningEnabled()).WillRepeatedly(::testing::Return(false));

    std::shared_ptr<Components> components = std::make_shared<Components>("test_meta_type_tp_qos_profile", "default_test_system", logger_mock);
    EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::anonymous' for topic 'my_topic'."))).WillOnce(::testing::Return(fep3::Result{}));

    auto simulation_bus = createSimulationBus(getDomainId(), components);
    auto reader = dynamic_cast<ISimulationBus*>(simulation_bus.get())->getReader("my_topic");

    reader->reset(std::make_shared< CountSampleTestReceiver>());
    EXPECT_CALL((*logger_mock.get()), logDebug(fep3::mock::LogStringRegexMatcher("Replaced already registered data receiver for reader from topic 'my_topic'."))).WillOnce(::testing::Return(fep3::Result{}));
    reader->reset(nullptr);
}

/*
* @detail Test qos change from big streamtype "anonymus" qos to small streamtype qos "c-plain"
* This include to recreate rti reader and writer to change qos settings
* 
* @req_id FEPSDK-2892
*/
TEST_F(ReaderWriterTestClass, TestChangeFromBigToSmallToBigQOS)
{
  
    auto writer = getSimulationBus()->getWriter("uint32", fep3::base::StreamTypePlain<uint32_t>());
    auto reader = getSimulationBus2()->getReader("uint32");

    EXPECT_TRUE(writer);
    EXPECT_TRUE(reader);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    test::helper::Notification all_items_received;
    const auto& mock_receiver = std::make_shared<::testing::StrictMock<fep3::mock::DataReceiver>>();

    uint32_t test_sample_value_1 = 6;
    const data_read_ptr<const IDataSample> test_sample_1 = std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value_1);
    uint32_t test_sample_value_2 = 7;
    const data_read_ptr<const IDataSample> test_sample_2 = std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value_2);
    uint32_t test_sample_value_3 = 8;
    const data_read_ptr<const IDataSample> test_sample_3 = std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value_3);

    { // setting of expectations
        ::testing::InSequence sequence;
        // ignore initial stream type
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IStreamType>&>
            (::testing::_))).WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            (fep3::mock::DataSampleSmartPtrValueMatcher(test_sample_1))))
            .WillOnce(::testing::Return())
            ;
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            (fep3::mock::DataSampleSmartPtrValueMatcher(test_sample_2))))
            .WillOnce(::testing::Return())
            ;
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            (fep3::mock::DataSampleSmartPtrValueMatcher(test_sample_3))))
            .WillOnce(Notify(&all_items_received))
            ;
    }

    reader->reset(mock_receiver);

    startReception(getSimulationBus2());
    writer->write(*test_sample_1.get());
    writer->write(*test_sample_2.get());
    writer->write(*test_sample_3.get());
    writer->transmit();
    // For ARMv8 with limited resource, 1 Core and 1 GB RAM, the running time can exceed 13s.
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(15)));

    // Now check that the write also can change his qos settings
    // By default we can not send big data over a signal with small qos settings. RTI would tell us:
    // "Reliable fragmented data requires asynchronous writer"
    
    const data_read_ptr<const IDataSample> image_sample1 = std::make_shared<RandomSample>(24883200);
    const data_read_ptr<const IDataSample> image_sample2 = std::make_shared<RandomSample>(24883200);
    const data_read_ptr<const IDataSample> image_sample3 = std::make_shared<RandomSample>(24883200);

    { // setting of expectations
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IStreamType>&>
            (::testing::_))).WillRepeatedly(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            (fep3::mock::DataSampleSmartPtrValueMatcher(image_sample1))))
            .WillOnce(::testing::Return())
            ;
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            (fep3::mock::DataSampleSmartPtrValueMatcher(image_sample2))))
            .WillOnce(::testing::Return())
            ;
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            (fep3::mock::DataSampleSmartPtrValueMatcher(image_sample3))))
            .WillOnce(Notify(&all_items_received))
            ;
    }

    auto video_type = base::StreamType(fep3::base::arya::meta_type_video);
    video_type.setProperty("height", "3840", "uint32_t");
    video_type.setProperty("width", "2160", "uint32_t");
    video_type.setProperty("pixelformat", "R(8)G(9)B(8)", "string");
    video_type.setProperty("max_size", "24883200", "uint32_t");

    writer->write(video_type);
   
    writer->write(*image_sample1.get());
    writer->write(*image_sample2.get());
    writer->write(*image_sample3.get());
    writer->transmit();
    // For ARMv8 with limited resource, 1 Core and 1 GB RAM, the running time can exceed 13s.
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(15)));

    stopReception(getSimulationBus2());
}

/*
* @detail Test that a reader can be requested during a running reception thread
* This is necessary after changing qos settings
*
* @req_id FEPSDK-2892
*/
TEST_F(ReaderWriterTestClass, TestReaderAfterStartReception)
{
    auto writer = getSimulationBus()->getWriter("uint32", fep3::base::StreamTypePlain<uint32_t>());
    EXPECT_TRUE(writer);

    test::helper::Notification all_items_received;
    const auto& mock_receiver = std::make_shared<::testing::StrictMock<fep3::mock::DataReceiver>>();

    uint32_t test_sample_value_1 = 6;
    const data_read_ptr<const IDataSample> test_sample_1 = std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value_1);

    { // setting of expectations
        ::testing::InSequence sequence;
        // ignore initial stream type
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IStreamType>&>
            (::testing::_))).WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            (fep3::mock::DataSampleSmartPtrValueMatcher(test_sample_1))))
            .WillOnce(Notify(&all_items_received))
            ;
    }

    startReception(getSimulationBus2());

    auto reader = getSimulationBus2()->getReader("uint32");
    reader->reset(mock_receiver);
    EXPECT_TRUE(reader);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    writer->write(*test_sample_1.get());
    writer->transmit();
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(10)));
}
