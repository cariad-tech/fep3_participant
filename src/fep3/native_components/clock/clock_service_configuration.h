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

#pragma once

#include <fep3/base/properties/propertynode.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/logging/logger_intf.h>

#include <string>

namespace fep3::arya {
class ILogger;
} // namespace fep3::arya

namespace fep3 {
namespace native {

struct ClockServiceConfiguration : public base::Configuration {
    ClockServiceConfiguration();

    fep3::Result registerPropertyVariables() override;
    fep3::Result unregisterPropertyVariables() override;
    fep3::Result validateSimClockConfiguration(const std::shared_ptr<ILogger>& logger) const;

private:
    fep3::Result validateSimClockConfigurationProperties(
        const std::shared_ptr<ILogger>& logger) const;
    fep3::Result validateWallClockStepSize(const std::shared_ptr<ILogger>& logger) const;

public:
    base::PropertyVariable<std::string> _main_clock_name{FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME};
    base::PropertyVariable<int64_t> _time_update_timeout{FEP3_TIME_UPDATE_TIMEOUT_DEFAULT_VALUE};
    base::PropertyVariable<double> _clock_sim_time_time_factor{
        FEP3_CLOCK_SIM_TIME_TIME_FACTOR_DEFAULT_VALUE};
    base::PropertyVariable<int64_t> _clock_sim_time_step_size{
        FEP3_CLOCK_SIM_TIME_STEP_SIZE_DEFAULT_VALUE};
};

} // namespace native
} // namespace fep3
