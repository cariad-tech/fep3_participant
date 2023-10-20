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

#include <helper/gmock_async_helper.h>
#include <testenvs/clock_service_envs.h>

using namespace ::testing;
using namespace fep3::native;

using namespace fep3::experimental;
using namespace std::literals;
using namespace fep3::test::env;
using namespace test::helper;

using AryaEventSinkMock = StrictMock<fep3::mock::arya::Clock::EventSink>;
using AryaClock = NiceMock<fep3::mock::arya::Clock>;
/**
 * @detail Test whether the clock service provides the native clocks by setting them as main clock
 * * LocalSystemRealClock
 * * LocalSystemSimTime
 * @req_id FEPSDK-2108, FEPSDK-2109
 */
TEST_F(NativeClockService, CTOR_successOnNativeClockAvailability)
{
    auto configuration_service = _component_registry->getComponent<fep3::IConfigurationService>();
    ASSERT_NE(configuration_service, nullptr);

    // actual test case
    {
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue(*configuration_service,
                                                         FEP3_CLOCK_SERVICE_MAIN_CLOCK,
                                                         FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME));
        ASSERT_FEP3_NOERROR(_component_registry->initialize());
        ASSERT_FEP3_NOERROR(_component_registry->tense());
        ASSERT_FEP3_NOERROR(_component_registry->relax());
        ASSERT_FEP3_NOERROR(_component_registry->deinitialize());

        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue(*configuration_service,
                                                         FEP3_CLOCK_SERVICE_MAIN_CLOCK,
                                                         FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME));
        ASSERT_FEP3_NOERROR(_component_registry->initialize());
        ASSERT_FEP3_NOERROR(_component_registry->tense());
        ASSERT_FEP3_NOERROR(_component_registry->relax());
        ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    }
}

/**
 * @detail Test whether the clock service default configuration is correct after creation.
 * This requires the following properties to be set:
 * * FEP3_CLOCK_SERVICE_MAIN_CLOCK = FEP3_CLOCK_SERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME
 * * FEP3_CLOCK_SERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR =
 * FEP3_CLOCK_SERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR_DEFAULT_VALUE
 * * FEP3_CLOCK_SERVICE_MAIN_CLOCK_SIM_TIME_STEP_SIZE =
 * FEP3_CLOCK_SERVICE_MAIN_CLOCK_SIM_TIME_STEP_SIZE_DEFAULT_VALUE
 * @req_id FEPSDK-2429, FEPSDK-2443
 */
TEST_F(NativeClockService, CTOR_successOnDefaultConfiguration)
{
    const std::string local_system_real_time_name = FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME;
    const std::string time_factor_default_value =
        std::to_string(FEP3_CLOCK_SIM_TIME_TIME_FACTOR_DEFAULT_VALUE);
    const std::string step_size_default_value =
        std::to_string(FEP3_CLOCK_SIM_TIME_STEP_SIZE_DEFAULT_VALUE);

    // actual test case
    {
        ASSERT_EQ(fep3::base::getPropertyValue<std::string>(
                      *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY)),
                  local_system_real_time_name);
        ASSERT_EQ(fep3::base::getPropertyValue<std::string>(*_clock_service_property_node->getChild(
                      FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY)),
                  time_factor_default_value);
        ASSERT_EQ(fep3::base::getPropertyValue<std::string>(*_clock_service_property_node->getChild(
                      FEP3_CLOCK_SIM_TIME_STEP_SIZE_PROPERTY)),
                  step_size_default_value);
    }
}

/**
 * @detail Test whether the clock service returns appropriate errors and log messages if the native
 * discrete clock is misconfigured regarding its step size property.
 */
TEST_F(NativeClockService, tense__failNativeDiscreteClockMisconfigurationStepSize)
{
    EXPECT_CALL(*_logger, logWarning(_)).WillRepeatedly(Return(fep3::Result{}));

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY),
        FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME));

    // Step size is too large and therefore invalid
    {
        constexpr int64_t step_size_invalid_too_large = std::numeric_limits<int64_t>::max();

        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<int64_t>(
            *_clock_service_property_node->getChild(FEP3_CLOCK_SIM_TIME_STEP_SIZE_PROPERTY),
            step_size_invalid_too_large));

        EXPECT_CALL(*_logger,
                    logError(fep3::mock::LogStringRegexMatcher(
                        std::string() + ".*9223372036854775808.000000.* >= " +
                        std::to_string(FEP3_CLOCK_SIM_TIME_STEP_SIZE_MIN_VALUE) + " and <= " +
                        std::to_string(FEP3_CLOCK_SIM_TIME_STEP_SIZE_MAX_VALUE) + ".*")))
            .WillOnce(Return(fep3::Result{}));

        ASSERT_FEP3_RESULT(_component_registry->tense(), fep3::ERR_INVALID_ARG);
    }

    ASSERT_FEP3_NOERROR(_component_registry->relax());

    // Step size is too small and therefore invalid
    {
        constexpr int64_t step_size_invalid_too_small = 0;

        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<int64_t>(
            *_clock_service_property_node->getChild(FEP3_CLOCK_SIM_TIME_STEP_SIZE_PROPERTY),
            step_size_invalid_too_small));

        EXPECT_CALL(*_logger,
                    logError(fep3::mock::LogStringRegexMatcher(
                        std::string() + ".*0.000000.*" +
                        std::to_string(FEP3_CLOCK_SIM_TIME_STEP_SIZE_MIN_VALUE) + " and <= " +
                        std::to_string(FEP3_CLOCK_SIM_TIME_STEP_SIZE_MAX_VALUE) + ".*")))
            .WillOnce(Return(fep3::Result{}));

        ASSERT_FEP3_RESULT(_component_registry->tense(), fep3::ERR_INVALID_ARG);
    }
}

