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

#include "detail/test_read_write_test_class.hpp"
#include "detail/test_receiver.hpp"
#include "detail/test_samples.hpp"

#include <fep3/base/sample/mock/mock_data_sample.h>
#include <fep3/base/stream_type/mock/mock_stream_type.h>
#include <fep3/components/simulation_bus/mock_simulation_bus.h>

#include <helper/gmock_async_helper.h>

using namespace std::chrono_literals;

TYPED_TEST_SUITE(ReaderWriterTestSimulationBus, SimulationBusTypes);

/**
 * @detail Test send and receive of one sample
 * @req_id FEPSDK-Sample
 */
TYPED_TEST(ReaderWriterTestSimulationBus, SendAndReceiveSample)
{
    test::helper::Notification all_items_received;
    const auto& mock_receiver =
        std::make_shared<::testing::StrictMock<fep3::mock::SimulationBus::DataReceiver>>();

    uint32_t test_sample_value = 6;
    const data_read_ptr<const IDataSample> test_sample =
        std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value);

    { // setting of expectations
        EXPECT_CALL(
            *mock_receiver.get(),
            call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
                 // DataSampleType currently doesn't handle timestamp and counter correctly (see
                 // FEPSDK-2668) thus we only check the value -> DataSampleSmartPtr*Value*Matcher
                 // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
                 (fep3::mock::DataSampleSmartPtrValueMatcher(test_sample))))
            .WillOnce(Notify(&all_items_received));
        // ignore stream types
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(::testing::_)))
            .WillRepeatedly(::testing::Return());
    }

    this->_reader->reset(mock_receiver);

    this->startReception(this->getSimulationBus());
    this->_writer->write(*test_sample.get());
    this->_writer->transmit();
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(5)));
    this->stopReception(this->getSimulationBus());
}

/**
 * @detail Test send and receive of one stream type change
 * @req_id FEPSDK-Sample
 */
TYPED_TEST(ReaderWriterTestSimulationBus, SendAndReceiveStreamType)
{
    test::helper::Notification all_items_received;
    const auto& mock_receiver =
        std::make_shared<::testing::StrictMock<fep3::mock::SimulationBus::DataReceiver>>();

    const auto test_stream_type =
        std::make_shared<base::StreamTypeDDL>("tStruct", "ddl_description");

    { // setting of expectations
        // ignore the initial stream type
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(::testing::_)))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(
                        fep3::mock::StreamTypeSmartPtrMatcher(test_stream_type))))
            .WillOnce(Notify(&all_items_received)) // stream type as sent by transmitter
            ;
    }

    this->_reader->reset(mock_receiver);

    this->startReception(this->getSimulationBus());
    this->_writer->write(*test_stream_type.get());
    this->_writer->transmit();
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(5)));
    this->stopReception(this->getSimulationBus());
}

/**
 * @detail Test change of stream type during sample transmition
 * @req_id FEPSDK-Sample
 */
TYPED_TEST(ReaderWriterTestSimulationBus, DISABLED_ChangeStreamType)
{
    test::helper::Notification all_items_received;
    const auto& mock_receiver =
        std::make_shared<::testing::StrictMock<fep3::mock::SimulationBus::DataReceiver>>();

    uint8_t test_sample_value_1 = 6;
    const data_read_ptr<const IDataSample> test_sample_1 =
        std::make_shared<fep3::base::DataSampleType<uint8_t>>(test_sample_value_1);

    const data_read_ptr<const IStreamType> test_stream_type =
        std::make_shared<base::StreamTypePlain<uint64_t>>();

    uint64_t test_sample_value_2 = 600000000;
    const data_read_ptr<const IDataSample> test_sample_2 =
        std::make_shared<fep3::base::DataSampleType<uint64_t>>(test_sample_value_2);

    { // setting of expectations
        ::testing::InSequence sequence;
        // ignore the initial stream type
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(::testing::_)))
            .WillOnce(::testing::Return());
        EXPECT_CALL(
            *mock_receiver.get(),
            call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
                 // DataSampleType currently doesn't handle timestamp and counter correctly (see
                 // FEPSDK-2668) thus we only check the value -> DataSampleSmartPtr*Value*Matcher
                 // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
                 (fep3::mock::DataSampleSmartPtrValueMatcher(test_sample_1))))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(
                        fep3::mock::StreamTypeSmartPtrMatcher(test_stream_type))))
            .WillOnce(::testing::Return()) // stream type as sent by transmitter
            ;
        EXPECT_CALL(
            *mock_receiver.get(),
            call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
                 // DataSampleType currently doesn't handle timestamp and counter correctly (see
                 // FEPSDK-2668) thus we only check the value -> DataSampleSmartPtr*Value*Matcher
                 // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
                 (fep3::mock::DataSampleSmartPtrValueMatcher(test_sample_2))))
            .WillOnce(Notify(&all_items_received));
    }

    this->_reader->reset(mock_receiver);

    this->startReception(this->getSimulationBus());
    this->_writer->write(*test_sample_1.get());
    this->_writer->write(*test_stream_type.get());
    this->_writer->write(*test_sample_2.get());
    this->_writer->transmit();
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(5)));
    this->stopReception(this->getSimulationBus());
}

