/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "clock_service_configuration.h"

#include <fep3/components/logging/easy_logger.h>

namespace fep3 {
namespace native {

ClockServiceConfiguration::ClockServiceConfiguration() : Configuration(FEP3_CLOCK_SERVICE_CONFIG)
{
}

fep3::Result ClockServiceConfiguration::registerPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_main_clock_name, FEP3_MAIN_CLOCK_PROPERTY));
    FEP3_RETURN_IF_FAILED(
        registerPropertyVariable(_time_update_timeout, FEP3_TIME_UPDATE_TIMEOUT_PROPERTY));
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_clock_sim_time_time_factor,
                                                   FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY));
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_clock_sim_time_step_size,
                                                   FEP3_CLOCK_SIM_TIME_STEP_SIZE_PROPERTY));

    return {};
}

fep3::Result ClockServiceConfiguration::unregisterPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_main_clock_name, FEP3_MAIN_CLOCK_PROPERTY));
    FEP3_RETURN_IF_FAILED(
        unregisterPropertyVariable(_time_update_timeout, FEP3_TIME_UPDATE_TIMEOUT_PROPERTY));
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_clock_sim_time_time_factor,
                                                     FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY));
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_clock_sim_time_step_size,
                                                     FEP3_CLOCK_SIM_TIME_STEP_SIZE_PROPERTY));

    return {};
}

fep3::Result ClockServiceConfiguration::validateSimClockConfiguration(
    const std::shared_ptr<ILogger>& logger) const
{
    FEP3_RETURN_IF_FAILED(validateSimClockConfigurationProperties(logger));

    if (FEP3_CLOCK_SIM_TIME_TIME_FACTOR_AFAP_VALUE == _clock_sim_time_time_factor) {
        return {};
    }

    return validateWallClockStepSize(logger);
}

fep3::Result ClockServiceConfiguration::validateSimClockConfigurationProperties(
    const std::shared_ptr<ILogger>& logger) const
{
    constexpr int64_t sim_time_step_size_min_value = FEP3_CLOCK_SIM_TIME_STEP_SIZE_MIN_VALUE;
    constexpr int64_t sim_time_step_size_max_value = FEP3_CLOCK_SIM_TIME_STEP_SIZE_MAX_VALUE;
    constexpr double sim_time_step_size_afap_value = FEP3_CLOCK_SIM_TIME_TIME_FACTOR_AFAP_VALUE;

    if (sim_time_step_size_min_value > _clock_sim_time_step_size ||
        sim_time_step_size_max_value < _clock_sim_time_step_size) {
        const auto result = CREATE_ERROR_DESCRIPTION(
            ERR_INVALID_ARG,
            a_util::strings::format(
                "Invalid clock service configuration: Invalid clock step size of '%f' ns. "
                "Clock step size has to be >= %lld and <= %lld.",
                static_cast<double>(_clock_sim_time_step_size),
                sim_time_step_size_min_value,
                sim_time_step_size_max_value)
                .c_str());

        FEP3_ARYA_LOGGER_LOG_RESULT(logger, result);

        return result;
    }
    else if (sim_time_step_size_afap_value > _clock_sim_time_time_factor) {
        const auto result = CREATE_ERROR_DESCRIPTION(
            ERR_INVALID_ARG,
            a_util::strings::format(
                "Invalid clock service configuration: Invalid clock time factor of '%f'. "
                "Clock time factor has to be >= %f.",
                static_cast<double>(_clock_sim_time_time_factor),
                sim_time_step_size_afap_value)
                .c_str());

        FEP3_ARYA_LOGGER_LOG_RESULT(logger, result);

        return result;
    }

    return {};
}

fep3::Result ClockServiceConfiguration::validateWallClockStepSize(
    const std::shared_ptr<ILogger>& logger) const
{
    constexpr int64_t wall_clock_step_size_min_value = FEP3_CLOCK_WALL_CLOCK_STEP_SIZE_MIN_VALUE;
    constexpr int64_t wall_clock_step_size_max_value = FEP3_CLOCK_WALL_CLOCK_STEP_SIZE_MAX_VALUE;
    const double wall_clock_step_size =
        static_cast<double>(_clock_sim_time_step_size) / _clock_sim_time_time_factor;

    if (wall_clock_step_size < static_cast<double>(wall_clock_step_size_min_value) ||
        wall_clock_step_size > static_cast<double>(wall_clock_step_size_max_value)) {
        const auto result = CREATE_ERROR_DESCRIPTION(
            ERR_INVALID_ARG,
            a_util::strings::format(
                "Invalid clock service configuration: Invalid wall clock step size of '%f' ns "
                "resulting by dividing configured step size '%f' by time factor '%f'. Wall clock "
                "step size has to be >= %lld ns and <= %lld ns.",
                wall_clock_step_size,
                static_cast<double>(_clock_sim_time_step_size),
                static_cast<double>(_clock_sim_time_time_factor),
                wall_clock_step_size_min_value,
                wall_clock_step_size_max_value)
                .c_str());

        FEP3_ARYA_LOGGER_LOG_RESULT(logger, result);

        return result;
    }

    return {};
}

} // namespace native
} // namespace fep3