/**
 * @detail Test whether the clock service returns appropriate errors and log messages if the native
 * discrete clock is misconfigured regarding its time factor property.
 */
TEST_F(NativeClockService, tense__failsOnNativeDiscreteClockMisconfigurationTimeFactor)
{
    EXPECT_CALL(*_logger, logWarning(_)).WillRepeatedly(Return(fep3::Result{}));

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY),
        FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME));

    // Time factor is too small and therefore invalid
    {
        constexpr double time_factor_invalid_too_small = -1;

        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<double>(
            *_clock_service_property_node->getChild(FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY),
            time_factor_invalid_too_small));

        EXPECT_CALL(*_logger,
                    logError(fep3::mock::LogStringRegexMatcher(
                        std::string() + ".*-1.000000.*" +
                        std::to_string(FEP3_CLOCK_SIM_TIME_TIME_FACTOR_AFAP_VALUE) + ".*")))
            .WillOnce(Return(fep3::Result{}));

        ASSERT_FEP3_RESULT(_component_registry->tense(), fep3::ERR_INVALID_ARG);
    }
}

/**
 * @detail Test whether the clock service returns appropriate errors and log messages if the native
 * discrete clock is misconfigured regarding its step size and time factor properties which results
 * in the wall clock step size being invalid.
 */
TEST_F(NativeClockService, tense__failOnNativeDiscreteClockMisconfigurationWallClockStepSize)
{
    EXPECT_CALL(*_logger, logWarning(_)).WillRepeatedly(Return(fep3::Result{}));

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY),
        FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME));

    constexpr int64_t step_size_max = FEP3_CLOCK_SIM_TIME_STEP_SIZE_MAX_VALUE;
    constexpr int64_t step_size_min = FEP3_CLOCK_SIM_TIME_STEP_SIZE_MIN_VALUE;

    // Step size is the max value and time factor is < 1 which results in the wall clock step size
    // being too large and therefore invalid
    {
        constexpr double time_factor_small = 0.5;

        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<int64_t>(
            *_clock_service_property_node->getChild(FEP3_CLOCK_SIM_TIME_STEP_SIZE_PROPERTY),
            step_size_max));
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<double>(
            *_clock_service_property_node->getChild(FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY),
            time_factor_small));

        EXPECT_CALL(*_logger,
                    logError(fep3::mock::LogStringRegexMatcher(
                        std::string() + ".*" + std::to_string(step_size_max / time_factor_small) +
                        ".*" + std::to_string(static_cast<double>(step_size_max)) + ".*" +
                        std::to_string(time_factor_small) + ".*" + std::to_string(step_size_min) +
                        ".*" + std::to_string(step_size_max) + ".*")))
            .WillOnce(Return(fep3::Result{}));

        ASSERT_FEP3_RESULT(_component_registry->tense(), fep3::ERR_INVALID_ARG);
    }

    ASSERT_FEP3_NOERROR(_component_registry->relax());

    // Step size is the min value and time factor is > 1 which results in the wall clock step size
    // being too small and therefore invalid
    {
        constexpr double time_factor_large = 2;

        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<int64_t>(
            *_clock_service_property_node->getChild(FEP3_CLOCK_SIM_TIME_STEP_SIZE_PROPERTY),
            step_size_min));
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<double>(
            *_clock_service_property_node->getChild(FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY),
            time_factor_large));

        EXPECT_CALL(*_logger,
                    logError(fep3::mock::LogStringRegexMatcher(
                        std::string() + ".*" + std::to_string(step_size_min / time_factor_large) +
                        ".*" + std::to_string(static_cast<double>(step_size_min)) + ".*" +
                        std::to_string(time_factor_large) + ".*" + std::to_string(step_size_min) +
                        ".*" + std::to_string(step_size_max) + ".*")))
            .WillOnce(Return(fep3::Result{}));

        ASSERT_FEP3_RESULT(_component_registry->tense(), fep3::ERR_INVALID_ARG);
    }
}

/**
 * @detail Test whether the main clock of the clock service may be configured by setting the
 * corresponding property. Both native clock implementations FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME and
 * FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME and a custom clock are configured to be main clock in this test.
 * @req_id FEPSDK-2116, FEPSDK-2138, FEPSDK-2443
 */
