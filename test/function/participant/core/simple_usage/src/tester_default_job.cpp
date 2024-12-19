/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/components/logging/mock_logger.h>
#include <fep3/components/logging/mock_logging_service.h>
#include <fep3/core.h>
#include <fep3/core/custom_element_factory.h>
#include <fep3/core/participant_executor.hpp>

#include <boost/thread/latch.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <future>
#include <memory>

using namespace std::chrono_literals;
using LoggingSinkMock = testing::NiceMock<fep3::mock::LoggingService::LoggingSink>;
using fep3::mock::LogMessageMatcher;

static std::string msg_to_send = "send from DefaultJob";

namespace {
const auto signal_name_1 = "signal_msg_1";
const auto receiver_participant_name = "test_receiver";
const auto sender_participant_name = "test_sender";
const auto receiver_job_name = "job_receive";
} // namespace

class MyJobSend : public fep3::core::DefaultJob {
public:
    MyJobSend(const std::string& name) : fep3::core::DefaultJob(name)
    {
    }

    void createDataIOs(const fep3::arya::IComponents&,
                       fep3::core::IDataIOContainer& io_container,
                       const fep3::catelyn::JobConfiguration&) override
    {
        _data_writer = io_container.addDataOut(signal_name_1, fep3::base::StreamTypeString(), 1);
    }

    fep3::Result execute(fep3::Timestamp) override
    {
        *_data_writer << msg_to_send;
        return {};
    }

private:
    fep3::core::DataWriter* _data_writer = nullptr;
};

class MyJobReceive : public fep3::core::DefaultJob {
public:
    MyJobReceive(std::promise<void>& signal_received,
                 std::string& value_received,
                 bool& properties_set,
                 const std::string& name)
        : _signal_received(signal_received),
          _value_received(value_received),
          _properties_set(properties_set),
          fep3::core::DefaultJob(name)
    {
    }

    void createDataIOs(const fep3::arya::IComponents&,
                       fep3::core::IDataIOContainer& io_container,
                       const fep3::catelyn::JobConfiguration& job_config) override
    {
        _data_reader =
            io_container.addDataIn(signal_name_1, fep3::base::StreamTypeString(), 1, job_config);
    }

    fep3::Result execute(fep3::Timestamp) override
    {
        fep3::Optional<std::string> value;
        *_data_reader >> value;
        if (value.has_value()) {
            _value_received = value.value();
            _signal_received.set_value();
        }
        return {};
    }

    fep3::Result registerPropertyVariables() override
    {
        FEP3_LOG_INFO(a_util::strings::format("job registerPropertyVariables"));
        FEP3_RETURN_IF_FAILED(registerPropertyVariable(_signal_name, "string_value"));
        _properties_set = true;
        return {};
    }

    fep3::Result unregisterPropertyVariables() override
    {
        FEP3_LOG_INFO(a_util::strings::format("job unregisterPropertyVariables"));
        FEP3_RETURN_IF_FAILED(registerPropertyVariable(_signal_name, "string_value"));
        return {};
    }

private:
    fep3::base::PropertyVariable<std::string> _signal_name{"default_signal"};
    fep3::core::DataReader* _data_reader = nullptr;
    std::promise<void>& _signal_received;
    std::string& _value_received;
    bool& _properties_set;
};

class JobSendElement : public fep3::core::CustomJobElement {
public:
    JobSendElement() : CustomJobElement("job_element")
    {
    }

    std::string getTypename() const override
    {
        return "job_element";
    }
    std::string getVersion() const override
    {
        return "1.0.0";
    }

    std::tuple<fep3::Result, JobPtr, JobConfigPtr> createJob(
        const fep3::arya::IComponents&) override
    {
        return {fep3::Result{},
                std::make_shared<MyJobSend>("job_sender"),
                std::make_unique<fep3::ClockTriggeredJobConfiguration>(1s)};
    }

    fep3::Result destroyJob() override
    {
        return {};
    }
};

