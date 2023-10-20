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

#include <fep3/base/sample/data_sample.h>
#include <fep3/base/sample/mock/mock_data_sample.h>
#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/base/stream_type/mock/mock_stream_type.h>
#include <fep3/components/simulation_bus/mock_simulation_bus.h>
#include <fep3/native_components/simulation_bus/simbus_datareader.h>

#include <future>
#include <gtest_asserts.h>
#include <helper/gmock_async_helper.h>

using namespace fep3;

namespace {

class DataSampleNumber : public base::DataSample {
public:
    explicit DataSampleNumber(uint32_t order)
    {
        auto memory = fep3::base::RawMemoryStandardType<uint32_t>(order);

        this->write(memory);
    }
};

MATCHER_P(StreamTypeMatcher, other, "Equality matcher for IStreamType")
{
    return arg == other;
}

struct FillTheReceiverQueueData {
    std::shared_ptr<fep3::native::DataItemQueue<>> item_queue;
    std::shared_ptr<fep3::mock::DataSample> sample;
};

ACTION_P(FillTheReceiverQueue, data)
{
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);

    data.item_queue->push(data.sample);
}

/**
 * This mock is able to use the fep3::mock::DataSampleMatcher
 */
class DataReceiver : public ISimulationBus::IDataReceiver {
public:
    MOCK_METHOD1(onSampleReceived, void(const IDataSample& type));
    MOCK_METHOD1(onStreamTypeReceived, void(const IStreamType& type));

    void operator()(const data_read_ptr<const IStreamType>& type) override
    {
        onStreamTypeReceived(*type);
    }

    void operator()(const data_read_ptr<const IDataSample>& sample) override
    {
        onSampleReceived(*sample);
    }
};

} // namespace

/**
 * @detail Test the stopping of the DataReader
 * @req_id FEPSDK-SimulationBus
 */
TEST(NativeSimulationBus, testDataTriggeredReception)
{
    const data_read_ptr<const IDataSample> sample = std::make_shared<base::DataSample>(0, true);
    const data_read_ptr<const IStreamType> stream_type =
        std::make_shared<base::StreamTypeDDL>("my_ddl_uint8", "Z:/fileref.ddl");

    const std::string signal_1_name = "signal_1";
    const size_t queue_size = 5;

    auto sim_bus = std::make_shared<fep3::native::SimulationBus>();
    auto reader = sim_bus->getReader(signal_1_name, queue_size);
    auto writer = sim_bus->getWriter(signal_1_name, queue_size);

    const auto& receiver1 =
        std::make_shared<::testing::StrictMock<fep3::mock::SimulationBus::DataReceiver>>();
    reader->reset(receiver1);

    test::helper::Notification done1;
    { // setting of expectations
        EXPECT_CALL(*receiver1.get(),
                    call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(
                        mock::DataSampleSmartPtrMatcher(sample))))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*receiver1.get(),
                    call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(
                        mock::StreamTypeSmartPtrMatcher(stream_type))))
            .WillOnce(Notify(&done1));
    }

    std::promise<void> blocking_reception_prepared;
    auto blocking_reception_prepared_result = blocking_reception_prepared.get_future();
    std::thread t1([sim_bus, &blocking_reception_prepared]() {
        sim_bus->startBlockingReception(
            [&blocking_reception_prepared]() { blocking_reception_prepared.set_value(); });
    });
    // wait for the blocking reception to be prepared
    blocking_reception_prepared_result.get();
    // transmitting a sample must trigger the receiver
    writer->write(*sample);
    writer->write(*stream_type);
    writer->transmit();
    done1.waitForNotificationWithTimeout(std::chrono::seconds(3));
    sim_bus->stopBlockingReception();
    t1.join();

    // remove the receiver
    reader->reset();

    bool reception_preparation_done_callback_called = false;
    // if no reader has a receiver set, startBlockingReception must not block
    sim_bus->startBlockingReception([&reception_preparation_done_callback_called]() {
        reception_preparation_done_callback_called = true;
    });
    EXPECT_TRUE(reception_preparation_done_callback_called);
    // transmitting another sample must not trigger the receiver
    // (because it has been removed from the reader)
    writer->write(*sample);
    writer->transmit();
    sim_bus->stopBlockingReception();
}

/**
 * @detail Test transmission of arbitrary data
 * @req_id FEPSDK-SimulationBus
 */
TEST(NativeSimulationBus, testTransmission)
{
    const data_read_ptr<const IDataSample> sample = std::make_shared<base::DataSample>(0, true);
    const base::StreamTypeDDL ddltype("my_ddl_uint8", "Z:/fileref.ddl");

    const std::string signal_1_name = "signal_1";
    const size_t queue_size = 5;

    auto sim_bus = std::make_shared<fep3::native::SimulationBus>();
    auto reader = sim_bus->getReader(signal_1_name, queue_size);
    auto writer = sim_bus->getWriter(signal_1_name, queue_size);

    writer->write(*sample);
    writer->write(ddltype);
    writer->write(*sample);

    writer->transmit();

    fep3::mock::SimulationBus::DataReceiver receiver;
    using ::testing::_;
    using MatchIDataSample = testing::Matcher<const data_read_ptr<const IDataSample>&>;
    using MatchIStreamType = testing::Matcher<const data_read_ptr<const IStreamType>&>;

    EXPECT_CALL(receiver, call(MatchIDataSample(_))).Times(2);
    EXPECT_CALL(receiver, call(MatchIStreamType(_))).Times(1);

    while (reader->pop(receiver))
        ;
}