TEST_F(NativeClockServiceWithClockMocks, registerClock__successOnConfigureMainClock)
{
    // actual test case
    {
        EXPECT_CALL(*_logger, isDebugEnabled()).WillRepeatedly(Return(true));
        EXPECT_CALL(*_logger,
                    logDebug(fep3::mock::LogStringRegexMatcher(std::string() + "Clock '" +
                                                               FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME +
                                                               "' set as main clock")))
            .WillOnce(Return(fep3::Result{}));

        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
            *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY),
            FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME));
        ASSERT_EQ(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME, _clock_service_intf->getMainClockName());
        ASSERT_FEP3_NOERROR(_component_registry->tense());
        ASSERT_EQ(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME, _clock_service_intf->getMainClockName());
        ASSERT_FEP3_NOERROR(_component_registry->relax());

        EXPECT_CALL(*_logger,
                    logDebug(fep3::mock::LogStringRegexMatcher(std::string() + "Clock '" +
                                                               FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME +
                                                               "' set as main clock")))
            .WillOnce(Return(fep3::Result{}));

        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
            *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY),
            FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME));
        ASSERT_EQ(FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME, _clock_service_intf->getMainClockName());
        ASSERT_FEP3_NOERROR(_component_registry->tense());
        ASSERT_EQ(FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME, _clock_service_intf->getMainClockName());
        ASSERT_FEP3_NOERROR(_component_registry->relax());

        EXPECT_CALL(*_logger, logDebug(_)).WillRepeatedly(Return(fep3::Result{}));
        EXPECT_CALL(
            *_logger,
            logDebug(fep3::mock::LogStringRegexMatcher(
                std::string() + "Clock '" + _clock_mock->getName() + "' set as main clock")))
            .WillOnce(Return(fep3::Result{}));

        ASSERT_FEP3_NOERROR(_clock_service_intf->registerClock(_clock_mock));
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
            *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY),
            _clock_mock->getName()));
        ASSERT_EQ(_clock_mock->getName(), _clock_service_intf->getMainClockName());
        ASSERT_FEP3_NOERROR(_component_registry->tense());
        ASSERT_EQ(_clock_mock->getName(), _clock_service_intf->getMainClockName());
        ASSERT_FEP3_NOERROR(_component_registry->relax());
    }
}

/**
 * @detail Test whether clock service supports usage of multiple clocks.
 * Usage involves registration, deregistration, retrieving time from non active clocks,
 * switching between active clocks and retrieving information from registered clocks.
 * @req_id FEPSDK-2118
 */
TEST_F(NativeClockServiceWithClockMocks, registerClock__successOnSupportMultipleClocks)
{
    EXPECT_CALL(*_clock_mock, getTime()).Times(1).WillOnce(Return(fep3::Timestamp{0}));
    EXPECT_CALL(*_clock_mock_2, getTime()).Times(1).WillOnce(Return(fep3::Timestamp{0}));

    ASSERT_FEP3_NOERROR(_clock_service_intf->registerClock(_clock_mock));
    ASSERT_FEP3_NOERROR(_clock_service_intf->registerClock(_clock_mock_2));

    ASSERT_EQ(fep3::Timestamp{0},
              _clock_service_intf->getTime(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME).value());
    ASSERT_EQ(fep3::Timestamp{0},
              _clock_service_intf->getTime(FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME).value());
    ASSERT_EQ(fep3::Timestamp{0}, _clock_service_intf->getTime(_clock_mock->getName()).value());
    ASSERT_EQ(fep3::Timestamp{0}, _clock_service_intf->getTime(_clock_mock_2->getName()).value());

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY), _clock_mock->getName()));
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_EQ(_clock_mock->getName(), _clock_service_intf->getMainClockName());
    ASSERT_FEP3_NOERROR(_component_registry->relax());

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY),
        _clock_mock_2->getName()));
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_EQ(_clock_mock_2->getName(), _clock_service_intf->getMainClockName());
    ASSERT_FEP3_NOERROR(_component_registry->relax());

    const auto clock_list = std::list<std::string>{FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME,
                                                   FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME,
                                                   _clock_mock->getName(),
                                                   _clock_mock_2->getName()};
    ASSERT_EQ(_clock_service_intf->getClockNames().size(), 4);
    ASSERT_EQ(_clock_service_intf->getClockNames(), clock_list);

    EXPECT_CALL((*_logger), logWarning(_)).Times(1).WillOnce(Return(fep3::Result{}));

    ASSERT_FEP3_NOERROR(_clock_service_intf->unregisterClock(_clock_mock->getName()));
    ASSERT_FEP3_NOERROR(_clock_service_intf->unregisterClock(_clock_mock_2->getName()));
}

/**
 * @detail Test whether the clock service provides logs the main clock name as debug log on start.
 */
TEST_F(NativeClockService, start__successOnLogMainClockName)
{
    EXPECT_CALL(*_logger, isDebugEnabled()).WillRepeatedly(Return(true));
    EXPECT_CALL(*_logger, logDebug(_)).WillRepeatedly(Return(fep3::Result{}));
    EXPECT_CALL(*_logger,
                logDebug(fep3::mock::LogStringRegexMatcher(std::string() + "Clock '" +
                                                           FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME +
                                                           "' is configured as main clock")))
        .WillOnce(Return(fep3::Result{}));

    ASSERT_FEP3_NOERROR(_component_registry->start());

    ASSERT_FEP3_NOERROR(_component_registry->stop());
}

/**
 * @detail Test whether the clock service provides a steadily rising time on getTime() calls after
 * start. This test uses the native continuous clock.
 * @req_id FEPSDK-2105, FEPSDK-2106, FEPSDK-2108
 */