/**
 * @detail Test send and receive from sample with timestamp
 * @req_id FEPSDK-Sample
 */
TYPED_TEST(ReaderWriterTestSimulationBus, SampleTimestamp)
{
    test::helper::Notification all_items_received;
    const auto& mock_receiver =
        std::make_shared<::testing::StrictMock<fep3::mock::SimulationBus::DataReceiver>>();

    uint32_t test_sample_value = 6;
    const data_read_ptr<const IDataSample> test_sample =
        std::make_shared<TimeDataSampleType<uint32_t>>(test_sample_value, Timestamp(3));

    { // setting of expectations
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                        fep3::mock::DataSampleSmartPtrTimestampAndValueMatcher(test_sample))))
            .WillOnce(Notify(&all_items_received));
        // ignore stream types
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(::testing::_)))
            .WillRepeatedly(::testing::Return());
    }

    this->_reader->reset(mock_receiver);

    this->startReception(this->getSimulationBus());
    this->_writer->write(*test_sample.get());
    this->_writer->transmit();
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(5)));
    this->stopReception(this->getSimulationBus());
}

/**
 * @detail Test send and receive from sample with timestamp
 * @req_id FEPSDK-Sample
 */
TYPED_TEST(ReaderWriterTestSimulationBus, getFrontTime)
{
    this->startReception(this->getSimulationBus());

    uint32_t value = 6;
    this->_writer->write(TimeDataSampleType<uint32_t>(value, Timestamp(3)));
    this->_writer->transmit();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Pop stream_type
    CountSampleTestReceiver receiver;
    this->_reader->pop(receiver);

    // Now pop the real sample
    EXPECT_EQ(this->_reader->getFrontTime().value_or(Timestamp(0)), Timestamp(3));

    TestReceiver sample_receiver;
    this->_reader->pop(sample_receiver);

    ASSERT_EQ(sample_receiver._samples.size(), 1);
    EXPECT_EQ(sample_receiver._samples.at(0)->getTime(), Timestamp(3));
}

MATCHER_P(DataSampleSmartPtrCounterMatcher,
          pointer_to_expected_counter,
          "Matcher for counter of smart pointer to IDataSample")
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
TYPED_TEST(ReaderWriterTestSimulationBus, SampleCounter)
{
    test::helper::Notification all_items_received;
    const auto& mock_receiver =
        std::make_shared<::testing::StrictMock<fep3::mock::SimulationBus::DataReceiver>>();

    uint32_t test_sample_value_1 = 6;
    const data_read_ptr<const IDataSample> test_sample_1 =
        std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value_1);
    uint32_t test_sample_value_2 = 7;
    const data_read_ptr<const IDataSample> test_sample_2 =
        std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value_2);
    uint32_t test_sample_value_3 = 8;
    const data_read_ptr<const IDataSample> test_sample_3 =
        std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value_3);

    uint32_t reference_counter = 0;

    { // setting of expectations
        ::testing::InSequence sequence;
        // ignore initial stream type
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(::testing::_)))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                        fep3::mock::DataSampleSmartPtrValueMatcher(test_sample_1))))
            .WillOnce(::testing::DoAll(
                AssignDataSampleSmartPtrCounter(&reference_counter),
                Increment(&reference_counter))) // first sample sets the reference_counter
            ;
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                        DataSampleSmartPtrCounterMatcher(&reference_counter))))
            .WillOnce(Increment(&reference_counter))
            .WillOnce(Notify(&all_items_received));
    }

    this->_reader->reset(mock_receiver);

    this->startReception(this->getSimulationBus());
    this->_writer->write(*test_sample_1.get());
    this->_writer->write(*test_sample_2.get());
    this->_writer->write(*test_sample_3.get());
    this->_writer->transmit();
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(5)));
    this->stopReception(this->getSimulationBus());
}

/**
 * @detail Test a large sample (video)
 * @req_id FEPSDK-Sample
 */
