/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/base/sample/mock/mock_data_sample.h>
#include <fep3/base/stream_type/mock/mock_stream_type.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/core/data/data_reader_backlog.h>

using namespace fep3;
using namespace testing;
using namespace std::chrono_literals;
using fep3::mock::LogStringRegexMatcher;
using StreamTypeMock = NiceMock<mock::StreamType>;
using DataSampleMock = NiceMock<mock::DataSample>;
using LoggerMock = NiceMock<mock::LoggerWithDefaultBehaviour>;

void pushDataSampleToBacklog(core::DataReaderBacklog& data_reader_backlog,
                             Timestamp sample_timestamp)
{
    const auto data_sample_mock = std::make_shared<DataSampleMock>();
    EXPECT_CALL(*data_sample_mock.get(), getTime()).WillRepeatedly(Return(sample_timestamp));
    data_reader_backlog(data_sample_mock);
}

void pushStreamTypeToBacklog(core::DataReaderBacklog& data_reader_backlog,
                             const std::string& stream_type_name)
{
    const auto stream_type_mock = std::make_shared<StreamTypeMock>();
    EXPECT_CALL(*stream_type_mock.get(), getMetaTypeName())
        .Times(AtLeast(0))
        .WillRepeatedly(Return(stream_type_name));
    data_reader_backlog(stream_type_mock);
}

struct TestDataReaderBacklogBase : Test {
    TestDataReaderBacklogBase()
    {
    }

    void SetUp(core::DataReaderBacklog& data_reader_backlog,
               size_t data_reader_capacity,
               const int sample_count)
    {
        data_reader_backlog.setCapacity(data_reader_capacity);

        for (int i = 0, j = sample_count; i < j; i++) {
            pushDataSampleToBacklog(data_reader_backlog, Timestamp{i});
        }

        pushStreamTypeToBacklog(data_reader_backlog, "stream_type_mock");
    }

    StreamTypeMock _stream_type_mock{};
    const LoggerMock _logger_mock;
};

struct TestDataReaderBacklogSetup : TestDataReaderBacklogBase {
    TestDataReaderBacklogSetup() : _data_reader_backlog(1, _stream_type_mock, {})
    {
    }

    void SetUp(size_t data_reader_capacity, const int sample_count)
    {
        TestDataReaderBacklogBase::SetUp(_data_reader_backlog, data_reader_capacity, sample_count);
    }

    core::DataReaderBacklog _data_reader_backlog;
};

/**
 * Test the data reader backlog ctor
 * @req_id TODO
 */
TEST(TestDataReaderBacklog, CTOR)
{
    core::DataReaderBacklog data_reader_backlog{10, StreamTypeMock{}, {}};

    ASSERT_EQ(data_reader_backlog.getSampleQueueCapacity(), 10);
    ASSERT_EQ(data_reader_backlog.getSampleQueueSize(), 0);
}

/**
 * Test the data reader backlog ctor providing an invalid value for sample queue capacity.
 * The sample queue capacity shall be set to 1 on creation.
 * @req_id TODO
 */
TEST(TestDataReaderBacklog, CTOR__invalidCapacity)
{
    core::DataReaderBacklog data_reader_backlog{0, StreamTypeMock{}, {}};

    ASSERT_EQ(data_reader_backlog.getSampleQueueCapacity(), 1);
    ASSERT_EQ(data_reader_backlog.getSampleQueueSize(), 0);
}

/**
 * Test whether stream type items may be pushed to the backlog.
 * @req_id TODO
 */
TEST(TestDataReaderBacklog, pushStreamType)
{
    core::DataReaderBacklog data_reader_backlog{10, StreamTypeMock{}, {}};

    pushStreamTypeToBacklog(data_reader_backlog, "stream_type_mock_1");

    ASSERT_EQ(data_reader_backlog.readType()->getMetaTypeName(), "stream_type_mock_1");

    pushStreamTypeToBacklog(data_reader_backlog, "stream_type_mock_2");

    ASSERT_EQ(data_reader_backlog.readType()->getMetaTypeName(), "stream_type_mock_2");
}