TEST_F(NativeClockService, getTime__testGetTimeMainClockContinuous)
{
    fep3::Timestamp current_time{0};
    fep3::Timestamp last_time{0};
    auto testidx = 0;

    ASSERT_FEP3_NOERROR(_component_registry->start());

    // actual test case
    {
        while (testidx < 10) {
            std::this_thread::sleep_for(10us);
            current_time = _clock_service_intf->getTime();
            ASSERT_GT(current_time, last_time);
            last_time = current_time;
            testidx++;
        }
    }

    ASSERT_FEP3_NOERROR(_component_registry->stop());
}

/**
 * @detail Test whether the clock service provides a steadily rising time on getTime() calls after
 * start. This test uses the native discrete clock.
 * @req_id FEPSDK-2105, FEPSDK-2107, FEPSDK-2109
 */
TEST_F(NativeClockService, getTime__successOnGetTimeMainClockDiscrete)
{
    fep3::Timestamp current_time{0};
    fep3::Timestamp last_time{-1};
    auto testidx = 0;

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue(
        *(_component_registry->getComponent<fep3::IConfigurationService>()),
        FEP3_CLOCK_SERVICE_MAIN_CLOCK,
        FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME));

    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    bool at_least_one_times_within_the_loop = false;
    // actual test case
    {
        while (testidx < 10) {
            std::this_thread::sleep_for(50ms);
            current_time = _clock_service_intf->getTime();
            EXPECT_GE(current_time, last_time);
            if (current_time < last_time) {
                // error !!
                break;
            }
            else if (current_time > last_time && last_time != fep3::Timestamp{-1}) {
                at_least_one_times_within_the_loop = true;
                EXPECT_EQ(current_time, last_time + 100ms);
            }
            last_time = current_time;
            testidx++;
        }
    }

    EXPECT_TRUE(at_least_one_times_within_the_loop);

    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
}

/**
 * @detail Test whether the clock service provides a time when calling getTime for specific clocks.
 * @req_id FEPSDK-2430
 */
TEST_F(NativeClockServiceWithClockMocks, getTime__successOnGetTimeByName)
{
    ASSERT_FEP3_NOERROR(_clock_service_intf->registerClock(_clock_mock));
    EXPECT_CALL(*_clock_mock, getTime()).Times(1).WillOnce(Return(fep3::Timestamp{2}));

    // actual test case
    {
        auto ts = _clock_service_intf->getTime(FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME);
        ASSERT_EQ(ts.has_value(), true);
        ASSERT_EQ(ts.value().count(), 0);

        auto ts2 = _clock_service_intf->getTime(_clock_mock->getName());
        ASSERT_EQ(ts2.has_value(), true);
        ASSERT_EQ(ts2.value(), fep3::Timestamp{2});
    }
}

/**
 * @detail Test whether the clock service provides a fep3::Timestamp(0) if getTime is called
 * when the service is not started.
 * @req_id FEPSDK-2431
 */
TEST_F(NativeClockService, getTime__successOnGetTimeServiceNotStarted)
{
    // actual test case
    {
        auto ts = _clock_service_intf->getTime();
        ASSERT_EQ(ts, fep3::Timestamp(0));
    }
}

/**
 * @detail Test whether the clock service returns an Optional without value when requesting the time
 * of a non existent clock.
 */
TEST_F(NativeClockService, getTime__successeOnGetTimeNonExistentClock)
{
    std::string non_existent_clock_name{"non_existent_clock"};

    // actual test case
    {
        EXPECT_CALL((*_logger), logWarning(_))
            .Times(1)
            .WillOnce(::testing::Return(::fep3::Result{}));

        ASSERT_EQ(_clock_service_intf->getTime(non_existent_clock_name).has_value(), false);
    }
}

/**
 * @detail Test whether the clock service provides the main clock's type if getType is called.
 * Either before initialization of the participant or afterwards.
 * @req_id FEPSDK-2432
 */
TEST_F(NativeClockServiceWithClockMocks, getType__successOnGetMainClockType)
{
    ASSERT_FEP3_NOERROR(_clock_service_intf->registerClock(_clock_mock));
    ASSERT_FEP3_NOERROR(_clock_service_intf->registerClock(_clock_mock_2));

    ON_CALL(*_clock_mock, getType())
        .WillByDefault(Return(fep3::arya::IClock::ClockType::continuous));
    ON_CALL(*_clock_mock_2, getType())
        .WillByDefault(Return(fep3::arya::IClock::ClockType::discrete));

    // native continuous clock
    {
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
            *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY),
            FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME));
        ASSERT_EQ(fep3::arya::IClock::ClockType::continuous, _clock_service_intf->getType());
        ASSERT_FEP3_NOERROR(_component_registry->tense());
        ASSERT_EQ(fep3::arya::IClock::ClockType::continuous, _clock_service_intf->getType());
        ASSERT_FEP3_NOERROR(_component_registry->relax());
        ASSERT_EQ(fep3::arya::IClock::ClockType::continuous, _clock_service_intf->getType());
    }

    // native discrete clock
    {
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
            *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY),
            FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME));
        ASSERT_EQ(fep3::arya::IClock::ClockType::discrete, _clock_service_intf->getType());
        ASSERT_FEP3_NOERROR(_component_registry->tense());
        ASSERT_EQ(fep3::arya::IClock::ClockType::discrete, _clock_service_intf->getType());
        ASSERT_FEP3_NOERROR(_component_registry->relax());
        ASSERT_EQ(fep3::arya::IClock::ClockType::discrete, _clock_service_intf->getType());
    }

    // custom continous clock
    {
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
            *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY),
            _clock_mock->getName()));
        ASSERT_EQ(_clock_mock->getType(), _clock_service_intf->getType());
        ASSERT_FEP3_NOERROR(_component_registry->tense());
        ASSERT_EQ(_clock_mock->getType(), _clock_service_intf->getType());
        ASSERT_FEP3_NOERROR(_component_registry->relax());
        ASSERT_EQ(_clock_mock->getType(), _clock_service_intf->getType());
    }

    // custom discrete clock
    {
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
            *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY),
            _clock_mock_2->getName()));
        ASSERT_EQ(_clock_mock_2->getType(), _clock_service_intf->getType());
        ASSERT_FEP3_NOERROR(_component_registry->tense());
        ASSERT_EQ(_clock_mock_2->getType(), _clock_service_intf->getType());
        ASSERT_FEP3_NOERROR(_component_registry->relax());
        ASSERT_EQ(_clock_mock_2->getType(), _clock_service_intf->getType());
    }
}