TYPED_TEST(ReaderWriterTestSimulationBus, VideoSample)
{
    auto video_type = base::StreamType(fep3::base::arya::meta_type_video);
    video_type.setProperty("height", "3840", "uint32_t");
    video_type.setProperty("width", "2160", "uint32_t");
    video_type.setProperty("pixelformat", "R(8)G(9)B(8)", "string");
    video_type.setProperty(
        fep3::base::arya::meta_type_prop_name_max_byte_size, "24883200", "uint32_t");

    auto writer = this->getSimulationBus()->getWriter("video", video_type);
    auto reader = this->getSimulationBus()->getReader("video", video_type);

    EXPECT_TRUE(writer);
    EXPECT_TRUE(reader);

    // std::this_thread::sleep_for(std::chrono::seconds(1));
    test::helper::Notification all_items_received;
    const auto& mock_receiver =
        std::make_shared<::testing::StrictMock<fep3::mock::SimulationBus::DataReceiver>>();

    const data_read_ptr<const IDataSample> image_sample1 = std::make_shared<RandomSample>(24883200);
    const data_read_ptr<const IDataSample> image_sample2 = std::make_shared<RandomSample>(24883200);
    const data_read_ptr<const IDataSample> image_sample3 = std::make_shared<RandomSample>(24883200);

    { // setting of expectations
        ::testing::InSequence sequence;
        // ignore initial stream type
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(::testing::_)))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                        fep3::mock::DataSampleSmartPtrValueMatcher(image_sample1))))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                        fep3::mock::DataSampleSmartPtrValueMatcher(image_sample2))))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                        fep3::mock::DataSampleSmartPtrValueMatcher(image_sample3))))
            .WillOnce(Notify(&all_items_received));
    }

    reader->reset(mock_receiver);

    this->startReception(this->getSimulationBus());
    writer->write(*image_sample1.get());
    writer->write(*image_sample2.get());
    writer->write(*image_sample3.get());
    writer->transmit();
    // For ARMv8 with limited resource, 1 Core and 1 GB RAM, the running time can exceed 13s.
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(15)));
    this->stopReception(this->getSimulationBus());
}

TYPED_TEST(ReaderWriterTestSimulationBus, TestCorrectOrderOfSampleAndStreamType)
{
    OrderTestReceiver sample_receiver;

    std::thread sender_thread([&]() {
        for (uint8_t i = 0; i < 100; i++) {
            this->_writer->write(base::DataSampleType<uint8_t>(i));
            this->_writer->write(base::StreamTypePlain<uint32_t>());
            this->_writer->transmit();
        }
    });

    for (int i = 0; i < 100; i++) {
        this->_reader->pop(sample_receiver);
        this->_reader->pop(sample_receiver);
    }

    sender_thread.join();
}

TYPED_TEST(ReaderWriterTestSimulationBus, TestPopOfDataReader)
{
    test::helper::Notification all_items_received;

    const auto& mock_receiver =
        std::make_shared<::testing::StrictMock<fep3::mock::SimulationBus::DataReceiver>>();
    auto writer = this->getSimulationBus()->getWriter(makePlatformDepName("plain_type"),
                                                      fep3::base::StreamTypePlain<uint32_t>());
    auto reader = this->getSimulationBus2()->getReader(makePlatformDepName("plain_type"),
                                                       fep3::base::StreamTypePlain<uint32_t>());
    reader->reset(mock_receiver);

    EXPECT_CALL(*mock_receiver.get(),
                call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(::testing::_)))
        .WillOnce(DoAll(Notify(&all_items_received), ::testing::Return()));

    this->startReception(this->getSimulationBus2());

    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(5s));

    uint32_t value = 6;
    const auto test_sample = std::make_shared<fep3::base::DataSampleType<uint32_t>>(value);

    {
        ::testing::InSequence sequence;

        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                        fep3::mock::DataSampleSmartPtrValueMatcher(test_sample))))
            .Times(9);

        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                        fep3::mock::DataSampleSmartPtrValueMatcher(test_sample))))
            .WillOnce(Notify(&all_items_received));
    }

    for (int i = 0; i < 10; i++) {
        writer->write(*test_sample.get());
        writer->transmit();
    }

    // wait for 10 samples
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(10s));

    this->stopReception(this->getSimulationBus());
}

/**
 * @detail Test mapping of meta stream type to the associated qos profile in QOS_USER_PROFILE.xml.
 *         All fep defined meta stream type should be covered by a qos profile.
 *         Self-defined types are tried to map if nothing is found, the 'fep3::default_profile' type
 * is used.
 *
 * @req_id FEPSDK-
 */