class JobReceiveElement : public fep3::core::CustomJobElement {
public:
    JobReceiveElement(std::promise<void>& signal_received,
                      std::string& value_received,
                      bool& properties_set,
                      int value_int,
                      std::string simple_string,
                      std::shared_ptr<int> shared_ptr_int,
                      std::unique_ptr<int> unique_ptr_int)
        : _signal_received(signal_received),
          _value_received(value_received),
          _properties_set(properties_set),
          _value_int(value_int),
          _simple_string(simple_string),
          _shared_ptr_int(shared_ptr_int),
          _unique_ptr_int(std::move(unique_ptr_int)),
          CustomJobElement("job_element")
    {
        (void)_value_int;
    }

    std::string getTypename() const override
    {
        return "job_element";
    }
    std::string getVersion() const override
    {
        return "1.0.0";
    }

    std::tuple<fep3::Result, JobPtr, JobConfigPtr> createJob(
        const fep3::arya::IComponents&) override
    {
        auto config = std::make_unique<fep3::DataTriggeredJobConfiguration>(
            std::vector<std::string>{signal_name_1});

        _job = std::make_shared<MyJobReceive>(
            _signal_received, _value_received, _properties_set, receiver_job_name);

        return {fep3::Result{}, _job, std::move(config)};
    }

    fep3::Result destroyJob() override
    {
        _job.reset();
        return {};
    }

private:
    std::promise<void>& _signal_received;
    std::string& _value_received;
    bool& _properties_set;
    int _value_int;
    std::string _simple_string;
    std::unique_ptr<int> _unique_ptr_int;
    std::shared_ptr<int> _shared_ptr_int;

    std::shared_ptr<MyJobReceive> _job;
};

TEST(DefaultJobAPITest, JobSendReceive_successful)
{
    auto sender_elem_factory = std::make_shared<fep3::core::CustomElementFactory<JobSendElement>>();

    fep3::Participant partsender =
        fep3::base::createParticipant(sender_participant_name,
                                      FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
                                      "system_name",
                                      sender_elem_factory);

    fep3::core::ParticipantExecutor executor_sender(partsender);

    std::promise<void> receive_promise;
    std::future<void> receive_future = receive_promise.get_future();
    std::unique_ptr<int> unique_ptr_int = std::make_unique<int>(100);
    std::shared_ptr<int> shared_ptr_int = std::make_shared<int>(200);

    std::string last_value;
    bool properties_set = false;
    std::string simple_string = "simple string";
    // int x = 1;

    auto receiver_elem_factory =
        std::make_shared<fep3::core::CustomElementFactory<JobReceiveElement>>(
            std::ref(receive_promise),
            std::ref(last_value),
            std::ref(properties_set),
            1,
            simple_string,
            shared_ptr_int,
            std::move(unique_ptr_int));

    fep3::Participant partreceiver =
        fep3::base::createParticipant(receiver_participant_name,
                                      FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
                                      "system_name",
                                      receiver_elem_factory);

    fep3::core::ParticipantExecutor executor_receiver(partreceiver);

    executor_sender.exec();
    executor_receiver.exec();

    ASSERT_TRUE(executor_sender.load());
    ASSERT_TRUE(executor_receiver.load());

    ASSERT_TRUE(properties_set);

    ASSERT_TRUE(executor_sender.initialize());
    ASSERT_TRUE(executor_receiver.initialize());

    ASSERT_TRUE(executor_receiver.deinitialize());
    ASSERT_TRUE(executor_receiver.initialize());

    ASSERT_TRUE(executor_receiver.start());
    ASSERT_TRUE(executor_sender.start());

    std::future_status status = receive_future.wait_for(std::chrono::seconds(5));

    ASSERT_TRUE(executor_sender.stop());
    ASSERT_TRUE(executor_receiver.stop());

    ASSERT_TRUE(executor_sender.deinitialize());
    ASSERT_TRUE(executor_receiver.deinitialize());

    ASSERT_TRUE(executor_sender.unload());
    ASSERT_TRUE(executor_receiver.unload());

    EXPECT_EQ(status, std::future_status::ready) << "No signal was received from test_receiver";
    ASSERT_EQ(last_value, msg_to_send);
}