/**
 * @detail Test whether the clock service logs a warning if the configured main clock's type shall
 * be retrieved but the corresponding main clock is not available.
 */
TEST_F(NativeClockServiceWithClockMocks, getType__sucessOnGetNonExistentMainClockType)
{
    const auto non_existent_clock_name = "non_existent_clock";

    EXPECT_CALL(*_logger, isWarningEnabled()).WillRepeatedly(Return(true));
    EXPECT_CALL(*_logger, logWarning(_)).WillRepeatedly(Return(fep3::Result{}));
    EXPECT_CALL(*_logger,
                logWarning(fep3::mock::LogStringRegexMatcher(
                    std::string() + "failed. .*clock .* name " + non_existent_clock_name + "*.")))
        .WillOnce(Return(fep3::Result{}));

    // not registered clock
    {
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
            *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY),
            non_existent_clock_name));
        // clock type of current clock is returned and therefore continuous
        ASSERT_EQ(fep3::arya::IClock::ClockType::continuous, _clock_service_intf->getType());
    }
}

/**
 * @detail Test whether the clock service provides the clock's type for specific clocks.
 * @req_id FEPSDK-2433
 */
TEST_F(NativeClockServiceWithClockMocks, getType__successOnGetClockTypeByName)
{
    ASSERT_FEP3_NOERROR(_clock_service_intf->registerClock(_clock_mock));
    EXPECT_CALL(*_clock_mock, getType())
        .Times(1)
        .WillOnce(Return(fep3::arya::IClock::ClockType::continuous));

    // actual test case
    {
        auto type = _clock_service_intf->getType(FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME);
        ASSERT_EQ(type.has_value(), true);
        ASSERT_EQ(type.value(), fep3::arya::IClock::ClockType::discrete);

        auto type2 = _clock_service_intf->getType(_clock_mock->getName());
        ASSERT_EQ(type2.has_value(), true);
        ASSERT_EQ(type2.value(), fep3::arya::IClock::ClockType::continuous);
    }
}

/**
 * @detail Test whether the clock service returns an Optional without value when requesting the time
 * of a non existent clock.
 */
TEST_F(NativeClockService, getType__successOnGetTypeNonExistentClock)
{
    std::string non_existent_clock_name{"non_existent_clock"};

    // actual test case
    {
        EXPECT_CALL((*_logger), logWarning(_))
            .Times(1)
            .WillOnce(::testing::Return(::fep3::Result{}));

        ASSERT_EQ(_clock_service_intf->getType(non_existent_clock_name).has_value(), false);
    }
}

/**
 * @detail Test whether the clock service returns and logs an error if a clock is registered while
 * the clock service is started.
 * @req_id 2136
 */
TEST_F(NativeClockServiceWithClockMocks, registerClock__successOnRegisterClockWhileRunning)
{
    ASSERT_FEP3_NOERROR(_component_registry->start());

    // actual test case
    {
        EXPECT_CALL((*_logger), logError(_)).Times(1).WillOnce(Return(::fep3::Result{}));

        ASSERT_FEP3_RESULT(_clock_service_intf->registerClock(_clock_mock),
                           fep3::ERR_INVALID_STATE);
    }

    ASSERT_FEP3_NOERROR(_component_registry->stop());
}

/**
 * @detail Test whether the clock service returns and logs an error if a clock is unregistered while
 * the clock service is started.
 * @req_id FEPSDK-2137
 */
TEST_F(NativeClockService, unregisterClock__successOnUnregisterClockWhileRunning)
{
    std::string clock_name = "test_clock";

    ASSERT_FEP3_NOERROR(_component_registry->start());

    // actual test case
    {
        EXPECT_CALL((*_logger), logError(_)).Times(1).WillOnce(Return(fep3::Result{}));

        ASSERT_FEP3_RESULT(_clock_service_intf->unregisterClock(clock_name),
                           fep3::ERR_INVALID_STATE);
    }

    ASSERT_FEP3_NOERROR(_component_registry->stop());
}

