/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2023 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/core.h>
#include <fep3/core/custom_element_factory.h>
#include <fep3/core/participant_executor.hpp>

#include <gtest/gtest.h>

#include <future>
#include <memory>

static std::string msg_to_send = "send from DefaultJob";

class MyJobSend : public fep3::core::DefaultJob {
public:
    MyJobSend(const std::string& name) : fep3::core::DefaultJob(name)
    {
    }

    void createDataIOs(const fep3::arya::IComponents&,
                       fep3::core::IDataIOContainer& io_container,
                       const fep3::catelyn::JobConfiguration&) override
    {
        _data_writer = io_container.addDataOut("signal_msg", fep3::base::StreamTypeString(), 1);
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
            io_container.addDataIn("signal_msg", fep3::base::StreamTypeString(), 1, job_config);
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

    std::tuple<fep3::Result, JobPtr, JobConfigPtr> createJob() override
    {
        using namespace std::chrono_literals;
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

    std::tuple<fep3::Result, JobPtr, JobConfigPtr> createJob() override
    {
        auto config = std::make_unique<fep3::DataTriggeredJobConfiguration>(
            std::vector<std::string>{"signal_msg"});

        _job = std::make_shared<MyJobReceive>(
            _signal_received, _value_received, _properties_set, "job_receive");

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

    fep3::Participant partsender = fep3::base::createParticipant(
        "test_sender", FEP3_PARTICIPANT_LIBRARY_VERSION_STR, "system_name", sender_elem_factory);

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
        fep3::base::createParticipant("test_receiver",
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

    ASSERT_TRUE(executor_receiver.stop());
    ASSERT_TRUE(executor_sender.stop());

    EXPECT_EQ(status, std::future_status::ready) << "No signal was received from test_receiver";
    ASSERT_EQ(last_value, msg_to_send);
}