namespace {
const auto signal_name_2 = "signal_msg_2", signal_name_3 = "signal_msg_3";
const auto reader_queue_size_1 = 5, reader_queue_size_2 = 100, reader_queue_size_3 = 100;
} // namespace

class PurgedSamplesJobSend : public fep3::core::DefaultJob {
public:
    PurgedSamplesJobSend(const std::string& name) : fep3::core::DefaultJob(name)
    {
    }

    void createDataIOs(const fep3::arya::IComponents&,
                       fep3::core::IDataIOContainer& io_container,
                       const fep3::catelyn::JobConfiguration&) override
    {
        _data_writer_1 = io_container.addDataOut(signal_name_1, fep3::base::StreamTypeString(), 1);
        _data_writer_2 = io_container.addDataOut(signal_name_2, fep3::base::StreamTypeString(), 1);
        _data_writer_3 = io_container.addDataOut(signal_name_3, fep3::base::StreamTypeString(), 1);
    }

    fep3::Result execute(fep3::Timestamp) override
    {
        *_data_writer_1 << msg_to_send;
        *_data_writer_2 << msg_to_send;
        *_data_writer_3 << msg_to_send;

        return {};
    }

private:
    fep3::core::DataWriter *_data_writer_1 = nullptr, *_data_writer_2 = nullptr,
                           *_data_writer_3 = nullptr;
};

class PurgedSamplesJobSendElement : public fep3::core::CustomJobElement {
public:
    PurgedSamplesJobSendElement() : CustomJobElement("job_element")
    {
    }

    std::string getTypename() const override
    {
        return "job_element";
    }
    std::string getVersion() const override
    {
        return "1.0.0";
    }

    std::tuple<fep3::Result, JobPtr, JobConfigPtr> createJob(
        const fep3::arya::IComponents&) override
    {
        return {fep3::Result{},
                std::make_shared<PurgedSamplesJobSend>("job_sender"),
                std::make_unique<fep3::ClockTriggeredJobConfiguration>(100ms)};
    }

    fep3::Result destroyJob() override
    {
        return {};
    }
};

class PurgedSamplesJobReceive : public fep3::core::DefaultJob {
public:
    PurgedSamplesJobReceive(std::promise<void>& signals_received, const std::string& name)
        : _signals_received(signals_received), fep3::core::DefaultJob(name)
    {
    }

    void createDataIOs(const fep3::arya::IComponents&,
                       fep3::core::IDataIOContainer& io_container,
                       const fep3::catelyn::JobConfiguration& job_config) override
    {
        _data_reader_1 = io_container.addDataIn(
            signal_name_1, fep3::base::StreamTypeString(), reader_queue_size_1, job_config);
        _data_reader_2 = io_container.addDataIn(
            signal_name_2, fep3::base::StreamTypeString(), reader_queue_size_2, job_config);
        _data_reader_3 = io_container.addDataIn(
            signal_name_3, fep3::base::StreamTypeString(), reader_queue_size_3, job_config);
    }

    fep3::Result execute(fep3::Timestamp) override
    {
        if (_reception_stopped || _data_reader_1->getSampleQueueSize() < reader_queue_size_1 ||
            _data_reader_2->getSampleQueueSize() < 10 ||
            _data_reader_3->getSampleQueueSize() < 10) {
            return {};
        }

        _data_reader_2->purgeAndPopSampleBefore(fep3::Timestamp::max());
        _data_reader_3->clear();

        _reception_stopped = true;
        _signals_received.set_value();

        return {};
    }

private:
    fep3::base::PropertyVariable<std::string> _signal_name{"default_signal"};
    fep3::core::DataReader *_data_reader_1 = nullptr, *_data_reader_2 = nullptr,
                           *_data_reader_3 = nullptr;
    std::promise<void>& _signals_received;
    bool _reception_stopped{false};
};

class PurgedSamplesJobReceiveElement : public fep3::core::CustomJobElement {
public:
    PurgedSamplesJobReceiveElement(std::promise<void>& signal_received,
                                   const std::shared_ptr<LoggingSinkMock>& logging_sink_mock)
        : _signal_received(signal_received),
          _logging_sink_mock(logging_sink_mock),
          CustomJobElement("job_element")
    {
    }

