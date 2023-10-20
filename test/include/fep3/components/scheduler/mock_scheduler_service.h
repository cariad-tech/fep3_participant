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
#include <fep3/components/scheduler/scheduler_service_intf.h>

#include <gmock/gmock.h>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace arya {
namespace detail {

template <template <typename...> class component_base_type>
struct SchedulerService : public component_base_type<fep3::arya::ISchedulerService> {
    MOCK_METHOD(std::string, getActiveSchedulerName, (), (const, override));
    MOCK_METHOD(fep3::Result,
                registerScheduler,
                (std::unique_ptr<fep3::arya::IScheduler>),
                (override));
    MOCK_METHOD(fep3::Result, unregisterScheduler, (const std::string&), (override));
    MOCK_METHOD(std::list<std::string>, getSchedulerNames, (), (const, override));
};

} // namespace detail
using SchedulerService = detail::SchedulerService<fep3::base::arya::Component>;
} // namespace arya
namespace catelyn {
namespace detail {

template <template <typename...> class component_base_type>
struct SchedulerService : public component_base_type<fep3::catelyn::ISchedulerService> {
    MOCK_METHOD(std::string, getActiveSchedulerName, (), (const, override));
    MOCK_METHOD(fep3::Result,
                registerScheduler,
                (std::unique_ptr<fep3::arya::IScheduler>),
                (override));
    MOCK_METHOD(fep3::Result,
                registerScheduler,
                (std::unique_ptr<fep3::catelyn::IScheduler>),
                (override));
    MOCK_METHOD(fep3::Result, unregisterScheduler, (const std::string&), (override));
    MOCK_METHOD(std::list<std::string>, getSchedulerNames, (), (const, override));
};

} // namespace detail
using SchedulerService = detail::SchedulerService<fep3::base::arya::Component>;
} // namespace catelyn

using catelyn::SchedulerService;
} // namespace mock
} // namespace fep3

///@endcond nodoc