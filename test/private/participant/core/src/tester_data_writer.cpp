/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/base/sample/mock/mock_data_sample.h>
#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/base/stream_type/mock/mock_stream_type.h>
#include <fep3/components/clock/mock_clock_service.h>
#include <fep3/components/data_registry/mock_data_registry.h>
#include <fep3/core/data/data_writer.h>

#include <gtest_asserts.h>

using namespace fep3;

/**
 * Test the data writer backlog constructors
 * @req_id TODO
 */
TEST(TestDataWriter, constructorsAndGetName)
{
    { // default constructor
        core::DataWriter data_writer;
        EXPECT_EQ("", data_writer.getName());
        EXPECT_EQ(core::arya::DATA_WRITER_QUEUE_SIZE_DYNAMIC, data_writer.getQueueSize());
    }

    const auto& test_name = std::string{"foo"};

    { // by name and stream type
        core::DataWriter data_writer(test_name, base::StreamTypePlain<uint8_t>());
        EXPECT_EQ(test_name, data_writer.getName());
        EXPECT_EQ(core::arya::DATA_WRITER_QUEUE_SIZE_DYNAMIC, data_writer.getQueueSize());
    }

    const size_t& test_queue_size{10};
    { // by name, stream type and queue size
        core::DataWriter data_writer(test_name, base::StreamTypePlain<uint8_t>(), test_queue_size);
        EXPECT_EQ(test_name, data_writer.getName());
        EXPECT_EQ(test_queue_size, data_writer.getQueueSize());
    }
}

/**
 * Test adding to and removing from Data Registry
 * @req_id TODO
 */
TEST(TestDataWriter, addRemoveToFromDataRegistry)
{
    const std::string& test_signal_name{"foo"};
    const size_t& test_queue_size{10};

    core::DataWriter data_writer{
        test_signal_name, base::StreamTypePlain<uint8_t>(), test_queue_size};

    ::testing::StrictMock<mock::DataRegistry> mock_data_registry;
    auto mock_data_registry_data_writer =
        std::make_unique<::testing::StrictMock<mock::DataRegistry::DataWriter>>();
    EXPECT_CALL(mock_data_registry,
                registerDataOut(::testing::StrEq(test_signal_name), ::testing::_, false))
        .WillOnce(::testing::Return(Result{}));
    EXPECT_CALL(mock_data_registry, getWriter(::testing::StrEq(test_signal_name), test_queue_size))
        .WillOnce(::testing::Return(::testing::ByMove(std::move(mock_data_registry_data_writer))));
    EXPECT_CALL(mock_data_registry, unregisterDataOut(::testing::StrEq(test_signal_name)))
        .WillOnce(::testing::Return(Result{}));

    ASSERT_FEP3_RESULT(data_writer.addToDataRegistry(mock_data_registry), Result{});

    ASSERT_FEP3_RESULT(data_writer.removeFromDataRegistry(mock_data_registry), Result{});
}

/**
 * Test adding and removing a clock
 * @req_id TODO
 */
TEST(TestDataWriter, addRemoveClock)
{
    const std::string& test_signal_name{"foo"};

    core::DataWriter data_writer{test_signal_name, base::StreamTypePlain<uint8_t>()};

    ::testing::StrictMock<mock::ClockService> mock_clock_service;

    ASSERT_FEP3_RESULT(
        data_writer.addClockTimeGetter([&]() { return mock_clock_service.getTime(); }), Result{});
    ASSERT_FEP3_RESULT(data_writer.removeClockTimeGetter(), Result{});
}

/**
 * Test write and flush
 * @req_id TODO
 */
TEST(TestDataWriter, writeAndFlush)
{
    const std::string& test_signal_name{"foo"};
    const size_t& test_queue_size{3};

    core::DataWriter data_writer{
        test_signal_name, base::StreamTypePlain<uint8_t>(), test_queue_size};

    ::testing::StrictMock<mock::DataRegistry> mock_data_registry;
    auto mock_data_registry_data_writer =
        std::make_unique<::testing::StrictMock<mock::DataRegistry::DataWriter>>();
    auto* mock_data_registry_data_writer_ptr = mock_data_registry_data_writer.get();
    EXPECT_CALL(mock_data_registry,
                registerDataOut(::testing::StrEq(test_signal_name), ::testing::_, false))
        .WillOnce(::testing::Return(Result{}));
    EXPECT_CALL(mock_data_registry, getWriter(::testing::StrEq(test_signal_name), test_queue_size))
        .WillOnce(::testing::Return(::testing::ByMove(std::move(mock_data_registry_data_writer))));

    ASSERT_FEP3_RESULT(data_writer.addToDataRegistry(mock_data_registry), Result{});

    const base::DataSample test_data_sample_1;

    EXPECT_CALL(
        *mock_data_registry_data_writer_ptr,
        write(::testing::Matcher<const IDataSample&>(mock::DataSampleMatcher(test_data_sample_1))))
        .WillOnce(::testing::Return(Result{}));
    uint8_t value{3};
    base::arya::DataSampleType<uint8_t> test_data_sample_2(value);
    test_data_sample_2.setCounter(1);
    EXPECT_CALL(*mock_data_registry_data_writer_ptr,
                write(::testing::Matcher<const IDataSample&>(
                    mock::DataSampleMatcher(::testing::ByRef(test_data_sample_2)))))
        .WillOnce(::testing::Return(Result{}));
    const base::StreamType test_stream_type(base::StreamMetaType("test_stream_meta_type_1"));
    EXPECT_CALL(
        *mock_data_registry_data_writer_ptr,
        write(::testing::Matcher<const IStreamType&>(mock::StreamTypeMatcher(test_stream_type))))
        .WillOnce(::testing::Return(Result{}));

    EXPECT_CALL(*mock_data_registry_data_writer_ptr, flush()).WillOnce(::testing::Return(Result{}));

    // note: writing a sample increments the internal counter of class core::DataWriter
    ASSERT_FEP3_RESULT(data_writer.write(test_data_sample_1), Result{});
    ASSERT_FEP3_RESULT(data_writer.writeByType(value), Result{});
    ASSERT_FEP3_RESULT(data_writer.write(test_stream_type), Result{});
    ASSERT_FEP3_RESULT(data_writer.flushNow(std::chrono::nanoseconds(1)), Result{});

    // with a valid time getter, the time is retrieved from the function and not the sample
    ASSERT_FEP3_RESULT(data_writer.addClockTimeGetter([&]() { return fep3::Timestamp{20}; }),
                       Result{});
    EXPECT_CALL(*mock_data_registry_data_writer_ptr,
                write(::testing::Matcher<const IDataSample&>(::testing::ResultOf(
                    [](auto& sample) { return sample.getTime(); }, fep3::Timestamp{20}))))
        .WillOnce(::testing::Return(Result{}));

    ASSERT_FEP3_RESULT(data_writer.write(test_data_sample_1), Result{});
}
