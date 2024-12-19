/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/core/participant_executor.hpp>
#include <fep3/cpp.h>
#include <fep3/rpc_services/component_registry/component_registry_client_stub.h>
#include <fep3/rpc_services/component_registry/component_registry_rpc_intf_def.h>

#include <component_registry_rpc_service_helper.h>

using namespace std::literals::chrono_literals;
using namespace fep3::cpp;

class MyJobSend : public fep3::cpp::DataJob {
public:
    MyJobSend(uint32_t send_signal_count)
        : fep3::cpp::DataJob("myjob_send", 100ms), _send_signal_count(send_signal_count)
    {
        my_out_data = addDataOut("my_data", fep3::base::StreamTypeString());
        registerPropertyVariable(_value_to_send, "value_to_send");
    }

    fep3::Result process(fep3::Timestamp time) override
    {
        if (_signals_sent <= _send_signal_count) {
            updatePropertyVariables();
            std::string data_to_write = _value_to_send;
            data_to_write += std::to_string(time.count());
            *my_out_data << data_to_write;
            ++_signals_sent;
        }
        return {};
    }

private:
    DataWriter* my_out_data;
    PropertyVariable<std::string> _value_to_send{"send value at time:"};
    uint32_t _send_signal_count;
    uint32_t _signals_sent{};
};

class MyJobReceive : public fep3::cpp::DataJob {
public:
    template <typename... Args>
    MyJobReceive(std::future<void>& signal_received,
                 std::string& last_value,
                 uint32_t signals_to_receive,
                 Args&&... args)
        : fep3::cpp::DataJob(std::forward<Args>(args)...),
          _last_value(last_value),
          _signals_to_receive(signals_to_receive)
    {
        my_in_data = addDataIn("my_data", fep3::base::StreamTypeString());
        signal_received = _signal_received.get_future();
    }
    fep3::Result process(fep3::Timestamp /*time*/) override
    {
        fep3::Optional<std::string> value;
        *my_in_data >> value;

        if (value.has_value()) {
            ++_signals_received;
            _last_value = value.value();
        }

        if (_signals_received == _signals_to_receive) {
            _signal_received = {};
        }
        return {};
    }

    fep3::Result reset() override
    {
        FEP3_LOG_INFO("info");
        FEP3_LOG_WARNING("warning");
        FEP3_LOG_ERROR("error");
        FEP3_LOG_DEBUG("debug");
        FEP3_LOG_FATAL("fatal");
        FEP3_LOG_RESULT(fep3::ERR_RETRY);
        return {};
    }

    std::promise<void> _signal_received;
    std::string& _last_value;
    uint32_t _signals_received{};
    uint32_t _signals_to_receive;

private:
    DataReader* my_in_data;
};

class TestClient : public fep3::rpc::RPCServiceClient<
                       ::fep3::rpc_stubs::RPCParticipantComponentRegistryClientStub,
                       fep3::rpc::IRPCComponentRegistryDef> {
private:
    using base_type =
        fep3::rpc::RPCServiceClient<::fep3::rpc_stubs::RPCParticipantComponentRegistryClientStub,
                                    fep3::rpc::IRPCComponentRegistryDef>;

public:
    using base_type::GetStub;

    TestClient(const std::string& server_object_name,
               const std::shared_ptr<fep3::arya::IRPCRequester>& rpc_requester)
        : base_type(server_object_name, rpc_requester)
    {
    }

protected:
};

void PerformComponentRegistryRpcTest(Participant& participant)
{
    auto _service_bus = participant.getComponent<fep3::IServiceBus>();
    ASSERT_NE(nullptr, _service_bus);

    TestClient client(fep3::rpc::IRPCComponentRegistryDef::getRPCDefaultName(),
                      _service_bus->getRequester("test_sender"));

    // test existing component
    auto json_value = client.getPluginVersion(_service_bus->getComponentIID());
    ASSERT_NO_FATAL_FAILURE(fep3::test::helper::validateJsonKey(
        json_value, "version", FEP3_PARTICIPANT_LIBRARY_VERSION_STR))
        << "Json return value not as expected";
    json_value = client.getFilePath(_service_bus->getComponentIID());
    ASSERT_NO_FATAL_FAILURE(fep3::test::helper::validateJsonKey(json_value, "file_path"))
        << "Json return value not as expected";
    json_value = client.getParticipantLibraryVersion(_service_bus->getComponentIID());
    // participant is in the form fep3_participant @ 3.0.1 build 0
    ASSERT_NO_FATAL_FAILURE(
        fep3::test::helper::keyContains(json_value,
                                        "participant_version",
                                        {FEP3_PARTICIPANT_LIBRARY_VERSION_STR, "fep3_participant"}))
        << "Json return value not as expected";

    // test non existing component
    json_value = client.getPluginVersion("nonExistingComponent");
    ASSERT_NO_FATAL_FAILURE(fep3::test::helper::validateJsorError(json_value))
        << "Json return value not as expected";
}

template <typename JobType>
struct JobConstructorArguments {
    virtual ~JobConstructorArguments() = default;
    virtual std::shared_ptr<JobType> create_job_from_args() const = 0;
};

template <typename JobType, typename... Args>
struct JobConstructorArgumentsImpl : public JobConstructorArguments<JobType> {
    JobConstructorArgumentsImpl(Args&&... args)
        : _args(std::make_tuple(std::forward<Args>(args)...))
    {
    }

