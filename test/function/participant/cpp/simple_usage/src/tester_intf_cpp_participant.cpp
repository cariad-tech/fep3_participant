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

#include <gtest/gtest.h>

#include <fep3/cpp.h>
#include <fep3/core/participant_executor.hpp>
#include <chrono>

#include <fep3/fep3_participant_version.h>
#include <component_registry_rpc_service_helper.h>
#include <fep3/fep3_participant_version.h>
#include <fep3/rpc_services/component_registry/component_registry_rpc_intf_def.h>
#include <fep3/rpc_services/component_registry/component_registry_client_stub.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include "fep3/components/service_bus/service_bus_intf.h"

using namespace std::literals::chrono_literals;
using namespace fep3::cpp;


class MyJobSend : public DataJob
{
public:
    MyJobSend() : DataJob("myjob_send", 100ms)
    {
        my_out_data = addDataOut("my_data", fep3::base::StreamTypeString());
        registerPropertyVariable(_value_to_send, "value_to_send");
    }
    fep3::Result process(fep3::Timestamp time) override
    {
        updatePropertyVariables();
        std::string data_to_write = _value_to_send;
        data_to_write += std::to_string(time.count());
        *my_out_data << data_to_write;
        std::this_thread::sleep_for(1ms);
        return {};
    }
private:
    DataWriter* my_out_data;
    PropertyVariable<std::string> _value_to_send{ "send value at time:" };
};

class MyJobReceive : public DataJob
{
public:
    MyJobReceive() : DataJob("myjob_receive", 100ms)
    {
        _counter = 0;
        my_in_data = addDataIn("my_data", fep3::base::StreamTypeString());
    }
    fep3::Result process(fep3::Timestamp /*time*/) override
    {
        std::string value;
        *my_in_data >> value;

        _last_value = value;
        if (_last_value.length() > 0)
        {
            _counter++;
        }
        std::this_thread::sleep_for(1ms);
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

    static std::atomic<int32_t> _counter;
    static std::string _last_value;

private:
    DataReader* my_in_data;
};

std::atomic<int32_t> MyJobReceive::_counter;
std::string MyJobReceive::_last_value;

class TestClient : public fep3::rpc::RPCServiceClient<::fep3::rpc_stubs::RPCParticipantComponentRegistryClientStub, fep3::rpc::bronn::IRPCComponentRegistryDef>
{
private:
    using base_type = fep3::rpc::RPCServiceClient<::fep3::rpc_stubs::RPCParticipantComponentRegistryClientStub, fep3::rpc::bronn::IRPCComponentRegistryDef>;

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

    TestClient client(fep3::rpc::bronn::IRPCComponentRegistryDef::getRPCDefaultName(),
        _service_bus->getRequester("test_sender"));

    // test existing component
    auto json_value = client.getPluginVersion(_service_bus->getComponentIID());
    ASSERT_NO_FATAL_FAILURE(fep3::test::helper::validateJsonKey(json_value, "version", FEP3_PARTICIPANT_LIBRARY_VERSION_STR))
        << "Json return value not as expected";
    json_value = client.getFilePath(_service_bus->getComponentIID());
    ASSERT_NO_FATAL_FAILURE(fep3::test::helper::validateJsonKey(json_value, "file_path"))
        << "Json return value not as expected";
    json_value = client.getParticipantLibraryVersion(_service_bus->getComponentIID());
    // participant is in the form fep3_participant @ 3.0.1 build 0
    ASSERT_NO_FATAL_FAILURE(fep3::test::helper::keyContains(json_value, "participant_version", { FEP3_PARTICIPANT_LIBRARY_VERSION_STR , "fep3_participant"}))
        << "Json return value not as expected";


    // test non existing component
    json_value = client.getPluginVersion("nonExistingComponent");
    ASSERT_NO_FATAL_FAILURE(fep3::test::helper::validateJsorError(json_value)) << "Json return value not as expected";
}

/**
 * @detail Test the registration, unregistration and memorymanagment of the ComponentRegistry
 * @req_id FEPSDK-Sample
 */
TEST(CPPAPITester, testSimpleUse)
{
    using namespace fep3::cpp;
    Participant partsender = createParticipant<DataJobElement<MyJobSend>>("test_sender", "system_name");
    fep3::core::ParticipantExecutor executor_sender(partsender);

    Participant partreceiver = createParticipant<DataJobElement<MyJobReceive>>("test_receiver", "system_name");
    fep3::core::ParticipantExecutor executor_receiver(partreceiver);

    executor_sender.exec();//this will not block
    executor_receiver.exec();//this will not block

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

    int try_count = 20;
    while (MyJobReceive::_counter < 1 && try_count > 0)
    {
        std::this_thread::sleep_for(100ms);
        --try_count;
    }
    ASSERT_TRUE(executor_receiver.stop());
    ASSERT_TRUE(executor_sender.stop());

    EXPECT_GT(MyJobReceive::_counter, 0);
    ASSERT_TRUE(MyJobReceive::_last_value.find("send value at time:") != std::string::npos);
}