/**
 * Test whether data sample items may be pushed to a backlog
 * if the sample count does not exceed the backlog capacity.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, pushSamplesUntilCapacityReached)
{
    // simplest case
    SetUp(1, 1);

    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 1);
    ASSERT_EQ(_data_reader_backlog.readSampleOldest()->getTime().count(), 0);

    // reach queue capacity
    SetUp(10, 10);

    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 10);
    ASSERT_EQ(_data_reader_backlog.readSampleOldest()->getTime().count(), 0);

    for (int i = 9, j = 0; i >= j; i--) {
        ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), i);
        ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), i);
    }
}

/**
 * Test whether data sample items may be pushed to a backlog
 * if the sample count does exceed the backlog capacity.
 * A sample pushed to a full backlog shall override the oldest sample.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, pushSamplesExceedingCapacity)
{
    // simplest case
    SetUp(1, 2);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 1);
    ASSERT_EQ(_data_reader_backlog.readSampleOldest()->getTime().count(), 1);

    // override first item twice to test whether front item index is updated correctly
    SetUp(10, 21);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 10);
    ASSERT_EQ(_data_reader_backlog.readSampleOldest()->getTime().count(), 11);

    for (int i = 20, j = 11; i >= j; i--) {
        ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), i);
        ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), i - j);
    }
}

/**
 * Test whether the front time of a data reader backlog may be received.
 * Front means the oldest sample of the backlog.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, getFrontTime)
{
    ASSERT_EQ(_data_reader_backlog.getFrontTime(), fep3::arya::Optional<Timestamp>{});

    SetUp(5, 5);
    ASSERT_TRUE(_data_reader_backlog.getFrontTime().has_value());
    ASSERT_EQ(_data_reader_backlog.getFrontTime().value(), Timestamp{0});
}

/**
 * Test whether the sample queue size of a data reader backlog may be retrieved correctly.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, getSampleQueueSize)
{
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 0);

    SetUp(1, 1);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 1);

    SetUp(10, 10);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 10);
}

/**
 * Test whether the sample queue capacity of a data reader backlog may be retrieved correctly.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, getSampleQueueCapacity)
{
    ASSERT_EQ(_data_reader_backlog.getSampleQueueCapacity(), 1);

    ASSERT_EQ(_data_reader_backlog.setCapacity(10), 10);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueCapacity(), 10);

    ASSERT_EQ(_data_reader_backlog.setCapacity(2), 2);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueCapacity(), 2);

    // backlog has to have a capacity > 0
    ASSERT_EQ(_data_reader_backlog.setCapacity(0), 1);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueCapacity(), 1);
}

/**
 * Test whether the latest data samples may be popped from a backlog.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, popLatestDataSample)
{
    SetUp(1, 1);
    ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), 0);

    SetUp(5, 5);
    for (int i = 4, j = 0; i >= j; i--) {
        ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), i);
    }

    // empty backlog returns an invalid data read ptr
    EXPECT_EQ(_data_reader_backlog.popSampleLatest(), data_read_ptr<IDataSample>{});
}

/**
 * Test whether the latest data samples may be popped from a backlog which has overflown and dropped
 * samples.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, popLatestDataSampleOverflow)
{
    SetUp(5, 12);
    for (int i = 11, j = 7; i >= j; i--) {
        ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), i);
    }

    EXPECT_EQ(_data_reader_backlog.popSampleLatest(), data_read_ptr<IDataSample>{});
}

/**
 * Test whether the oldest data samples may be popped from a backlog.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, popOldestDataSample)
{
    SetUp(1, 1);
    ASSERT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), 0);

    SetUp(5, 5);
    for (int i = 0, j = 5; i < j; i++) {
        EXPECT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), i);
    }

    EXPECT_EQ(_data_reader_backlog.popSampleOldest(), data_read_ptr<IDataSample>{});
}

/**
 * Test whether the oldest data samples may be popped from a backlog which has overflown and dropped
 * samples.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, popOldestDataSampleOverflow)
{
    SetUp(5, 12);
    for (int i = 7, j = 11; i <= j; i++) {
        ASSERT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), i);
    }

    EXPECT_EQ(_data_reader_backlog.popSampleOldest(), data_read_ptr<IDataSample>{});
}

/**
 * Test whether the latest/oldest data sample may be read from a backlog.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, readDataSamples)
{
    SetUp(1, 1);
    ASSERT_EQ(_data_reader_backlog.readSampleLatest()->getTime().count(), 0);
    ASSERT_EQ(_data_reader_backlog.readSampleOldest()->getTime().count(), 0);

    SetUp(5, 5);
    ASSERT_EQ(_data_reader_backlog.readSampleLatest()->getTime().count(), 4);
    ASSERT_EQ(_data_reader_backlog.readSampleOldest()->getTime().count(), 0);
}

/**
 * Test whether the oldest data samples may be read from a backlog which has overflown and dropped
 * samples.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, readDataSamplesOverflow)
{
    SetUp(5, 12);
    ASSERT_EQ(_data_reader_backlog.readSampleLatest()->getTime().count(), 11);
    ASSERT_EQ(_data_reader_backlog.readSampleOldest()->getTime().count(), 7);
}

/**
 * Test whether data samples before a given timestamp may be read from a backlog.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, readDataSampleBeforeTimestamp)
{
    SetUp(1, 1);
    ASSERT_EQ(_data_reader_backlog.readSampleBefore(Timestamp{1})->getTime().count(), 0);

    SetUp(5, 5);
    for (int i = 4, j = 0; i > j; i--) {
        ASSERT_EQ(_data_reader_backlog.readSampleBefore(Timestamp{i})->getTime().count(), i - 1);
    }
}

/**
 * Test whether data samples before a given timestamp may be read from a backlog which has overflown
 * and dropped samples.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, readDataSampleBeforeTimestampOverflow)
{
    SetUp(5, 12);
    for (int i = 11, j = 7; i > j; i--) {
        ASSERT_EQ(_data_reader_backlog.readSampleBefore(Timestamp{i})->getTime().count(), i - 1);
    }
}

/**
 * Test whether an empty data sample is returned if reading a sample before a given upper bound
 * which is not available in a backlog.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, readDataSampleBeforeTimestampUnavailable)
{
    SetUp(5, 5);
    ASSERT_EQ(_data_reader_backlog.readSampleBefore(Timestamp{0}), data_read_ptr<IDataSample>{});
}

/**
 * Test whether an empty data sample is returned if read from an empty backlog.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, readPopDataSampleFromEmptyBacklog)
{
    ASSERT_EQ(_data_reader_backlog.popSampleLatest(), data_read_ptr<const IDataSample>{});
    ASSERT_EQ(_data_reader_backlog.popSampleOldest(), data_read_ptr<const IDataSample>{});
    ASSERT_EQ(_data_reader_backlog.readSampleLatest(), data_read_ptr<const IDataSample>{});
    ASSERT_EQ(_data_reader_backlog.readSampleOldest(), data_read_ptr<const IDataSample>{});
    ASSERT_EQ(_data_reader_backlog.readSampleBefore(Timestamp{}),
              data_read_ptr<const IDataSample>{});
}

/**
 * Test whether the latests and oldest data samples may be read from a backlog.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, readStreamType)
{
    SetUp(5, 5);
    ASSERT_EQ(_data_reader_backlog.readType()->getMetaTypeName(), "stream_type_mock");
}

/**
 * Test whether samples before timestamp will be removed and next younger will be popped.
 * @req_id FEPSDK-3107
 */