    std::string getTypename() const override
    {
        return "job_element";
    }
    std::string getVersion() const override
    {
        return "1.0.0";
    }

    std::tuple<fep3::Result, JobPtr, JobConfigPtr> createJob(
        const fep3::arya::IComponents&) override
    {
        auto config = std::make_unique<fep3::ClockTriggeredJobConfiguration>(100ms);
        _job = std::make_shared<PurgedSamplesJobReceive>(_signal_received, receiver_job_name);

        return {fep3::Result{}, _job, std::move(config)};
    }

    fep3::Result destroyJob() override
    {
        _job.reset();
        return {};
    }

    fep3::Result initialize(const fep3::arya::IComponents& components)
    {
        auto* logging_service = components.getComponent<fep3::ILoggingService>();
        if (!logging_service) {
            RETURN_ERROR_DESCRIPTION(fep3::ERR_POINTER, "Logging Service unavailable");
        }
        return logging_service->registerSink("logging_sink", _logging_sink_mock);
    }

private:
    std::promise<void>& _signal_received;
    std::shared_ptr<LoggingSinkMock> _logging_sink_mock;
    std::shared_ptr<PurgedSamplesJobReceive> _job;
};

TEST(DefaultJobAPITest, JobLogPurgedSamples_successful)
{
    auto sender_elem_factory =
        std::make_shared<fep3::core::CustomElementFactory<PurgedSamplesJobSendElement>>();

    fep3::Participant part_sender =
        fep3::base::createParticipant(sender_participant_name,
                                      FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
                                      "system_name",
                                      sender_elem_factory);

    fep3::core::ParticipantExecutor executor_sender(part_sender);

    std::promise<void> receive_promise;
    auto receive_future = receive_promise.get_future();
    auto logging_sink_mock = std::make_shared<LoggingSinkMock>();

    auto receiver_elem_factory =
        std::make_shared<fep3::core::CustomElementFactory<PurgedSamplesJobReceiveElement>>(
            std::ref(receive_promise), logging_sink_mock);

    fep3::Participant part_receiver =
        fep3::base::createParticipant(receiver_participant_name,
                                      FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
                                      "system_name",
                                      receiver_elem_factory);

    fep3::core::ParticipantExecutor executor_receiver(part_receiver);

    executor_sender.exec();
    executor_receiver.exec();

    ASSERT_TRUE(executor_sender.load());
    ASSERT_TRUE(executor_receiver.load());

    part_receiver.getComponent<fep3::IConfigurationService>()
        ->getNode(FEP3_LOGGING_DEFAULT_SINKS)
        ->setValue("logging_sink");
    part_sender.getComponent<fep3::IConfigurationService>()
        ->getNode(FEP3_LOGGING_DEFAULT_SINKS)
        ->setValue("");

    ASSERT_TRUE(executor_sender.initialize());
    ASSERT_TRUE(executor_receiver.initialize());

    ASSERT_TRUE(executor_receiver.start());
    ASSERT_TRUE(executor_sender.start());

    EXPECT_EQ(receive_future.wait_for(std::chrono::seconds(10)), std::future_status::ready)
        << "Expected amount of signals have not been received in time.";

    fep3::LogMessage log_message_1{"0",
                                   fep3::LoggerSeverity::warning,
                                   receiver_participant_name,
                                   "job_receive.job",
                                   "purged .* 'signal_msg_1'"};
    fep3::LogMessage log_message_2{"0",
                                   fep3::LoggerSeverity::warning,
                                   receiver_participant_name,
                                   "job_receive.job",
                                   "purged .* 'signal_msg_2'"};
    fep3::LogMessage log_message_3{"0",
                                   fep3::LoggerSeverity::warning,
                                   receiver_participant_name,
                                   "job_receive.job",
                                   "purged .* 'signal_msg_3'"};
    EXPECT_CALL(*logging_sink_mock, log(testing::_))
        .WillRepeatedly(testing::Return(fep3::Result()));
    EXPECT_CALL(*logging_sink_mock, log(LogMessageMatcher(log_message_1))).Times(1);
    EXPECT_CALL(*logging_sink_mock, log(LogMessageMatcher(log_message_2))).Times(1);
    EXPECT_CALL(*logging_sink_mock, log(LogMessageMatcher(log_message_3))).Times(1);

    ASSERT_TRUE(executor_sender.stop());
    ASSERT_TRUE(executor_receiver.stop());

    ASSERT_TRUE(executor_sender.deinitialize());
    ASSERT_TRUE(executor_receiver.deinitialize());

    ASSERT_TRUE(executor_sender.unload());
    ASSERT_TRUE(executor_receiver.unload());
}