TYPED_TEST(ReaderWriterTestSimulationBus, TestMetaTypeToQOSProfile)
{
    EXPECT_CALL(*this->_logger_mock.get(), isDebugEnabled())
        .WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*this->_logger_mock.get(), isWarningEnabled())
        .WillRepeatedly(::testing::Return(true));

    EXPECT_CALL(*this->_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Using qos profile 'fep3::ddl' for sample topic 'topic_ddl'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(*this->_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Using qos profile 'fep3::stream_type' for stream type topic 'topic_ddl'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(
        this->getSimulationBus()->getWriter("topic_ddl", fep3::base::StreamTypeDDL("", "")));

    EXPECT_CALL(*this->_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Using qos profile 'fep3::plain-ctype' for sample topic 'topic_plain_c'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(
        *this->_logger_mock.get(),
        logDebug(fep3::mock::LogStringRegexMatcher(
            "Using qos profile 'fep3::stream_type' for stream type topic 'topic_plain_c'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(this->getSimulationBus()->getWriter("topic_plain_c",
                                                    fep3::base::StreamTypePlain<int16_t>()));

    EXPECT_CALL(*this->_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Using qos profile 'fep3::anonymous' for sample topic 'topic_raw'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(*this->_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Using qos profile 'fep3::stream_type' for stream type topic 'topic_raw'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(this->getSimulationBus()->getWriter("topic_raw", fep3::base::StreamTypeRaw()));

    EXPECT_CALL(*this->_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Using qos profile 'fep3::video' for sample topic 'topic_video'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(*this->_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Using qos profile 'fep3::stream_type' for stream type topic 'topic_video'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(this->getSimulationBus()->getWriter(
        "topic_video", fep3::base::StreamType(fep3::base::arya::meta_type_video)));

    EXPECT_CALL(*this->_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Using qos profile 'fep3::audio' for sample topic 'topic_audio'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(*this->_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Using qos profile 'fep3::stream_type' for stream type topic 'topic_audio'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(this->getSimulationBus()->getWriter(
        "topic_audio", fep3::base::StreamType(fep3::base::arya::meta_type_audio)));

    EXPECT_CALL(
        *this->_logger_mock.get(),
        logDebug(fep3::mock::LogStringRegexMatcher(
            "Using qos profile 'fep3::ascii-string' for sample topic 'topic_ascii-string'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(
        *this->_logger_mock.get(),
        logDebug(fep3::mock::LogStringRegexMatcher(
            "Using qos profile 'fep3::stream_type' for stream type topic 'topic_ascii-string'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(
        this->getSimulationBus()->getWriter("topic_ascii-string", fep3::base::StreamTypeString()));

    EXPECT_CALL(*this->_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Using qos profile 'fep3::ddl-array' for sample topic 'topic_ddl-array'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(
        *this->_logger_mock.get(),
        logDebug(fep3::mock::LogStringRegexMatcher(
            "Using qos profile 'fep3::stream_type' for stream type topic 'topic_ddl-array'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(this->getSimulationBus()->getWriter("topic_ddl-array",
                                                    fep3::base::StreamTypeDDLArray("", "", 10)));

    EXPECT_CALL(
        *this->_logger_mock.get(),
        logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::ddl-fileref-array' "
                                                   "for sample topic 'topic_ddl-fileref-array'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(
        *this->_logger_mock.get(),
        logDebug(fep3::mock::LogStringRegexMatcher("Using qos profile 'fep3::stream_type' for "
                                                   "stream type topic 'topic_ddl-fileref-array'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(this->getSimulationBus()->getWriter(
        "topic_ddl-fileref-array", fep3::base::StreamTypeDDLArrayFileRef("", "", 10)));

    EXPECT_CALL(*this->_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Using qos profile 'fep3::ddl-fileref' for sample topic 'topic_ddl-fileref'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(
        *this->_logger_mock.get(),
        logDebug(fep3::mock::LogStringRegexMatcher(
            "Using qos profile 'fep3::stream_type' for stream type topic 'topic_ddl-fileref'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(this->getSimulationBus()->getWriter("topic_ddl-fileref",
                                                    fep3::base::StreamTypeDDLFileRef("", "")));

    EXPECT_CALL(*this->_logger_mock.get(),
                logWarning(fep3::mock::LogStringRegexMatcher(
                    "MetaType 'my_fancy_stream_meta_type' not defined in USER_QOS_PROFILES.xml. "
                    "Using 'fep3::default_profile'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(
        *this->_logger_mock.get(),
        logDebug(fep3::mock::LogStringRegexMatcher(
            "Using qos profile 'fep3::default_profile' for sample topic 'topic_my_fancy'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(
        *this->_logger_mock.get(),
        logDebug(fep3::mock::LogStringRegexMatcher(
            "Using qos profile 'fep3::stream_type' for stream type topic 'topic_my_fancy'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(this->getSimulationBus()->getWriter(
        "topic_my_fancy",
        fep3::base::StreamType(base::StreamMetaType("my_fancy_stream_meta_type"))));

    EXPECT_CALL(*this->_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Using qos profile 'fep3::user_stream_meta_type' for sample topic "
                    "'topic_user_stream_meta_type'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(*this->_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Using qos profile 'fep3::stream_type' for stream type topic "
                    "'topic_user_stream_meta_type'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_TRUE(this->getSimulationBus()->getWriter(
        "topic_user_stream_meta_type",
        fep3::base::StreamType({"user_stream_meta_type", std::list<std::string>{}})));
}

void transmitBigDataSample(const std::unique_ptr<arya::ISimulationBus::IDataWriter>& writer,
                           const std::unique_ptr<arya::ISimulationBus::IDataReader>& reader,
                           TestConnextDDSSimulationBus& test_fixture,
                           fep3::ISimulationBus* simulation_bus)
{
    ASSERT_TRUE(writer);
    ASSERT_TRUE(reader);

    test::helper::Notification all_items_received;
    const auto& mock_receiver =
        std::make_shared<::testing::StrictMock<fep3::mock::SimulationBus::DataReceiver>>();

    const auto max_message_layer_transport_size_bit = 512000;
    const auto sample = std::make_shared<RandomSample>(max_message_layer_transport_size_bit);

    EXPECT_CALL(*mock_receiver.get(),
                call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(::testing::_)))
        .WillOnce(Notify(&all_items_received));

    reader->reset(mock_receiver);
    test_fixture.startReception(simulation_bus);
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(5s));

    EXPECT_CALL(*mock_receiver.get(),
                call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                    fep3::mock::DataSampleSmartPtrValueMatcher(sample))))
        .WillOnce(Notify(&all_items_received));
    writer->write(*sample);
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(5s));
    test_fixture.stopReception(simulation_bus);
}

/*
 * @detail Stream types having a size >= the max transport limit (64KB)
 * shall be transmitted correctly by using the corresponding RTI DDS QoS profile
 * "fep3::stream_type_big".
 * @req_id FEPSDK-2675
 */
TYPED_TEST(ReaderWriterTestSimulationBus, TestBigStreamType)
{
    const auto max_transport_message_size = 63000;
    const std::string stream_type_name = "big_stream_type", property_name = "property_name",
                      property_value = std::string(max_transport_message_size, ' ');
    const auto big_stream_type = std::make_shared<base::StreamType>(stream_type_name);
    big_stream_type->setProperty(property_name, property_value, "string");

    auto reader = this->getSimulationBus()->getReader(stream_type_name, *big_stream_type);

    const auto& mock_receiver =
        std::make_shared<::testing::StrictMock<fep3::mock::SimulationBus::DataReceiver>>();
    reader->reset(mock_receiver);

    test::helper::Notification all_items_received;
    EXPECT_CALL(*mock_receiver.get(),
                call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(
                    fep3::mock::StreamTypeSmartPtrMatcher(big_stream_type))))
        .WillOnce(Notify(&all_items_received));

    this->startReception(this->getSimulationBus());

    auto writer = this->getSimulationBus2()->getWriter(stream_type_name, *big_stream_type);
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(5)));

    EXPECT_CALL(*mock_receiver.get(),
                call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(
                    fep3::mock::StreamTypeSmartPtrMatcher(big_stream_type))))
        .WillOnce(Notify(&all_items_received));
    writer->write(*big_stream_type.get());
    writer->transmit();
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(5)));

    this->stopReception(this->getSimulationBus());
}

/*
 * @detail For plain-array-ctype we expecting a small data qos profile,
 * but we need to make sure that if max_array_size is bigger that we switch to a big data qos.
 * @req_id FEPSDK-2675
 */
TYPED_TEST(ReaderWriterTestSimulationBus, TestOverSizedPlainArray)
{
    auto plain_array_type = base::StreamType(fep3::base::arya::meta_type_plain_array);
    plain_array_type.setProperty("max_array_size", "1000000", "uint32_t");
    plain_array_type.setProperty("datatype", "uint32_t", "string");

    auto writer = this->getSimulationBus()->getWriter("plain_array", plain_array_type);
    auto reader = this->getSimulationBus2()->getReader("plain_array", plain_array_type);

    transmitBigDataSample(writer, reader, *this, this->getSimulationBus2());
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
const auto ddl_description_filename = TEST_FILE_DIR "test_max_signal_size.description";

/*
 * @detail For ddl we expect a small data qos profile,
 * but we need to make sure if the streamtype is too big we switch to the corresponding big data
 * qos.
 * @req_id FEPSDK-3464
 */
TYPED_TEST(ReaderWriterTestSimulationBus, TestOverSizedDDL)
{
    auto ddl_type = base::StreamTypeDDL(struct_name_big, ddl_description);

    auto writer = this->getSimulationBus()->getWriter("ddl", ddl_type);
    auto reader = this->getSimulationBus2()->getReader("ddl", ddl_type);

    transmitBigDataSample(writer, reader, *this, this->getSimulationBus2());
}

/*
 * @detail For ddl-array we expect a small data qos profile,
 * but we need to make sure if the streamtype is too big we switch to the corresponding big data
 * qos.
 * @req_id FEPSDK-3464
 */
TYPED_TEST(ReaderWriterTestSimulationBus, TestOverSizedDDLArray)
{
    const auto max_array_size = 2;
    auto ddl_array_type =
        base::StreamTypeDDLArray(struct_name_small, ddl_description, max_array_size);
    auto writer = this->getSimulationBus()->getWriter("ddl-array", ddl_array_type);
    auto reader = this->getSimulationBus2()->getReader("ddl-array", ddl_array_type);

    transmitBigDataSample(writer, reader, *this, this->getSimulationBus2());
}

/*
 * @detail For ddl-fileref we expect a small data qos profile,
 * but we need to make sure if the streamtype is too big we switch to the corresponding big data
 * qos.
 * @req_id FEPSDK-3464
 */
TYPED_TEST(ReaderWriterTestSimulationBus, TestOverSizedDDLFileref)
{
    auto ddl_type = base::StreamTypeDDLFileRef(struct_name_big, ddl_description_filename);
    auto writer = this->getSimulationBus()->getWriter("ddl-fileref", ddl_type);
    auto reader = this->getSimulationBus2()->getReader("ddl-fileref", ddl_type);

    transmitBigDataSample(writer, reader, *this, this->getSimulationBus2());
}

/*
 * @detail For ddl-fileref-array we expect a small data qos profile,
 * but we need to make sure if the streamtype is too big we switch to the corresponding big data
 * qos.
 * @req_id FEPSDK-3464
 */
TYPED_TEST(ReaderWriterTestSimulationBus, TestOverSizedDDLFilerefArray)
{
    const auto max_array_size = 2;
    auto ddl_type = base::StreamTypeDDLArrayFileRef(
        struct_name_small, ddl_description_filename, max_array_size);
    auto writer = this->getSimulationBus()->getWriter("ddl-fileref-array", ddl_type);
    auto reader = this->getSimulationBus2()->getReader("ddl-fileref-array", ddl_type);

    transmitBigDataSample(writer, reader, *this, this->getSimulationBus2());
}

/*
 * @detail Test logger is correct distributed in RTI DDS Simulation Bus
 *
 * @req_id FEPSDK-2839
 */
TYPED_TEST(ReaderWriterTestSimulationBus, TestLogging)
{
    EXPECT_CALL(*this->_logger_mock.get(), isDebugEnabled())
        .WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*this->_logger_mock.get(), isWarningEnabled())
        .WillRepeatedly(::testing::Return(false));

    EXPECT_CALL(*this->_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Using qos profile 'fep3::anonymous' for sample topic 'my_topic'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(*this->_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Using qos profile 'fep3::stream_type' for stream type topic 'my_topic'.")))
        .WillOnce(::testing::Return(fep3::Result{}));

    auto reader = this->getSimulationBus()->getReader("my_topic");

    reader->reset(std::make_shared<CountSampleTestReceiver>());
    EXPECT_CALL(*this->_logger_mock.get(),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Replaced already registered data receiver for reader from topic 'my_topic'.")))
        .WillOnce(::testing::Return(fep3::Result{}));
    reader->reset(nullptr);
}

/*
 * @detail Test qos change from big streamtype "anonymus" qos to small streamtype qos "c-plain"
 * This include to recreate rti reader and writer to change qos settings
 *
 * @req_id FEPSDK-2892
 */
// TEST_F(ReaderWriterTestClassTimeout, TestChangeFromBigToSmallToBigQOS)
TYPED_TEST(ReaderWriterTestSimulationBus, DISABLED_TestChangeFromBigToSmallToBigQOS)
{
    auto writer =
        this->getSimulationBus()->getWriter("uint32", fep3::base::StreamTypePlain<uint32_t>());
    ASSERT_TRUE(writer);

    auto reader = this->getSimulationBus2()->getReader("uint32");
    ASSERT_TRUE(reader);

    test::helper::Notification all_items_received;
    const auto& mock_receiver =
        std::make_shared<::testing::StrictMock<fep3::mock::SimulationBus::DataReceiver>>();

    uint32_t test_sample_value_1 = 6;
    const data_read_ptr<const IDataSample> test_sample_1 =
        std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value_1);
    uint32_t test_sample_value_2 = 7;
    const data_read_ptr<const IDataSample> test_sample_2 =
        std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value_2);
    uint32_t test_sample_value_3 = 8;
    const data_read_ptr<const IDataSample> test_sample_3 =
        std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value_3);

    { // setting of expectations
        ::testing::InSequence sequence;
        // ignore initial stream type
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(::testing::_)))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                        fep3::mock::DataSampleSmartPtrValueMatcher(test_sample_1))))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                        fep3::mock::DataSampleSmartPtrValueMatcher(test_sample_2))))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                        fep3::mock::DataSampleSmartPtrValueMatcher(test_sample_3))))
            .WillOnce(Notify(&all_items_received));
    }

    reader->reset(mock_receiver);

    this->startReception(this->getSimulationBus2());
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
        ::testing::InSequence sequence;
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(::testing::_)))
            .WillRepeatedly(::testing::Return());

        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                        fep3::mock::DataSampleSmartPtrValueMatcher(image_sample1))))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                        fep3::mock::DataSampleSmartPtrValueMatcher(image_sample2))))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                        fep3::mock::DataSampleSmartPtrValueMatcher(image_sample3))))
            .WillOnce(Notify(&all_items_received));
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
    this->stopReception(this->getSimulationBus2());
}

