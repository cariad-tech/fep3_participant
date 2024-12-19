/**
 * @copyright
 * @verbatim
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @endverbatim
 */
#include "tester_service_bus_native_and_base_mocks.h"

#include <common/gtest_asserts.h>
#include <http_systemaccess.h>

using namespace ::testing;
using namespace fep3;

struct TestSystemAccess : public ::testing::Test {
    TestSystemAccess()
        : _service_discovery_factory(std::make_shared<NiceMock<ServiceDiscoveryFactoryMock>>()),
          _service_bus_defaults(std::make_shared<NiceMock<SystemAccessDefaultUrlsMock>>()),
          _service_finder_mock(std::make_unique<testing::NiceMock<ServiceFinderMock>>()),
          _service_finder(_service_finder_mock.get())
    {
    }

protected:
    void SetUp() override
    {
        ON_CALL(*_service_bus_defaults, getDefaultServerUrl())
            .WillByDefault(Return("use_default_url"));

        ON_CALL(*_service_finder, sendMSearch()).WillByDefault(Return(true));
        ON_CALL(*_service_finder, checkForServices(_, _)).WillByDefault(Return(true));
    }

    std::shared_ptr<ServiceDiscoveryFactoryMock> _service_discovery_factory;
    std::shared_ptr<SystemAccessDefaultUrlsMock> _service_bus_defaults;
    std::unique_ptr<ServiceFinderMock> _service_finder_mock;
    ServiceFinderMock* _service_finder;
};

// Valid system url and  discovery_active = true make system discoverable and able to discover
TEST_F(TestSystemAccess, ValidSysUrl_DicoveryActive)
{
    const std::string valid_sys_url = "url";
    const bool discoverable_flag = true;

    EXPECT_CALL(*_service_discovery_factory, getServiceFinder(_, _, _, _, _, _))
        .WillOnce(Return(ByMove(std::move(_service_finder_mock))));
    EXPECT_CALL(*_service_discovery_factory, getServiceDiscovery(_, _, _, _, _, _, _, _, _, _))
        .WillOnce(Return(ByMove(nullptr)));

    auto sys_access = std::make_unique<fep3::native::HttpSystemAccess>(
        "sys", valid_sys_url, _service_bus_defaults, nullptr, _service_discovery_factory);
    sys_access->createServer("serv_name", "use_default_url", discoverable_flag);
}

// Empty system url but discovery_active = true throws exception
TEST_F(TestSystemAccess, EmptySysUrl_DicoveryActive)
{
    const std::string empty_sys_url = "";
    const bool discoverable_flag = true;

    auto sys_access = std::make_unique<fep3::native::HttpSystemAccess>(
        "sys", empty_sys_url, _service_bus_defaults, nullptr, _service_discovery_factory);
    ASSERT_FEP3_RESULT(sys_access->createServer("serv_name", "use_default_url", discoverable_flag),
                       fep3::Result(ERR_BAD_DEVICE));
}

// Valid system url and  discovery_active = false make system not discoverable and able to discover
TEST_F(TestSystemAccess, ValidSysUrl_DicoveryNotActive)
{
    const std::string valid_sys_url = "url";
    const bool discoverable_flag = false;

    EXPECT_CALL(*_service_discovery_factory, getServiceFinder(_, _, _, _, _, _))
        .WillOnce(Return(ByMove(std::move(_service_finder_mock))));

    auto sys_access = std::make_unique<fep3::native::HttpSystemAccess>(
        "sys", valid_sys_url, _service_bus_defaults, nullptr, _service_discovery_factory);
    sys_access->createServer("serv_name", "use_default_url", discoverable_flag);
}

// Empty system url and  discovery_active = false make system not discoverable and not able to
// discover
TEST_F(TestSystemAccess, EmptySysUrl_DicoveryNotActive)
{
    const std::string empty_sys_url = "";
    const bool discoverable_flag = false;

    auto sys_access = std::make_unique<fep3::native::HttpSystemAccess>(
        "sys", empty_sys_url, _service_bus_defaults, nullptr, _service_discovery_factory);
    sys_access->createServer("serv_name", "use_default_url", discoverable_flag);
}

// Old behavior, Valid system url make system discoverable and able to discover
TEST_F(TestSystemAccess, Old_behavior_ValidSysUrl)
{
    const std::string valid_sys_url = "url";

    EXPECT_CALL(*_service_discovery_factory, getServiceFinder(_, _, _, _, _, _))
        .WillOnce(Return(ByMove(std::move(_service_finder_mock))));
    EXPECT_CALL(*_service_discovery_factory, getServiceDiscovery(_, _, _, _, _, _, _, _, _, _))
        .WillOnce(Return(ByMove(nullptr)));

    auto sys_access = std::make_unique<fep3::native::HttpSystemAccess>(
        "sys", valid_sys_url, _service_bus_defaults, nullptr, _service_discovery_factory);
    sys_access->createServer("serv_name", "use_default_url");
}

// Old behavior,  empty system url  system not discoverable and not able to discover
TEST_F(TestSystemAccess, Old_behavior_EmptySysUrl)
{
    const std::string empty_sys_url = "";

    auto sys_access = std::make_unique<fep3::native::HttpSystemAccess>(
        "sys", empty_sys_url, _service_bus_defaults, nullptr, _service_discovery_factory);
    sys_access->createServer("serv_name", "use_default_url");
}
