/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2022 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include <fep3/components/clock/mock_clock_service.h>
#include <fep3/native_components/clock/clock_service.h>
#include <fep3/native_components/clock/rpc_clock_service.h>

#include <gtest/gtest.h>

namespace fep3 {
namespace test {
namespace env {

using namespace ::testing;

using ClockServiceMock = NiceMock<mock::ClockService>;
using ClockType = fep3::arya::IClock::ClockType;

struct RPCClockServiceTest : public ::testing::Test {
    RPCClockServiceTest()
        : _clock_service_mock(std::make_shared<ClockServiceMock>()),
          _rpc_clock_service(std::make_shared<native::RPCClockService>(*_clock_service_mock))
    {
    }

    std::shared_ptr<ClockServiceMock> _clock_service_mock;
    std::shared_ptr<native::RPCClockService> _rpc_clock_service;
};

TEST_F(RPCClockServiceTest, getClockNames__successful)
{
    EXPECT_CALL(*_clock_service_mock, getClockNames())
        .WillOnce(Return(std::list<std::string>{"clock_name_1", "clock_name_2"}));

    ASSERT_EQ(_rpc_clock_service->getClockNames(), "clock_name_1,clock_name_2");
}

TEST_F(RPCClockServiceTest, getMainClocName__successful)
{
    std::string main_clock = "main_clock";
    EXPECT_CALL(*_clock_service_mock, getMainClockName).Times(1).WillOnce(Return(main_clock));
    ASSERT_EQ(_rpc_clock_service->getMainClockName(), main_clock);
}

TEST_F(RPCClockServiceTest, getTime__successfulWithEmptyName)
{
    Timestamp time = Timestamp{100};
    EXPECT_CALL(*_clock_service_mock, getTime()).Times(1).WillOnce(Return(time));

    ASSERT_EQ(_rpc_clock_service->getTime(""), std::to_string(time.count()));
}

TEST_F(RPCClockServiceTest, getTime__successfulWithValidName)
{
    Timestamp time = Timestamp{100};

    EXPECT_CALL(*_clock_service_mock, getTime(_)).Times(1).WillOnce(Return(time));

    ASSERT_EQ(_rpc_clock_service->getTime("valid_name"), std::to_string(time.count()));
}

TEST_F(RPCClockServiceTest, getTime__failWithInvalidName)
{
    EXPECT_CALL(*_clock_service_mock, getTime(_)).Times(1).WillOnce(Return(Optional<Timestamp>{}));

    ASSERT_EQ(_rpc_clock_service->getTime("valid_name"), "-1");
}

TEST_F(RPCClockServiceTest, getType__successfulWithEmptyName)
{
    EXPECT_CALL(*_clock_service_mock, getType())
        .Times(2)
        .WillOnce(Return(ClockType::continuous))
        .WillOnce(Return(ClockType::discrete));

    ASSERT_EQ(_rpc_clock_service->getType(""), 0);
    ASSERT_EQ(_rpc_clock_service->getType(""), 1);
}

TEST_F(RPCClockServiceTest, getType__successfulWithValidName)
{
    EXPECT_CALL(*_clock_service_mock, getType(_)).Times(1).WillOnce(Return(ClockType::continuous));

    ASSERT_EQ(_rpc_clock_service->getType("valid_name"), 0);
}

TEST_F(RPCClockServiceTest, getType_failWithInvalidName)
{
    EXPECT_CALL(*_clock_service_mock, getType(_)).Times(1).WillOnce(Return(Optional<ClockType>{}));

    ASSERT_EQ(_rpc_clock_service->getType("non_existent_clock"), -1);
}

} // namespace env
} // namespace test
} // namespace fep3