TEST_F(TestDataReaderBacklogSetup, purgeAndPopSampleBefore)
{
    // Scenario 1 (One tnow-tc sample)
    // Take tnow-tc sample, no purge
    SetUp(1, 1);
    auto sample = _data_reader_backlog.purgeAndPopSampleBefore(1ns);
    ASSERT_EQ(sample->getTime(), 0ns);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 0);

    // Scenario 2 (One tnow-2tc and one tnow-tc sample)
    // Take tnow-tc sample, purge tnow-2tc
    SetUp(2, 2);
    sample = _data_reader_backlog.purgeAndPopSampleBefore(2ns);
    ASSERT_EQ(sample->getTime(), 1ns);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 0);

    // Scenario 3 (One tnow-tc and one tnow sample)
    // Take tnow-tc sample, no purge
    SetUp(2, 2);
    sample = _data_reader_backlog.purgeAndPopSampleBefore(1ns);
    ASSERT_EQ(sample->getTime(), 0ns);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 1);

    // Scenario 4 (One tnow-2tc, One tnow-tc and one tnow sample)
    // Take tnow-tc sample, purge tnow-2tc, leave tnow
    SetUp(3, 3);
    sample = _data_reader_backlog.purgeAndPopSampleBefore(2ns);
    ASSERT_EQ(sample->getTime(), 1ns);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 1);

    // Scenario 5 (One tnow sample)
    // Don't take the sample, leave it in queue
    SetUp(1, 1);
    sample = _data_reader_backlog.purgeAndPopSampleBefore(0ns);
    ASSERT_EQ(sample, data_read_ptr<IDataSample>{});
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 1);

    // Scenario 6 (No sample)
    // There is nothing to take
    SetUp(0, 0);
    sample = _data_reader_backlog.purgeAndPopSampleBefore(0ns);
    ASSERT_EQ(sample, data_read_ptr<IDataSample>{});
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 0);

    // Longer queue, similar to scenario 4
    SetUp(5, 5);
    sample = _data_reader_backlog.purgeAndPopSampleBefore(3ns);
    ASSERT_EQ(sample->getTime(), 2ns);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 2);

    // Picking from the middle of the queue
    SetUp(5, 5);
    sample = _data_reader_backlog.purgeAndPopSampleBefore(2ns);
    ASSERT_EQ(sample->getTime(), 1ns);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 3);

    // Only newer samples than we are looking for, similar to scenario 5
    // Setup is "overloading" the buffer, the first two samples are lost
    SetUp(3, 5);
    sample = _data_reader_backlog.purgeAndPopSampleBefore(1ns);
    ASSERT_EQ(sample, data_read_ptr<IDataSample>{});
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 3);
}

