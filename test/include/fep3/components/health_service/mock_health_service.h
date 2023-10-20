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

#pragma once

#include <fep3/components/base/component.h>
#include <fep3/components/health_service/health_service_intf.h>

#include <gmock/gmock.h>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace catelyn {
namespace detail {

template <template <typename...> class component_base_type>
struct HealthService : public component_base_type<fep3::catelyn::IHealthService> {
    MOCK_METHOD(fep3::Result,
                updateJobStatus,
                (const std::string&, const fep3::catelyn::IHealthService::JobExecuteResult&),
                (override));
    MOCK_METHOD(fep3::Result, resetHealth, (), (override));
};

} // namespace detail

MATCHER_P(JobExecuteResultMatcher,
          other,
          "Equality matcher for fep3::IHealthService::JobExecuteResult")
{
    return (arg.simulation_time == other.simulation_time) &&
           (arg.result_execute_data_in.getErrorCode() ==
            other.result_execute_data_in.getErrorCode()) &&
           (arg.result_execute.getErrorCode() == other.result_execute.getErrorCode()) &&
           (arg.result_execute_data_out.getErrorCode() ==
            other.result_execute_data_out.getErrorCode());
}

using HealthService = detail::HealthService<fep3::base::arya::Component>;

} // namespace catelyn
using catelyn::HealthService;
using catelyn::JobExecuteResultMatcher;
} // namespace mock
} // namespace fep3

///@endcond nodoc