/*
 * @detail Test that a reader can be requested during a running reception thread
 * This is necessary after changing qos settings
 *
 * @req_id FEPSDK-2892
 */
TYPED_TEST(ReaderWriterTestSimulationBus, TestReaderAfterStartReception)
{
    auto writer =
        this->getSimulationBus()->getWriter("uint32", fep3::base::StreamTypePlain<uint32_t>());
    EXPECT_TRUE(writer);

    test::helper::Notification all_items_received;
    const auto& mock_receiver =
        std::make_shared<::testing::StrictMock<fep3::mock::SimulationBus::DataReceiver>>();

    uint32_t test_sample_value_1 = 6;
    const data_read_ptr<const IDataSample> test_sample_1 =
        std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value_1);

    { // setting of expectations
        ::testing::InSequence sequence;
        // ignore initial stream type
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(::testing::_)))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                        fep3::mock::DataSampleSmartPtrValueMatcher(test_sample_1))))
            .WillOnce(Notify(&all_items_received));
    }

    this->startReception(this->getSimulationBus2());

    auto reader = this->getSimulationBus2()->getReader("uint32");
    reader->reset(mock_receiver);
    EXPECT_TRUE(reader);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    writer->write(*test_sample_1.get());
    writer->transmit();
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(10)));
}

