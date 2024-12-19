/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/native_components/clock_sync/system_clock_client_reset.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

struct Reset_callable {
    MOCK_METHOD(void, reset, (fep3::arya::Timestamp), (const));
};

struct TestSystemClockClientReset : public testing::Test {
    const Reset_callable _callable;

    fep3::native::SystemClockClientReset::ResetFunction _reset_function =
        [&](fep3::Timestamp time) { _callable.reset(time); };

    fep3::native::SystemClockClientReset _client_reset;
    const fep3::Timestamp _reset_time{100};

    std::shared_ptr<fep3::ILogger> _logger =
        std::make_shared<testing::NiceMock<fep3::mock::LoggerWithDefaultBehavior>>();
};

/**
 * @brief: Tests the happy path
 * @detail When the start is called and then reset,
 * the reset callable is called.
 */
TEST_F(TestSystemClockClientReset, startThenReset__resetCalled)
{
    {
        EXPECT_CALL(_callable, reset(_reset_time)).Times(0);
        _client_reset.start(_reset_function, _logger.get());
    }
    {
        EXPECT_CALL(_callable, reset(_reset_time)).Times(1);
        _client_reset.reset(_reset_time, _reset_function);
    }
}

/**
 * @brief: Tests the not desired path
 * @detail When the reset is called and then start, the reset time
 * is cached and reset callable is called inside the start function.
 */
TEST_F(TestSystemClockClientReset, resetThenStart__resetCalledBeforeStart)
{
    {
        EXPECT_CALL(_callable, reset(_reset_time)).Times(0);
        _client_reset.reset(_reset_time, _reset_function);
    }
    {
        EXPECT_CALL(_callable, reset(_reset_time)).Times(1);
        _client_reset.start(_reset_function, _logger.get());
    }
}

/**
 * @brief: Tests the not desired path
 * @detail When functions are called in this order: start, stop, reset,
 * the reset callable is not called.
 */
TEST_F(TestSystemClockClientReset, stopThenReset__noResetCalled)
{
    using testing::_;

    EXPECT_CALL(_callable, reset(_)).Times(0);

    _client_reset.start(_reset_function, _logger.get());
    _client_reset.stop();
    _client_reset.reset(_reset_time, _reset_function);
}

/**
 * @brief: Tests the happy path after stop.
 */
TEST_F(TestSystemClockClientReset, startThenResetThenStop__resetCalledAndRunTwice)
{
    {
        EXPECT_CALL(_callable, reset(_reset_time)).Times(0);
        _client_reset.start(_reset_function, _logger.get());
    }
    {
        EXPECT_CALL(_callable, reset(_reset_time)).Times(1);
        _client_reset.reset(_reset_time, _reset_function);
    }

    _client_reset.stop();

    {
        EXPECT_CALL(_callable, reset(_reset_time)).Times(0);
        _client_reset.start(_reset_function, _logger.get());
    }
    {
        EXPECT_CALL(_callable, reset(_reset_time)).Times(1);
        _client_reset.reset(_reset_time, _reset_function);
    }
}

/**
 * @brief: Tests the undesired path after stop.
 */
TEST_F(TestSystemClockClientReset, resetThenStartThenStop__resetCalledBeforeStartRunTwice)
{
    {
        EXPECT_CALL(_callable, reset(_reset_time)).Times(0);
        _client_reset.reset(_reset_time, _reset_function);
    }

    {
        EXPECT_CALL(_callable, reset(_reset_time)).Times(1);
        _client_reset.start(_reset_function, _logger.get());
    }

    _client_reset.stop();

    {
        EXPECT_CALL(_callable, reset(_reset_time)).Times(0);
        _client_reset.reset(_reset_time, _reset_function);
    }

    {
        EXPECT_CALL(_callable, reset(_reset_time)).Times(1);
        _client_reset.start(_reset_function, _logger.get());
    }
}