/**
 * @detail Test whether the clock service logs a warning if the main clock is being unregistered and
 * sets clock 'FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME' as new main clock.
 * @req_id FEPSDK-2732
 */
TEST_F(NativeClockServiceWithClockMocks, unregisterClock__successOnUnregisterMainClock)
{
    ASSERT_FEP3_NOERROR(_clock_service_intf->registerClock(_clock_mock));
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY), _clock_mock->getName()));

    ASSERT_FEP3_NOERROR(_component_registry->tense());

    // actual test case
    {
        EXPECT_CALL(
            (*_logger),
            logWarning(fep3::mock::LogStringRegexMatcher(
                std::string() + "Unregistered main clock " + _clock_mock->getName() +
                ". Reset main clock to default value " + FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME)))
            .Times(1)
            .WillOnce(Return(fep3::Result{}));

        ASSERT_FEP3_NOERROR(_clock_service_intf->unregisterClock(_clock_mock->getName()));
    }

    ASSERT_EQ(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME, _clock_service_intf->getMainClockName());
}

/**
 * @detail Test whether an event sink might be registered at the clock service to receive events
 * and might be unregistered to not receive events anymore.
 * @req_id FEPSDK-2143, FEPSDK-2144
 */
TEST_F(NativeClockServiceWithClockMocks, registerEventSink__successOnRegisterUnregisterEventSink)
{
    const auto event_sink_mock = std::make_shared<ExperimentalEventSinkMock>();
    std::weak_ptr<IClock::IEventSink> clock_event_sink;

    ASSERT_FEP3_NOERROR(_clock_service_intf->registerClock(_clock_mock));
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY), _clock_mock->getName()));

    EXPECT_CALL(*_clock_mock, start(_))
        .Times(2)
        .WillRepeatedly(
            Invoke([&clock_event_sink](const std::weak_ptr<IClock::IEventSink>& event_sink) {
                clock_event_sink = event_sink;
            }));

    // actual test case registration
    {
        EXPECT_CALL(*_logger, isDebugEnabled()).WillRepeatedly(Return(true));

        EXPECT_CALL(*_logger, logDebug(_)).WillRepeatedly(Return(fep3::Result{}));
        EXPECT_CALL(
            *_logger,
            logDebug(fep3::mock::LogStringRegexMatcher(std::string() + "Registered event sink")))
            .Times(1)
            .WillRepeatedly(Return(fep3::Result{}));

        ASSERT_FEP3_NOERROR(_clock_service_intf->registerEventSink(event_sink_mock));
        ASSERT_FEP3_NOERROR(_component_registry->tense());
        ASSERT_FEP3_NOERROR(_component_registry->start());

        EXPECT_CALL(*event_sink_mock,
                    timeUpdating(fep3::Timestamp{0}, Optional(fep3::Timestamp{1})))
            .Times(1);

        auto ptr = clock_event_sink.lock();
        ptr->timeUpdating(fep3::Timestamp{0}, fep3::Timestamp{1});

        ASSERT_FEP3_NOERROR(_component_registry->stop());
    }

    // actual test case deregistration
    {
        EXPECT_CALL(*_logger, logDebug(_)).WillRepeatedly(Return(fep3::Result{}));
        EXPECT_CALL(
            *_logger,
            logDebug(fep3::mock::LogStringRegexMatcher(std::string() + "Unregistered event sink")))
            .WillOnce(Return(fep3::Result{}));

        ASSERT_FEP3_NOERROR(_clock_service_intf->unregisterEventSink(event_sink_mock));
        ASSERT_FEP3_NOERROR(_component_registry->start());

        EXPECT_CALL(*event_sink_mock, timeUpdating(_, _)).Times(0);

        auto ptr = clock_event_sink.lock();
        ptr->timeUpdating(fep3::Timestamp{0}, std::nullopt);

        ASSERT_FEP3_NOERROR(_component_registry->stop());
    }
}