/*
 * @detail Test if a timeout occurs when a specific signal is configured that
 * its reader has to wait for a connecting writer, and no writer connects.
 *
 * @req_id FEPSDK-3174
 */
TEST_F(SignalWaitingTestClass, TestConnectedWriterTimeout)
{
    auto logger_mock = std::make_shared<::testing::StrictMock<fep3::mock::Logger>>();
    EXPECT_CALL((*logger_mock.get()), isDebugEnabled()).WillRepeatedly(::testing::Return(false));
    EXPECT_CALL((*logger_mock.get()), isErrorEnabled()).WillRepeatedly(::testing::Return(true));

    auto sim_participant_name = makePlatformDepName("simbus_participant");

    auto domain_id = randomDomainId();

    auto topic = makePlatformDepName("mytopic");

    auto system_name = makePlatformDepName("mysystem");
    auto simulation_bus =
        createSimulationBus(domain_id,
                            sim_participant_name,
                            system_name,
                            std::chrono::duration_cast<std::chrono::nanoseconds>(1s).count(),
                            topic,
                            logger_mock);

    EXPECT_CALL(
        *logger_mock,
        logError(fep3::mock::LogStringRegexMatcher("Not enough writers connected to reader ")))
        .WillOnce(::testing::Return(fep3::Result{}));

    auto reader = dynamic_cast<ISimulationBus*>(simulation_bus.get())
                      ->getReader(topic, fep3::base::StreamTypePlain<uint32_t>());

    ASSERT_FALSE(reader);
}

