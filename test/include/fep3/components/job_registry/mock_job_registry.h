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
#include <fep3/components/job_registry/job_registry_intf.h>

#include <gmock/gmock.h>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace arya {

namespace detail {

template <template <typename...> class component_base_type>
struct JobRegistry : public component_base_type<fep3::arya::IJobRegistry> {
    MOCK_METHOD(fep3::Result,
                addJob,
                (const std::string&,
                 const std::shared_ptr<IJob>&,
                 const fep3::arya::JobConfiguration&),
                (override));
    MOCK_METHOD(fep3::Result, removeJob, (const std::string&), (override));
    MOCK_METHOD(std::list<fep3::arya::JobInfo>, getJobInfos, (), (const, override));
    MOCK_METHOD(fep3::arya::Jobs, getJobs, (), (const, override));
};

} // namespace detail

using JobRegistry = detail::JobRegistry<fep3::base::arya::Component>;

} // namespace arya

namespace catelyn {
namespace detail {

template <template <typename...> class component_base_type>
struct JobRegistry : public component_base_type<fep3::catelyn::IJobRegistry> {
    MOCK_METHOD(fep3::Result,
                addJob,
                (const std::string&,
                 const std::shared_ptr<IJob>&,
                 const fep3::arya::JobConfiguration&),
                (override));
    MOCK_METHOD(fep3::Result,
                addJob,
                (const std::string&,
                 const std::shared_ptr<fep3::arya::IJob>&,
                 const fep3::catelyn::JobConfiguration&),
                (override));
    MOCK_METHOD(fep3::Result, removeJob, (const std::string&), (override));
    MOCK_METHOD(std::list<fep3::arya::JobInfo>, getJobInfos, (), (const, override));
    MOCK_METHOD(std::list<fep3::catelyn::JobInfo>, getJobInfosCatelyn, (), (const, override));
    MOCK_METHOD(fep3::arya::Jobs, getJobs, (), (const, override));
    MOCK_METHOD(fep3::catelyn::Jobs, getJobsCatelyn, (), (const, override));
};
} // namespace detail

using JobRegistry = detail::JobRegistry<fep3::base::arya::Component>;

} // namespace catelyn

using catelyn::JobRegistry;

} // namespace mock
} // namespace fep3

///@endcond nodoc
