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

#include <fep3/components/service_bus/mock_service_bus.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/core.h>
#include <fep3/native_components/service_bus/testing/service_bus_testing.hpp>
#include <fep3/rpc_services/service_bus/http_server_rpc_intf_def.h>

#include <test_http_server_client_stub.h>

using namespace ::testing;

using ServiceBusComponentMock = StrictMock<fep3::mock::ServiceBus>;
using RPCServerMock = StrictMock<fep3::mock::RPCServer>;

namespace fep3 {
namespace test {
namespace env {

using namespace ::testing;

class TestClient : public rpc::RPCServiceClient<::test::rpc_stubs::TestHttpServerClientStub,
                                                rpc::IRPCHttpServerDef> {
private:
    typedef RPCServiceClient<TestHttpServerClientStub, rpc::IRPCHttpServerDef> base_type;

public:
    using base_type::GetStub;

    TestClient(const std::string& server_object_name,
               const std::shared_ptr<IRPCRequester>& rpc_requester)
        : base_type(server_object_name, rpc_requester)
    {
    }
};

class TestElement : public fep3::core::ElementBase {
public:
    TestElement() : fep3::core::ElementBase("", "")
    {
    }
};

struct NativeServiceBusRPC : public Test {
    NativeServiceBusRPC() : _service_bus(std::make_shared<fep3::native::ServiceBus>())
    {
    }

    void SetUp() override
    {
        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));
        _client = std::make_unique<TestClient>(
            rpc::IRPCHttpServerDef::getRPCDefaultName(),
            _service_bus->getRequester(fep3::native::testing::participant_name_default));
    }

    void TearDown() override
    {
    }

    std::shared_ptr<native::ServiceBus> _service_bus{};
    std::unique_ptr<TestClient> _client{};
};

TEST_F(NativeServiceBusRPC, testGetHeartbeatInterval)
{
    EXPECT_EQ(_client->getHeartbeatInterval()["interval_ms"], 5000);
}

TEST_F(NativeServiceBusRPC, testSetHeartbeatInterval)
{
    EXPECT_EQ(_client->setHeartbeatInterval(1000)["interval_ms"], 1000);
    EXPECT_EQ(_client->getHeartbeatInterval()["interval_ms"], 1000);
}

} // namespace env
} // namespace test
} // namespace fep3