/*
 * @detail Test if no timeout occurs when a specific signal is configured that
 * its reader has to wait for a connecting writer, and a writer connects.
 *
 * @req_id FEPSDK-3174
 */
TEST_F(SignalWaitingTestClass, TestConnectedWriterNoTimeout)
{
    auto logger_mock = std::make_shared<::testing::StrictMock<fep3::mock::Logger>>();
    EXPECT_CALL((*logger_mock.get()), isDebugEnabled()).WillRepeatedly(::testing::Return(false));
    EXPECT_CALL((*logger_mock.get()), isErrorEnabled()).WillRepeatedly(::testing::Return(true));

    auto sim_participant_name_w = makePlatformDepName("simbus_participant_w");
    auto sim_participant_name_r = makePlatformDepName("simbus_participant_r");

    auto domain_id = randomDomainId();

    auto topic = makePlatformDepName("mytopic");

    auto system_name = makePlatformDepName("mysystem");
    auto simulation_bus_w = createSimulationBus(domain_id, sim_participant_name_w, system_name);
    auto simulation_bus_r =
        createSimulationBus(domain_id,
                            sim_participant_name_r,
                            system_name,
                            std::chrono::duration_cast<std::chrono::nanoseconds>(1s).count(),
                            topic,
                            logger_mock);

    auto writer = dynamic_cast<ISimulationBus*>(simulation_bus_w.get())
                      ->getWriter(topic, fep3::base::StreamTypePlain<uint32_t>());
    auto reader = dynamic_cast<ISimulationBus*>(simulation_bus_r.get())
                      ->getReader(topic, fep3::base::StreamTypePlain<uint32_t>());

    ASSERT_TRUE(writer);
    ASSERT_TRUE(reader);
}

/*
 * @detail Test if a timeout occurs when all signals are configured that
 * their readers have to wait for a connecting writer, and no writer connects.
 *
 * @req_id FEPSDK-3174
 */
TEST_F(SignalWaitingTestClass, TestConnectedWriterTimeoutAsterisk)
{
    auto logger_mock = std::make_shared<::testing::StrictMock<fep3::mock::Logger>>();
    EXPECT_CALL((*logger_mock.get()), isDebugEnabled()).WillRepeatedly(::testing::Return(false));
    EXPECT_CALL((*logger_mock.get()), isErrorEnabled()).WillRepeatedly(::testing::Return(true));

    auto sim_participant_name = makePlatformDepName("simbus_participant");

    auto domain_id = randomDomainId();

    auto topic = makePlatformDepName("mytopic");

    auto system_name = makePlatformDepName("mysystem");
    auto simulation_bus =
        createSimulationBus(domain_id,
                            sim_participant_name,
                            system_name,
                            std::chrono::duration_cast<std::chrono::nanoseconds>(1s).count(),
                            "*",
                            logger_mock);

    EXPECT_CALL(
        *logger_mock,
        logError(fep3::mock::LogStringRegexMatcher("Not enough writers connected to reader ")))
        .WillOnce(::testing::Return(fep3::Result{}));

    auto reader = dynamic_cast<ISimulationBus*>(simulation_bus.get())
                      ->getReader(topic, fep3::base::StreamTypePlain<uint32_t>());

    ASSERT_FALSE(reader);
}

/*
 * @detail Test if a negative timeout value is handled correctly,
 * i.e. warning is emitted and waiting mechanism is skipped.
 *
 * @req_id FEPSDK-3174
 */