/**
 * Test whether popOldest and write of samples works in combination.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, popOldestWriteDataSample)
{
    SetUp(5, 8);
    for (int i = 3, j = 6; i < j; i++) {
        ASSERT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), i);
    }

    // refill backlog
    for (int i = 8, j = 10; i < j; i++) {
        pushDataSampleToBacklog(_data_reader_backlog, Timestamp{i});
    }
    for (int i = 6, j = 10; i < j; i++) {
        ASSERT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), i);
    }

    // overfill backlog and overwrite 2 samples
    for (int i = 10, j = 17; i < j; i++) {
        pushDataSampleToBacklog(_data_reader_backlog, Timestamp{i});
    }
    for (int i = 12, j = 17; i < j; i++) {
        ASSERT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), i);
    }

    // override front item twice to test whether front item index is updated correctly
    for (int i = 17, j = 28; i < j; i++) {
        pushDataSampleToBacklog(_data_reader_backlog, Timestamp{i});
    }
    for (int i = 23, j = 28; i < j; i++) {
        ASSERT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), i);
    }
}

/**
 * Test whether popLatest and writing of samples works in combination.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, popLatestWriteDataSample)
{
    SetUp(5, 8);

    for (int i = 7, j = 4; i > j; i--) {
        ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), i);
    }

    // overwrite existing samples
    for (int i = 8, j = 10; i < j; i++) {
        pushDataSampleToBacklog(_data_reader_backlog, Timestamp{i});
    }

    for (int i = 9, j = 7; i > j; i--) {
        ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), i);
    }

    // overfill backlog and overwrite 2 samples
    for (int i = 10, j = 17; i < j; i++) {
        pushDataSampleToBacklog(_data_reader_backlog, Timestamp{i});
    }

    for (int i = 16, j = 11; i > j; i--) {
        ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), i);
    }
}

/**
 * Test whether popLatest, popOldest and writing of samples works in combination.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, popLatestOldestWriteDataSample)
{
    SetUp(5, 8);

    // empty backlog using popOldest and popLatest in combination
    ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), 7);
    ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), 6);
    ASSERT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), 3);
    ASSERT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), 4);
    ASSERT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), 5);

    // overfill backlog and overwrite 2 samples
    for (int i = 10, j = 17; i < j; i++) {
        pushDataSampleToBacklog(_data_reader_backlog, Timestamp{i});
    }

    // empty backlog using popOldest and popLatest in combination
    ASSERT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), 12);
    ASSERT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), 13);
    ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), 16);
    ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), 15);
    ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), 14);

    // overfill backlog and overwrite 4 samples
    for (int i = 17, j = 26; i < j; i++) {
        pushDataSampleToBacklog(_data_reader_backlog, Timestamp{i});
    }

    // empty backlog using popOldest and popLatest in combination
    ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), 25);
    ASSERT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), 21);
    ASSERT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), 22);
    ASSERT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), 23);
    ASSERT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), 24);

    // overfill backlog and overwrite 2 samples
    for (int i = 26, j = 33; i < j; i++) {
        pushDataSampleToBacklog(_data_reader_backlog, Timestamp{i});
    }

    // empty backlog using popOldest and popLatest in combination
    ASSERT_EQ(_data_reader_backlog.popSampleOldest()->getTime().count(), 28);
    ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), 32);
    ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), 31);
    ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), 30);
    ASSERT_EQ(_data_reader_backlog.popSampleLatest()->getTime().count(), 29);
}

/**
 * Test whether the backlog's data sample capacity may be changed after creation.
 * Resizing with a value <= 0 shall lead to a capacity of 1.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, resize)
{
    ASSERT_EQ(_data_reader_backlog.getSampleQueueCapacity(), 1);

    ASSERT_EQ(_data_reader_backlog.setCapacity(20), 20);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueCapacity(), 20);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 0);

    ASSERT_EQ(_data_reader_backlog.setCapacity(0), 1);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueCapacity(), 1);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 0);

    SetUp(5, 5);

    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 5);

    ASSERT_EQ(_data_reader_backlog.setCapacity(0), 1);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueCapacity(), 1);
    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 0);
}

/**
 * Test whether the backlog can be cleared. The sample queue size shall be 0 after calling clear.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogSetup, clear)
{
    SetUp(5, 5);

    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 5);

    _data_reader_backlog.clear();

    ASSERT_EQ(_data_reader_backlog.getSampleQueueSize(), 0);
}

/**
 * Test whether the backlog logs information regarding samples purged by ther user calling
 * purgeAndPopSampleBefore.
 * 1 sample is pushed to the backlog.
 * 0 samples are purged by the user
 * calling purgeAndPopSampleBefore.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogBase, logPurgedSamples__purgedByPop__noSamplesPurged)
{
    auto backlog_capacity = 1, purged_sample_info_capacity = 2;
    core::DataReaderBacklog data_reader_backlog(
        backlog_capacity, _stream_type_mock, purged_sample_info_capacity);

    SetUp(data_reader_backlog, backlog_capacity, backlog_capacity);

    auto sample =
        data_reader_backlog.purgeAndPopSampleBefore(std::chrono::nanoseconds(backlog_capacity));
    ASSERT_EQ(sample->getTime(), 0ns);

    EXPECT_CALL(_logger_mock, logDebug(HasSubstr("No samples"))).Times(1);
    data_reader_backlog.logPurgedSamples("signal", &_logger_mock);
}

/**
 * Test whether the backlog logs information regarding samples purged by ther user calling
 * purgeAndPopSampleBefore.
 * 3 samples are pushed to the backlog.
 * 2 samples are purged by the user
 * calling purgeAndPopSampleBefore.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogBase, logPurgedSamples__purgedByPop__samplesPurged)
{
    auto backlog_capacity = 3, purged_sample_info_capacity = 3;
    ;
    core::DataReaderBacklog data_reader_backlog(
        backlog_capacity, _stream_type_mock, purged_sample_info_capacity);

    SetUp(data_reader_backlog, backlog_capacity, backlog_capacity);

    auto sample =
        data_reader_backlog.purgeAndPopSampleBefore(std::chrono::nanoseconds(backlog_capacity));
    ASSERT_EQ(sample->getTime(), 2ns);

    EXPECT_CALL(_logger_mock, logWarning(LogStringRegexMatcher("'2' samples .* '0, 1'"))).Times(1);
    data_reader_backlog.logPurgedSamples("signal", &_logger_mock);
}

/**
 * Test whether the backlog logs information regarding the purged sample buffer being reached
 * if more samples are purged than the buffer can store.
 * 3 samples are pushed to the backlog having a capacity of 3.
 * 2 samples are purged by calling purgeAndPopSampleBefore but added only partly to the purge sample
 * buffer which has a capacity of 1. A warning shall be logged regarding the buffer limit being
 * reached.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogBase, logPurgedSamples__purgedByPop__bufferLimitReached)
{
    auto backlog_capacity = 3, purged_sample_info_capacity = 1;
    ;
    core::DataReaderBacklog data_reader_backlog(
        backlog_capacity, _stream_type_mock, purged_sample_info_capacity);

    SetUp(data_reader_backlog, backlog_capacity, backlog_capacity);

    auto sample =
        data_reader_backlog.purgeAndPopSampleBefore(std::chrono::nanoseconds(backlog_capacity));
    ASSERT_EQ(sample->getTime(), 2ns);

    EXPECT_CALL(_logger_mock, logWarning(_)).Times(AnyNumber());
    EXPECT_CALL(_logger_mock, logWarning(HasSubstr("reached its limit"))).Times(1);
    data_reader_backlog.logPurgedSamples("signal", &_logger_mock);
}

/**
 * Test whether the backlog logs information regarding samples purged by ther user calling clear.
 * 0 samples are pushed to the backlog.
 * 0 samples are purged by the user calling clear.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogBase, logPurgedSamples__purgedByClear__noSamplesPurged)
{
    auto backlog_capacity = 1, purged_sample_info_capacity = 1;
    core::DataReaderBacklog data_reader_backlog(
        backlog_capacity, _stream_type_mock, purged_sample_info_capacity);

    data_reader_backlog.clear();

    EXPECT_CALL(_logger_mock, logDebug(HasSubstr("No samples"))).Times(1);
    data_reader_backlog.logPurgedSamples("signal", &_logger_mock);
}

/**
 * Test whether the backlog logs information regarding samples purged by ther user calling clear.
 * 2 samples are pushed to the backlog.
 * 2 samples are purged by the user calling clear.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogBase, logPurgedSamples__purgedByClear__samplesPurged)
{
    auto backlog_capacity = 2, purged_sample_info_capacity = 3;
    core::DataReaderBacklog data_reader_backlog(
        backlog_capacity, _stream_type_mock, purged_sample_info_capacity);

    SetUp(data_reader_backlog, backlog_capacity, backlog_capacity);

    data_reader_backlog.clear();

    EXPECT_CALL(_logger_mock, logWarning(LogStringRegexMatcher("'2' samples .* '0, 1'"))).Times(1);
    data_reader_backlog.logPurgedSamples("signal", &_logger_mock);
}

/**
 * Test whether the backlog logs information regarding the purged sample buffer being reached
 * if more samples are purged than the buffer can store.
 * 2 samples are pushed to the backlog having a capacity of 2.
 * 2 samples are purged by calling clear but added only partly to the purge sample buffer which has
 * a capacity of 1. A warning shall be logged regarding the buffer limit being reached.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogBase, logPurgedSamples__purgedByClear__bufferLimitReached)
{
    auto backlog_capacity = 2, purged_sample_info_capacity = 1,
         sample_count = purged_sample_info_capacity + 1;
    core::DataReaderBacklog data_reader_backlog(
        backlog_capacity, _stream_type_mock, purged_sample_info_capacity);

    SetUp(data_reader_backlog, backlog_capacity, sample_count);

    data_reader_backlog.clear();

    EXPECT_CALL(_logger_mock, logWarning(_)).Times(AnyNumber());
    EXPECT_CALL(_logger_mock, logWarning(HasSubstr("reached its limit"))).Times(1);
    data_reader_backlog.logPurgedSamples("signal", &_logger_mock);
}

/**
 * Test whether the backlog logs information regarding samples purged automatically during
 * reception.
 * 1 sample is pushed to the backlog having a capacity of 1.
 * 0 samples are purged automatically.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogBase, logPurgedSamples__purgedAutomatically__noSamplesPurged)
{
    auto backlog_capacity = 1, purged_sample_info_capacity = 2;
    core::DataReaderBacklog data_reader_backlog(
        backlog_capacity, _stream_type_mock, purged_sample_info_capacity);

    SetUp(data_reader_backlog, backlog_capacity, backlog_capacity);

    EXPECT_CALL(_logger_mock, logDebug(HasSubstr("No samples"))).Times(1);
    data_reader_backlog.logPurgedSamples("signal", &_logger_mock);
}

/**
 * Test whether the backlog logs information regarding samples purged automatically during
 * reception.
 * 4 samples are pushed to the backlog having a capacity of 2.
 * 2 samples are purged automatically because of the backlog capacity being too small.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogBase, logPurgedSamples__purgedAutomatically__samplesPurged)
{
    auto backlog_capacity = 2, purged_sample_info_capacity = 3, sample_count = backlog_capacity + 2;
    core::DataReaderBacklog data_reader_backlog(
        backlog_capacity, _stream_type_mock, purged_sample_info_capacity);

    SetUp(data_reader_backlog, backlog_capacity, sample_count);

    EXPECT_CALL(_logger_mock, logWarning(LogStringRegexMatcher("'2' samples .* '0, 1'"))).Times(1);
    data_reader_backlog.logPurgedSamples("signal", &_logger_mock);
}

/**
 * Test whether the backlog logs information regarding the purged sample buffer being reached
 * if more samples are purged than the buffer can store.
 * 3 samples are pushed to the backlog having a capacity of 1.
 * 2 samples are purged automatically but added only partly to the purge sample buffer which has a
 * capacity of 1. A warning shall be logged regarding the buffer limit being reached.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogBase, logPurgedSamples__purgedAutomatically__bufferLimitReached)
{
    auto backlog_capacity = 1, purged_sample_info_capacity = 1,
         sample_count = backlog_capacity + purged_sample_info_capacity + 1;
    core::DataReaderBacklog data_reader_backlog(
        backlog_capacity, _stream_type_mock, purged_sample_info_capacity);

    SetUp(data_reader_backlog, backlog_capacity, sample_count);

    EXPECT_CALL(_logger_mock, logWarning(_)).Times(AnyNumber());
    EXPECT_CALL(_logger_mock, logWarning(HasSubstr("reached its limit"))).Times(1);
    data_reader_backlog.logPurgedSamples("signal", &_logger_mock);
}

/**
 * Test whether the backlog skips logging information regarding purged samples
 * if purged sample buffer capacity is not provided.
 * 3 samples are pushed to the backlog having a capacity of 2.
 * 1 sample is purged silently when popping a sample and 1 sample is purged silently
 * when calling clear.
 * @req_id TODO
 */
TEST_F(TestDataReaderBacklogBase, logPurgedSamples__loggingDisabled)
{
    auto backlog_capacity = 2, sample_count = backlog_capacity + 1;
    core::DataReaderBacklog data_reader_backlog(backlog_capacity, _stream_type_mock, 0);

    SetUp(data_reader_backlog, backlog_capacity, sample_count);

    EXPECT_TRUE(
        data_reader_backlog.purgeAndPopSampleBefore(std::chrono::nanoseconds(backlog_capacity)));
    data_reader_backlog.clear();

    EXPECT_CALL(_logger_mock, logWarning(_)).Times(0);
    data_reader_backlog.logPurgedSamples("signal", &_logger_mock);
}