class ClearInputSignalsJobSend : public fep3::core::DefaultJob {
public:
    ClearInputSignalsJobSend(const std::string& name) : fep3::core::DefaultJob(name)
    {
    }

    void createDataIOs(const fep3::arya::IComponents&,
                       fep3::core::IDataIOContainer& io_container,
                       const fep3::catelyn::JobConfiguration&) override
    {
        _data_writer = io_container.addDataOut(signal_name_1, fep3::base::StreamTypeString(), 1);
    }

    fep3::Result execute(fep3::Timestamp) override
    {
        assert(_data_writer);
        *_data_writer << msg_to_send;
        return {};
    }

private:
    fep3::core::DataWriter* _data_writer = nullptr;
};

class ClearInputSignalsJobSendElement : public fep3::core::CustomJobElement {
public:
    ClearInputSignalsJobSendElement() : CustomJobElement("job_element")
    {
    }

    std::string getTypename() const override
    {
        return "job_element";
    }
    std::string getVersion() const override
    {
        return "1.0.0";
    }

    std::tuple<fep3::Result, JobPtr, JobConfigPtr> createJob(
        const fep3::arya::IComponents&) override
    {
        return {fep3::Result{},
                std::make_shared<ClearInputSignalsJobSend>("job_sender"),
                std::make_unique<fep3::ClockTriggeredJobConfiguration>(100ms)};
    }

    fep3::Result destroyJob() override
    {
        return {};
    }
};

class ClearInputSignalsJobReceive : public fep3::core::DefaultJob {
public:
    ClearInputSignalsJobReceive(boost::latch& signals_received,
                                bool& test_failed,
                                const std::string& name)
        : _signals_received(signals_received),
          _test_failed(test_failed),
          fep3::core::DefaultJob(name)
    {
    }

    void createDataIOs(const fep3::arya::IComponents&,
                       fep3::core::IDataIOContainer& io_container,
                       const fep3::catelyn::JobConfiguration& job_config) override
    {
        _data_reader = io_container.addDataIn(
            signal_name_1, fep3::base::StreamTypeString(), reader_queue_size_1, job_config);
    }

    fep3::Result execute(fep3::Timestamp) override
    {
        size_t test1 = _data_reader->getSampleQueueSize();

        if (test1 > 1) {
            _test_failed = true;
        }

        _signals_received.count_down();

        return {};
    }

private:
    fep3::core::DataReader* _data_reader = nullptr;
    boost::latch& _signals_received;
    bool& _test_failed;
};

class ClearInputSignalsJobReceiveElement : public fep3::core::CustomJobElement {
public:
    ClearInputSignalsJobReceiveElement(boost::latch& signal_received, bool& test_failed)
        : _signal_received(signal_received),
          _test_failed(test_failed),
          CustomJobElement("job_element")
    {
    }

    std::string getTypename() const override
    {
        return "job_element";
    }
    std::string getVersion() const override
    {
        return "1.0.0";
    }

    std::tuple<fep3::Result, JobPtr, JobConfigPtr> createJob(
        const fep3::arya::IComponents&) override
    {
        auto config = std::make_unique<fep3::DataTriggeredJobConfiguration>(
            std::vector<std::string>{signal_name_1});
        _job = std::make_shared<ClearInputSignalsJobReceive>(
            _signal_received, _test_failed, receiver_job_name);

        return {fep3::Result{}, _job, std::move(config)};
    }

