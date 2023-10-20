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

#include <fep3/base/sample/mock/mock_data_sample.h>
#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/components/data_registry/mock_data_registry.h>
#include <fep3/core/data/data_reader.h>

#include <gtest_asserts.h>

using namespace fep3;
using namespace std::chrono_literals;

/**
 * Test the data reader constructors
 * @req_id TODO
 */
TEST(TestDataReader, constructorsAndGetName)
{
    { // default constructor
        core::DataReader data_reader;
        EXPECT_EQ("", data_reader.getName());
        EXPECT_EQ(2, data_reader.getSampleQueueCapacity());
    }

    const auto& test_name = std::string{"foo"};

    { // by name and stream type
        core::DataReader data_reader(test_name, base::StreamTypePlain<uint8_t>());
        EXPECT_EQ(test_name, data_reader.getName());
        EXPECT_EQ(2, data_reader.getSampleQueueCapacity());
    }

    const size_t& test_queue_size{10};
    { // by name, stream type and queue size
        core::DataReader data_reader(test_name, base::StreamTypePlain<uint8_t>(), test_queue_size);
        EXPECT_EQ(test_name, data_reader.getName());
        EXPECT_EQ(test_queue_size, data_reader.getSampleQueueCapacity());
    }
}

/**
 * Test adding to and removing from Data Registry
 * @req_id TODO
 */
TEST(TestDataReader, addRemoveToFromDataRegistry)
{
    const std::string& test_signal_name{"foo"};
    const size_t& test_queue_size{10};

    core::DataReader data_reader{
        test_signal_name, base::StreamTypePlain<uint8_t>(), test_queue_size};

    ::testing::StrictMock<mock::DataRegistry> mock_data_registry;
    auto mock_data_registry_data_reader =
        std::make_unique<::testing::StrictMock<mock::DataRegistry::DataReader>>();
    EXPECT_CALL(mock_data_registry,
                registerDataIn(::testing::StrEq(test_signal_name), ::testing::_, false))
        .WillOnce(::testing::Return(Result{}));
    EXPECT_CALL(mock_data_registry, getReader(::testing::StrEq(test_signal_name), test_queue_size))
        .WillOnce(::testing::Return(::testing::ByMove(std::move(mock_data_registry_data_reader))));
    EXPECT_CALL(mock_data_registry, unregisterDataIn(::testing::StrEq(test_signal_name)))
        .WillOnce(::testing::Return(Result{}));

    ASSERT_FEP3_RESULT(data_reader.addToDataRegistry(mock_data_registry), Result{});

    ASSERT_FEP3_RESULT(data_reader.removeFromDataRegistry(mock_data_registry), Result{});
}

/**
 * Test receiving samples and stream types
 * @req_id TODO
 */
TEST(TestDataReader, receive)
{
    const std::string& test_signal_name{"foo"};
    const size_t& test_queue_size{10};

    core::DataReader data_reader{
        test_signal_name, base::StreamTypePlain<uint8_t>(), test_queue_size};

    ::testing::StrictMock<mock::DataRegistry> mock_data_registry;
    auto mock_data_registry_data_reader =
        std::make_unique<::testing::StrictMock<mock::DataRegistry::DataReader>>();
    auto* mock_data_registry_data_reader_ptr = mock_data_registry_data_reader.get();
    EXPECT_CALL(mock_data_registry,
                registerDataIn(::testing::StrEq(test_signal_name), ::testing::_, false))
        .WillOnce(::testing::Return(Result{}));
    EXPECT_CALL(mock_data_registry, getReader(::testing::StrEq(test_signal_name), test_queue_size))
        .WillOnce(::testing::Return(::testing::ByMove(std::move(mock_data_registry_data_reader))));

    ASSERT_FEP3_RESULT(data_reader.addToDataRegistry(mock_data_registry), Result{});

    EXPECT_CALL(*mock_data_registry_data_reader_ptr, getFrontTime())
        .WillOnce(::testing::Return(
            arya::Optional<arya::Timestamp>{})) // no samples / stream types in the queue
        .WillOnce(
            ::testing::Return(arya::Optional<arya::Timestamp>{1ns})) // one sample in the queue
        .WillOnce(::testing::Return(arya::Optional<arya::Timestamp>{1ns})) // sample has value
        .WillOnce(
            ::testing::Return(arya::Optional<arya::Timestamp>{2ns})) // one sample in the queue
        .WillOnce(::testing::Return(arya::Optional<arya::Timestamp>{2ns})) // sample has value
        .WillOnce(
            ::testing::Return(arya::Optional<arya::Timestamp>{3ns})) // one sample in the queue
        .WillOnce(::testing::Return(arya::Optional<arya::Timestamp>{3ns})) // sample has value
        ;

    const auto& mock_data_sample = std::make_shared<::testing::NiceMock<mock::DataSample>>();
    EXPECT_CALL(*mock_data_registry_data_reader_ptr, pop(::testing::_))
        .WillOnce(fep3::mock::arya::Pop(mock_data_sample))
        .WillOnce(fep3::mock::arya::Pop(mock_data_sample));

    // for the first call we don't have any samples / stream types in the queue
    data_reader.receiveNow(1ns);
    EXPECT_EQ(0, data_reader.getSampleQueueSize());

    // for the second call we have three samples in the queue, but only two have a relevant
    // timestamp
    data_reader.receiveNow(3ns);
    EXPECT_EQ(2, data_reader.getSampleQueueSize());
}