TEST_F(NativeClockServiceWithClockMocks, start__successOnEventSinkReceivesEvents)
{
    const auto event_sink_mock = std::make_shared<ExperimentalEventSinkMock>();
    std::weak_ptr<IClock::IEventSink> clock_event_sink;

    ASSERT_FEP3_NOERROR(_clock_service_intf->registerClock(_clock_mock));
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY), _clock_mock->getName()));

    EXPECT_CALL(*_clock_mock, start(_))
        .Times(1)
        .WillOnce(Invoke([&clock_event_sink](const std::weak_ptr<IClock::IEventSink>& event_sink) {
            clock_event_sink = event_sink;
        }));

    // actual test
    {
        ASSERT_FEP3_NOERROR(_clock_service_intf->registerEventSink(event_sink_mock));
        ASSERT_FEP3_NOERROR(_component_registry->tense());

        ASSERT_FEP3_NOERROR(_component_registry->start());

        EXPECT_CALL(*_logger, isDebugEnabled()).WillRepeatedly(Return(true));

        EXPECT_CALL(*_logger, logDebug(_)).WillRepeatedly(Return(fep3::Result{}));
        EXPECT_CALL(*_logger,
                    logDebug(fep3::mock::LogStringRegexMatcher(
                        std::string() + "Distributing 'timeUpdateBegin' .* '0', new time '1'.")))
            .WillOnce(Return(fep3::Result{}));
        EXPECT_CALL(*event_sink_mock, timeUpdateBegin(_, _)).Times(1);
        EXPECT_CALL(*_logger,
                    logDebug(fep3::mock::LogStringRegexMatcher(
                        std::string() + "Distributing 'timeUpdating' .* New time '0'.")))
            .WillOnce(Return(fep3::Result{}));
        EXPECT_CALL(*event_sink_mock,
                    timeUpdating(fep3::Timestamp{0}, Optional(fep3::Timestamp{10})))
            .Times(1);
        EXPECT_CALL(*_logger,
                    logDebug(fep3::mock::LogStringRegexMatcher(
                        std::string() + "Distributing 'timeUpdateEnd' .* New time '0'.")))
            .WillOnce(Return(fep3::Result{}));
        EXPECT_CALL(*event_sink_mock, timeUpdateEnd(_)).Times(1);
        EXPECT_CALL(*event_sink_mock, timeResetBegin(_, _)).Times(1);
        EXPECT_CALL(*event_sink_mock, timeResetEnd(_)).Times(1);

        auto ptr = clock_event_sink.lock();
        ptr->timeUpdateBegin(fep3::Timestamp{0}, fep3::Timestamp{1});
        ptr->timeUpdating(fep3::Timestamp{0}, fep3::Timestamp{10});
        ptr->timeUpdateEnd(fep3::Timestamp{0});
        ptr->timeResetBegin(fep3::Timestamp{0}, fep3::Timestamp{1});
        ptr->timeResetEnd(fep3::Timestamp{0});

        ASSERT_FEP3_NOERROR(_component_registry->stop());
    }
}

/**
 * @detail Tests that the continuous clock will emit the timeReset events
 * Reset event should be emitted aftert startup and also after restart.
 */
TEST_F(NativeClockService, start__successOnContinuousResetEvent)
{
    auto _event_sink_mock = std::make_shared<fep3::mock::Clock::EventSink>();
    test::helper::Notification reset_end_received;
    test::helper::Notification reset_end_received_after_restart;

    ASSERT_FEP3_NOERROR(_clock_service_intf->registerEventSink(_event_sink_mock));

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue(
        *(_component_registry->getComponent<fep3::IConfigurationService>()),
        FEP3_CLOCK_SERVICE_MAIN_CLOCK,
        FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME));

    // Reset event at startup
    {
        EXPECT_CALL(*_event_sink_mock, timeResetBegin(fep3::Duration{0}, Ge(fep3::Duration{0})))
            .Times(1);
        EXPECT_CALL(*_event_sink_mock, timeResetEnd(Ge(fep3::Duration{0})))
            .WillOnce(Notify(&reset_end_received));

        ASSERT_FEP3_NOERROR(_component_registry->initialize());
        ASSERT_FEP3_NOERROR(_component_registry->tense());
        ASSERT_FEP3_NOERROR(_component_registry->start());

        // we make sure that retrieving time for the first time does not lead to another reset event
        _clock_service_intf->getTime();

        reset_end_received.waitForNotificationWithTimeout(std::chrono::seconds(1));
        ASSERT_FEP3_NOERROR(_component_registry->stop());
    }

    // Reset event after restart
    {
        EXPECT_CALL(*_event_sink_mock, timeResetBegin(Ge(fep3::Duration{0}), Ge(fep3::Duration{0})))
            .Times(1);
        EXPECT_CALL(*_event_sink_mock, timeResetEnd(Ge(fep3::Duration{0})))
            .WillOnce(Notify(&reset_end_received_after_restart));

        ASSERT_FEP3_NOERROR(_component_registry->start());

        // we make sure that retrieving time for the first time does not lead to another reset event
        _clock_service_intf->getTime();

        reset_end_received_after_restart.waitForNotificationWithTimeout(std::chrono::seconds(1));
        ASSERT_FEP3_NOERROR(_component_registry->stop());
    }
}

/**
 * @brief Tests that the discrete clock will emit the timeReset events
 * Reset event should be emitted after startup and also after restart.
 */
