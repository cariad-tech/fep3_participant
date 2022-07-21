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


#include <iostream>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common/gtest_asserts.h>
#include <common/properties_test_helper.h>

#pragma warning( push )
#pragma warning( disable : 4290)
#include <rpc/rpc.h>
#pragma warning( pop )

#include "test_health_proxy_stub.h"

#include <fep3/fep3_participant_version.h>
#include <fep3/components/base/component_registry.h>
#include <fep3/components/service_bus/rpc/fep_rpc.h>
#include <fep3/native_components/health/health_service.h>

#include <fep3/native_components/service_bus/service_bus.h>
#include <fep3/native_components/service_bus/testing/service_bus_testing.hpp>

#include <fep3/rpc_services/health/health_service_rpc_intf_def.h>

using namespace fep3;
using namespace ::testing;
using namespace a_util::strings;

class TestClient : public fep3::rpc::RPCServiceClient<::test::rpc_stubs::TestHealthServiceProxy, fep3::experimental::IRPCHealthServiceDef>
{
private:
    using base_type = fep3::rpc::RPCServiceClient<::test::rpc_stubs::TestHealthServiceProxy, fep3::experimental::IRPCHealthServiceDef> ;

public:
    using base_type::GetStub;

    TestClient(const char* server_object_name,
        std::shared_ptr<fep3::IRPCRequester> rpc) : base_type(server_object_name, rpc)
    {
    }
};

struct NativeHealthServiceRPC : public ::testing::Test
{
    NativeHealthServiceRPC() :
        _health_service{ std::make_shared<fep3::native::HealthService>() },
        _service_bus{ std::make_shared<fep3::native::ServiceBus>() },

        _component_registry { std::make_shared<fep3::ComponentRegistry>()}
    {}

    void SetUp() override
    {
        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(_service_bus, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::experimental::IHealthService>(_health_service, _dummy_component_version_info));

        ASSERT_FEP3_NOERROR(_component_registry->create());
    }

    std::shared_ptr<fep3::native::HealthService> _health_service{};
    std::shared_ptr<fep3::native::ServiceBus> _service_bus{};
    std::shared_ptr<fep3::ComponentRegistry> _component_registry{};
private:
    const fep3::ComponentVersionInfo _dummy_component_version_info{FEP3_PARTICIPANT_LIBRARY_VERSION_STR, "dummyPath", FEP3_PARTICIPANT_LIBRARY_VERSION_STR};
};

/**
 * @brief Test whether the health service rpc interface returns the correct participant health state via getHealth.
 */
TEST_F(NativeHealthServiceRPC, getHealthState)
{
    TestClient client(fep3::experimental::IRPCHealthServiceDef::getRPCDefaultName(),
                        _service_bus->getRequester(fep3::native::testing::participant_name_default));

    {
        EXPECT_EQ(static_cast<int>(experimental::HealthState::ok), client.getHealth());

        ASSERT_FEP3_NOERROR(_health_service->setHealthToError(""));
        EXPECT_EQ(static_cast<int>(experimental::HealthState::error), client.getHealth());
    }
}

/**
 * @brief Test whether the health service rpc interface correctly sets a participant health state to 'ok' via resetHealth.
 *
 */
TEST_F(NativeHealthServiceRPC, resetHealthState)
{
    TestClient client(fep3::experimental::IRPCHealthServiceDef::getRPCDefaultName(),
                        _service_bus->getRequester(fep3::native::testing::participant_name_default));

    {
        EXPECT_EQ(experimental::HealthState::ok, _health_service->getHealth());

        fep3::Result expected_rpc_result{ERR_NOERROR, "No error occurred", -1, "", ""};

        // resetting from state 'ok' to 'ok' succeeds
        auto received_result = ::rpc::cJSONConversions::json_to_result(client.resetHealth(""));
        EXPECT_EQ(expected_rpc_result.getErrorCode(), received_result.getErrorCode());
        EXPECT_STREQ(expected_rpc_result.getDescription(), received_result.getDescription());
        EXPECT_EQ(experimental::HealthState::ok, _health_service->getHealth());

        ASSERT_FEP3_NOERROR(_health_service->setHealthToError(""));
        EXPECT_EQ(experimental::HealthState::error, _health_service->getHealth());

        // resetting from state 'error' to 'ok' succeeds
        received_result = ::rpc::cJSONConversions::json_to_result(client.resetHealth(""));
        EXPECT_EQ(expected_rpc_result.getErrorCode(), received_result.getErrorCode());
        EXPECT_STREQ(expected_rpc_result.getDescription(), received_result.getDescription());

    }
}