    fep3::Result destroyJob() override
    {
        _job.reset();
        return {};
    }

private:
    boost::latch& _signal_received;
    bool& _test_failed;
    std::shared_ptr<ClearInputSignalsJobReceive> _job;
};

struct DefaultJobClearInputSignalsTest : public ::testing::Test {
protected:
    std::vector<std::shared_ptr<fep3::base::Participant>> createParticipants()
    {
        auto sender_elem_factory =
            std::make_shared<fep3::core::CustomElementFactory<ClearInputSignalsJobSendElement>>();

        auto sender = std::make_shared<fep3::base::Participant>(
            fep3::base::createParticipant(sender_participant_name,
                                          FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
                                          "system_name",
                                          sender_elem_factory));

        auto receiver_elem_factory =
            std::make_shared<fep3::core::CustomElementFactory<ClearInputSignalsJobReceiveElement>>(
                std::ref(_receive_latch), std::ref(_test_failed));

        auto receiver = std::make_shared<fep3::base::Participant>(
            fep3::base::createParticipant(receiver_participant_name,
                                          FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
                                          "system_name",
                                          receiver_elem_factory));

        return {sender, receiver};
    }

    boost::latch _receive_latch{3};
    bool _test_failed{false};
};

/* Test data reader's signal queue, when signals are not removed from the queue and property
 * 'clear_signal_queues' is not set. Expected behavior: The reader's queue grows by each received
 * signal
 */
TEST_F(DefaultJobClearInputSignalsTest, ClearInputSignals_failed)
{
    auto participants = createParticipants();
    fep3::core::ParticipantExecutor sender(*participants.at(0));
    fep3::core::ParticipantExecutor receiver(*participants.at(1));

    sender.exec();
    receiver.exec();

    ASSERT_TRUE(sender.load());
    ASSERT_TRUE(receiver.load());

    ASSERT_TRUE(sender.initialize());
    ASSERT_TRUE(receiver.initialize());

    ASSERT_TRUE(receiver.start());
    ASSERT_TRUE(sender.start());

    _receive_latch.wait();

    ASSERT_TRUE(sender.stop());
    ASSERT_TRUE(receiver.stop());

    ASSERT_TRUE(sender.deinitialize());
    ASSERT_TRUE(receiver.deinitialize());

    ASSERT_TRUE(sender.unload());
    ASSERT_TRUE(receiver.unload());

    EXPECT_TRUE(_test_failed);
}
/* Test data reader's signal queue, when signals are not removed from the queue and property
 * 'clear_signal_queues' is set. Expected behavior: The reader's queue contains only the latest
 * received signal. After each job cycle the queue is cleared automatically.
 */

TEST_F(DefaultJobClearInputSignalsTest, ClearInputSignals_successful)
{
    auto participants = createParticipants();
    fep3::core::ParticipantExecutor sender(*participants.at(0));
    fep3::core::ParticipantExecutor receiver(*participants.at(1));

    sender.exec();
    receiver.exec();

    ASSERT_TRUE(sender.load());
    ASSERT_TRUE(receiver.load());

    /* Set reiver property 'clear_signal_queues' to 'true' */
    participants.at(1)
        ->getComponent<fep3::IConfigurationService>()
        ->getNode(receiver_job_name)
        ->getChild(FEP3_CLEAR_INPUT_SIGNALS_QUEUES_PROPERTY)
        ->setValue("true");

    ASSERT_TRUE(sender.initialize());
    ASSERT_TRUE(receiver.initialize());

    ASSERT_TRUE(receiver.start());
    ASSERT_TRUE(sender.start());

    _receive_latch.wait();

    ASSERT_TRUE(sender.stop());
    ASSERT_TRUE(receiver.stop());

    ASSERT_TRUE(sender.deinitialize());
    ASSERT_TRUE(receiver.deinitialize());

    ASSERT_TRUE(sender.unload());
    ASSERT_TRUE(receiver.unload());

    EXPECT_FALSE(_test_failed);
}