    JobConstructorArgumentsImpl(const JobConstructorArgumentsImpl&) = delete;

    std::shared_ptr<JobType> create_job_from_args() const override
    {
        return std::apply(
            [&](auto&&... args) {
                return std::make_shared<JobType>(std::forward<decltype(args)>(args)...);
            },
            _args);
    }

    std::tuple<Args...> _args;
};

template <typename JobType>
class CustomElementFactory : public fep3::base::IElementFactory {
public:
    template <typename... Args>
    CustomElementFactory(Args&&... args)
        : _job_args(std::make_unique<JobConstructorArgumentsImpl<JobType, Args...>>(
              std::forward<Args>(args)...))
    {
    }

    // CustomElementFactory(const CustomElementFactory&) {}

    std::unique_ptr<fep3::base::IElement> createElement(const fep3::IComponents&) const override
    {
        return std::make_unique<DataJobElement<JobType>>(_job_args->create_job_from_args());
    }

private:
    std::unique_ptr<JobConstructorArguments<JobType>> _job_args;
};

/**
 * @detail Test the registration, unregistration and memorymanagment of the ComponentRegistry
 * @req_id FEPSDK-Sample
 */
TEST(CPPAPITester, testSimpleUse)
{
    using namespace fep3::cpp;

    const uint32_t send_signal_count{1};
    auto sender_elem_factory = std::make_shared<CustomElementFactory<MyJobSend>>(send_signal_count);

    Participant partsender = fep3::base::createParticipant(
        "test_sender", FEP3_PARTICIPANT_LIBRARY_VERSION_STR, "system_name", sender_elem_factory);

    fep3::core::ParticipantExecutor executor_sender(partsender);

    std::future<void> receive_future;
    std::string last_value;
    const uint32_t expected_rec_signal_count{1};

    auto receiver_elem_factory = std::make_shared<CustomElementFactory<MyJobReceive>>(
        std::ref(receive_future),
        std::ref(last_value),
        std::ref(expected_rec_signal_count),
        std::string("myjob_receive"),
        fep3::ClockTriggeredJobConfiguration(100ms));

    Participant partreceiver = fep3::base::createParticipant("test_receiver",
                                                             FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
                                                             "system_name",
                                                             receiver_elem_factory);

    fep3::core::ParticipantExecutor executor_receiver(partreceiver);

    executor_sender.exec();   // this will not block
    executor_receiver.exec(); // this will not block

    ASSERT_TRUE(executor_sender.load());
    ASSERT_TRUE(executor_receiver.load());

    ASSERT_TRUE(executor_sender.initialize());
    ASSERT_TRUE(executor_receiver.initialize());

    // check behavior on reinitialization
    ASSERT_TRUE(executor_receiver.deinitialize());
    ASSERT_TRUE(executor_receiver.initialize());

    ASSERT_TRUE(executor_receiver.start());
    ASSERT_TRUE(executor_sender.start());

    std::future_status status = receive_future.wait_for(std::chrono::seconds(2));

    ASSERT_TRUE(executor_receiver.stop());
    ASSERT_TRUE(executor_sender.stop());

    EXPECT_EQ(status, std::future_status::ready) << "No signal was received from test_receiver";
    ASSERT_TRUE(last_value.find("send value at time:") != std::string::npos);
}

TEST(CPPAPITester, testDataTriggeredUseCase)
{
    using namespace fep3::cpp;

    const uint32_t send_signal_count{3};
    auto sender_elem_factory = std::make_shared<CustomElementFactory<MyJobSend>>(send_signal_count);

    Participant partsender = fep3::base::createParticipant(
        "test_sender", FEP3_PARTICIPANT_LIBRARY_VERSION_STR, "system_name", sender_elem_factory);

    fep3::core::ParticipantExecutor executor_sender(partsender);

    std::future<void> receive_future;
    std::string last_value;
    const uint32_t expected_rec_signal_count{3};

    auto receiver_elem_factory = std::make_shared<CustomElementFactory<MyJobReceive>>(
        std::ref(receive_future),
        std::ref(last_value),
        std::ref(expected_rec_signal_count),
        std::string("myjob_receive"),
        fep3::DataTriggeredJobConfiguration({"my_data"}));

    Participant partreceiver = fep3::base::createParticipant("test_receiver",
                                                             FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
                                                             "system_name",
                                                             receiver_elem_factory);

    fep3::core::ParticipantExecutor executor_receiver(partreceiver);

    executor_sender.exec();   // this will not block
    executor_receiver.exec(); // this will not block

    ASSERT_TRUE(executor_sender.load());
    ASSERT_TRUE(executor_receiver.load());

    ASSERT_TRUE(executor_sender.initialize());
    ASSERT_TRUE(executor_receiver.initialize());

    // check behavior on reinitialization
    ASSERT_TRUE(executor_receiver.deinitialize());
    ASSERT_TRUE(executor_receiver.initialize());

    ASSERT_TRUE(executor_receiver.start());
    ASSERT_TRUE(executor_sender.start());

    ASSERT_NO_FATAL_FAILURE(PerformComponentRegistryRpcTest(partsender));

    std::future_status status = receive_future.wait_for(std::chrono::seconds(15));

    ASSERT_TRUE(executor_receiver.stop());
    ASSERT_TRUE(executor_sender.stop());

    EXPECT_EQ(status, std::future_status::ready)
        << "Datat triggered test_receiver did not receive ";
    ASSERT_TRUE(last_value.find("send value at time:") != std::string::npos);
}