TEST_F(SignalWaitingTestClass, TestConnectedWriterNoTimeoutNegativeVal)
{
    auto logger_mock = std::make_shared<::testing::StrictMock<fep3::mock::Logger>>();
    EXPECT_CALL((*logger_mock.get()), isDebugEnabled()).WillRepeatedly(::testing::Return(false));
    EXPECT_CALL((*logger_mock.get()), isWarningEnabled()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL((*logger_mock.get()), isErrorEnabled()).WillRepeatedly(::testing::Return(true));

    auto sim_participant_name = makePlatformDepName("simbus_participant");

    auto domain_id = randomDomainId();

    auto topic = makePlatformDepName("mytopic");

    auto system_name = makePlatformDepName("mysystem");

    EXPECT_CALL(
        *logger_mock,
        logWarning(
            "Negative timeout value (-1000000000), disabling the waiting for connecting writers"))
        .WillOnce(::testing::Return(fep3::Result{}));

    auto simulation_bus =
        createSimulationBus(domain_id,
                            sim_participant_name,
                            system_name,
                            -std::chrono::duration_cast<std::chrono::nanoseconds>(1s).count(),
                            topic,
                            logger_mock);

    auto reader = dynamic_cast<ISimulationBus*>(simulation_bus.get())
                      ->getReader(topic, fep3::base::StreamTypePlain<uint32_t>());

    ASSERT_TRUE(reader);
}

/*
 * @detail Test the correctness of the case when more signals are listed
 * in the must_be_ready_signals configuration.
 * Two out of three signals are selected and no writers arrive,
 * so two getReader calls should fail with error log.
 *
 * @req_id FEPSDK-3174
 */
TEST_F(SignalWaitingTestClass, TestConnectedWriterTwoTimeoutsOneUnselected)
{
    auto logger_mock = std::make_shared<::testing::StrictMock<fep3::mock::Logger>>();
    EXPECT_CALL((*logger_mock.get()), isDebugEnabled()).WillRepeatedly(::testing::Return(false));
    EXPECT_CALL((*logger_mock.get()), isErrorEnabled()).WillRepeatedly(::testing::Return(true));

    auto sim_participant_name = makePlatformDepName("simbus_participant");

    auto domain_id = randomDomainId();

    auto topic_1 = makePlatformDepName("mytopic_1");
    auto topic_2 = makePlatformDepName("mytopic_2");
    auto topic_3 = makePlatformDepName("mytopic_3");

    auto selected_topics = topic_1 + ";" + topic_2;

    auto system_name = makePlatformDepName("mysystem");
    auto simulation_bus =
        createSimulationBus(domain_id,
                            sim_participant_name,
                            system_name,
                            std::chrono::duration_cast<std::chrono::nanoseconds>(1s).count(),
                            selected_topics,
                            logger_mock);

    EXPECT_CALL(
        *logger_mock,
        logError(fep3::mock::LogStringRegexMatcher("Not enough writers connected to reader ")))
        .Times(2)
        .WillRepeatedly(::testing::Return(fep3::Result{}));

    auto reader_1 = dynamic_cast<ISimulationBus*>(simulation_bus.get())
                        ->getReader(topic_1, fep3::base::StreamTypePlain<uint32_t>());
    auto reader_2 = dynamic_cast<ISimulationBus*>(simulation_bus.get())
                        ->getReader(topic_2, fep3::base::StreamTypePlain<uint32_t>());
    auto reader_3 = dynamic_cast<ISimulationBus*>(simulation_bus.get())
                        ->getReader(topic_3, fep3::base::StreamTypePlain<uint32_t>());

    ASSERT_FALSE(reader_1);
    ASSERT_FALSE(reader_2);
    ASSERT_TRUE(reader_3);
}

/*
 * @detail Test the correctness of use_async_waitset property
 *
 * @req_id FEPSDK-3291
 */
TEST_F(ReaderWriterTestClass, TestAsycWaitSetProperty)
{
    auto logger_mock = std::make_shared<::testing::StrictMock<fep3::mock::Logger>>();
    auto components = std::make_shared<TestConnextDDSSimulationBus::Components>(
        "participant_1", _sim_test_system_name, logger_mock);
    auto sim_bus_component = createSimulationBus(components);
    auto simulation_bus = dynamic_cast<ISimulationBus*>(sim_bus_component.get());
    startReception(simulation_bus);
    EXPECT_EQ(fep3::base::getPropertyValue<bool>(*components->_configuration_service,
                                                 "rti_dds_simulation_bus/use_async_waitset")
                  .value(),
              false);
    // Wait 100ms to stop reception, because the startReception starts a thread, which
    // simulation_bus may not finished with initialization.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopReception(simulation_bus);

    fep3::base::setPropertyValue<bool>(
        *components->_configuration_service, "rti_dds_simulation_bus/use_async_waitset", true);
    startReception(simulation_bus);
    EXPECT_EQ(fep3::base::getPropertyValue<bool>(*components->_configuration_service,
                                                 "rti_dds_simulation_bus/use_async_waitset")
                  .value(),
              true);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopReception(simulation_bus);
}