TEST_F(NativeClockService, start__successOnDiscreteResetEvent)
{
    auto _event_sink_mock = std::make_shared<NiceMock<fep3::mock::Clock::EventSink>>();
    test::helper::Notification updating_received;
    test::helper::Notification updating_received_after_restart;

    ASSERT_FEP3_NOERROR(_clock_service_intf->registerEventSink(_event_sink_mock));

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue(
        *(_component_registry->getComponent<fep3::IConfigurationService>()),
        FEP3_CLOCK_SERVICE_MAIN_CLOCK,
        FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME));

    // Reset event at startup
    {
        EXPECT_CALL(*_event_sink_mock, timeResetBegin(fep3::Duration{0}, fep3::Duration{0}))
            .Times(1);
        EXPECT_CALL(*_event_sink_mock, timeResetEnd(fep3::Duration{0})).Times(1);
        EXPECT_CALL(*_event_sink_mock, timeUpdating(_, Ne(std::nullopt))).Times(1);
        EXPECT_CALL(*_event_sink_mock, timeUpdating(Gt(fep3::Duration{0}), Ne(std::nullopt)))
            .WillRepeatedly(Notify(&updating_received));

        ASSERT_FEP3_NOERROR(_component_registry->initialize());
        ASSERT_FEP3_NOERROR(_component_registry->tense());
        ASSERT_FEP3_NOERROR(_component_registry->start());

        // we make sure that retrieving time for the first time does not lead to another reset event
        _clock_service_intf->getTime();

        updating_received.waitForNotificationWithTimeout(std::chrono::seconds(1));
        ASSERT_FEP3_NOERROR(_component_registry->stop());
    }

    // Reset event after restart
    {
        EXPECT_CALL(*_event_sink_mock, timeResetBegin(Gt(fep3::Duration{0}), fep3::Duration{0}))
            .Times(1);
        EXPECT_CALL(*_event_sink_mock, timeResetEnd(fep3::Duration{0})).Times(1);
        EXPECT_CALL(*_event_sink_mock, timeUpdating(_, Ne(std::nullopt))).Times(1);
        EXPECT_CALL(*_event_sink_mock, timeUpdating(Gt(fep3::Duration{0}), Ne(std::nullopt)))
            .WillRepeatedly(Notify(&updating_received_after_restart));

        ASSERT_FEP3_NOERROR(_component_registry->start());

        // we make sure that retrieving time for the first time does not lead to another reset event
        _clock_service_intf->getTime();

        ASSERT_TRUE(updating_received_after_restart.waitForNotificationWithTimeout(
            std::chrono::seconds(1)));
        ASSERT_FEP3_NOERROR(_component_registry->stop());
    }
}

using ClockTypes = ::testing::Types<ExperimentalClock, AryaClock>;
TYPED_TEST_SUITE(ClockServiceClockCompatibilityTests, ClockTypes);

/**
 * @detail Tests the iteration over all states of the clock service and a registered clock.
 */
TYPED_TEST(ClockServiceClockCompatibilityTests, start__successOnIterateAllStates)
{
    EXPECT_CALL(*(this->_clock_mock), start(_)).Times(1);
    EXPECT_CALL(*(this->_clock_mock), stop()).Times(1);

    ASSERT_FEP3_NOERROR(this->_component_registry->initialize());
    ASSERT_FEP3_NOERROR(this->_component_registry->tense());
    ASSERT_FEP3_NOERROR(this->_component_registry->start());

    ASSERT_FEP3_NOERROR(this->_component_registry->stop());
    ASSERT_FEP3_NOERROR(this->_component_registry->relax());
    ASSERT_FEP3_NOERROR(this->_component_registry->deinitialize());
}

using ClockAndSinkTypes = ::testing::Types<std::tuple<ExperimentalClock, ExperimentalEventSinkMock>,
                                           std::tuple<ExperimentalClock, AryaEventSinkMock>,
                                           std::tuple<AryaClock, ExperimentalEventSinkMock>,
                                           std::tuple<AryaClock, AryaEventSinkMock>>;

TYPED_TEST_SUITE(ClockServiceEventSinkCompatibilityTests, ClockAndSinkTypes);

/**
 * @detail Test whether an event sink registered at the clock service receives following kinds of
 * events
 * - timeUpdateBegin
 * - timeUpdating
 * - timeUpdateEnd
 * - timeResetBegin
 * - timeResetEnd
 * and whether the corresponding debug information is logged.
 * @req_id FEPSDK-2112, FEPSDK-2139, FEPSDK-2140, FEPSDK-2141, FEPSDK-2142
 */
TYPED_TEST(ClockServiceEventSinkCompatibilityTests, start__successOnEventSinkReceivesEvents)
{
    EXPECT_CALL(*(this->_logger), isDebugEnabled()).WillRepeatedly(Return(true));

    EXPECT_CALL(*(this->_logger), logDebug(_)).WillRepeatedly(Return(fep3::Result{}));
    EXPECT_CALL(*(this->_logger),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    std::string() + "Distributing 'timeUpdateBegin' .* '0', new time '1'.")))
        .WillOnce(Return(fep3::Result{}));
    EXPECT_CALL(*(this->_event_sink_mock), timeUpdateBegin(_, _)).Times(1);
    EXPECT_CALL(*(this->_logger),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    std::string() + "Distributing 'timeUpdating' .* New time '0'.")))
        .WillOnce(Return(fep3::Result{}));
    this->setUpdateExpectation();
    EXPECT_CALL(*(this->_logger),
                logDebug(fep3::mock::LogStringRegexMatcher(
                    std::string() + "Distributing 'timeUpdateEnd' .* New time '0'.")))
        .WillOnce(Return(fep3::Result{}));
    EXPECT_CALL(*(this->_event_sink_mock), timeUpdateEnd(_)).Times(1);
    EXPECT_CALL(*(this->_event_sink_mock), timeResetBegin(_, _)).Times(1);
    EXPECT_CALL(*(this->_event_sink_mock), timeResetEnd(_)).Times(1);

    auto ptr = this->_clock_event_sink.lock();
    ptr->timeUpdateBegin(fep3::Timestamp{0}, fep3::Timestamp{1});
    this->doTimeUpdate(ptr);
    ptr->timeUpdateEnd(fep3::Timestamp{0});
    ptr->timeResetBegin(fep3::Timestamp{0}, fep3::Timestamp{1});
    ptr->timeResetEnd(fep3::Timestamp{0});
}