/**
 * @detail Test overflow of reader queue. Test sample loss.
 * @req_id FEPSDK-SimulationBus
 */
TEST(NativeSimulationBus, testTransmissionOfStreamType)
{
    const std::string signal_1_name = "signal_1";
    const size_t queue_size = 5;

    auto sim_bus = std::make_shared<fep3::native::SimulationBus>();
    auto reader = sim_bus->getReader(signal_1_name, queue_size);
    auto writer = sim_bus->getWriter(signal_1_name, queue_size);

    base::StreamTypeDDL ddltype("my_ddl_uint8", "Z:/fileref.ddl");
    writer->write(ddltype);

    writer->transmit();

    DataReceiver receiver;

    {
        ::testing::InSequence sequence;
        using M = ::testing::Matcher<const IStreamType&>;
        EXPECT_CALL(receiver, onStreamTypeReceived(M(StreamTypeMatcher(ddltype)))).Times(1);

        EXPECT_CALL(receiver, onStreamTypeReceived(::testing::_)).Times(0);
    }

    while (reader->pop(receiver))
        ;
}

/**
 * @detail Test whether a data reader may be retrieved multiple times from the simulation bus
 * if the simulation bus has been deinitialized between requests.
 * @req_id FEPSDK-SimulationBus
 */
TEST(NativeSimulationBus, testRegisterDataReaderWriterTwice)
{
    const std::string signal_name = "signal_name";

    fep3::native::SimulationBus sim_bus;

    ASSERT_FEP3_NOERROR(sim_bus.initialize());

    EXPECT_TRUE(sim_bus.getReader(signal_name));
    EXPECT_TRUE(sim_bus.getWriter(signal_name));
    // a second request of data readers/writers shall not succeed
    EXPECT_TRUE(nullptr == sim_bus.getReader(signal_name));
    EXPECT_TRUE(nullptr == sim_bus.getWriter(signal_name));

    ASSERT_FEP3_NOERROR(sim_bus.deinitialize());

    // after deinitialization, data readers/writers may be requested again
    EXPECT_TRUE(sim_bus.getReader(signal_name));
    EXPECT_TRUE(sim_bus.getWriter(signal_name));
}

namespace {
class SimpleDataSample : public ::testing::Test {
protected:
    SimpleDataSample()
    {
        _sim_bus = std::make_shared<fep3::native::SimulationBus>();

        _samples.reserve(10);

        const uint32_t magic_number = 455;

        for (uint32_t i = magic_number; i < magic_number + _sample_number; i++) {
            _samples.push_back(DataSampleNumber{i});
        }
    }

    const uint32_t _sample_number = 10;
    std::vector<DataSampleNumber> _samples;

    std::shared_ptr<fep3::native::SimulationBus> _sim_bus{nullptr};
};
} // namespace

/**
 * @detail Test overflow of reader queue. Test sample loss.
 * @req_id FEPSDK-SimulationBus
 */
TEST_F(SimpleDataSample, testOverflowReaderQueue)
{
    const std::string signal_1_name{"signal_1"};

    const size_t queue_size = 5;

    auto reader = _sim_bus->getReader(signal_1_name, 1);
    auto writer = _sim_bus->getWriter(signal_1_name, queue_size);

    for (auto sample: _samples) {
        writer->write(sample);
    }

    writer->transmit();

    DataReceiver receiver;

    {
        ::testing::InSequence sequence;
        using M = ::testing::Matcher<const IDataSample&>;
        EXPECT_CALL(
            receiver,
            onSampleReceived(M(fep3::mock::DataSampleMatcher(_samples[_samples.size() - 1]))))
            .Times(1);

        EXPECT_CALL(receiver, onSampleReceived(::testing::_)).Times(0);
    }

    while (reader->pop(receiver))
        ;
}

/**
 * @detail Test overflow of writer queue. Test sample loss.
 * @req_id FEPSDK-SimulationBus
 */
TEST_F(SimpleDataSample, testOverflowWriterQueue)
{
    const std::string signal_1_name{"signal_1"};

    const size_t queue_size = 5;

    auto reader = _sim_bus->getReader(signal_1_name, queue_size);
    auto writer = _sim_bus->getWriter(signal_1_name, 1);

    for (auto sample: _samples) {
        writer->write(sample);
    }

    writer->transmit();

    DataReceiver receiver;

    {
        ::testing::InSequence sequence;
        using M = ::testing::Matcher<const IDataSample&>;
        EXPECT_CALL(
            receiver,
            onSampleReceived(M(fep3::mock::DataSampleMatcher(_samples[_samples.size() - 1]))))
            .Times(1);

        EXPECT_CALL(receiver, onSampleReceived(::testing::_)).Times(0);
    }

    while (reader->pop(receiver))
        ;
}
