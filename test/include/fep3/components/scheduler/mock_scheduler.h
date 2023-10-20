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

#include <fep3/components/scheduler/scheduler_intf.h>

#include <gmock/gmock.h>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace arya {

struct Scheduler : public fep3::arya::IScheduler {
    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(fep3::Result,
                initialize,
                (fep3::arya::IClockService&, const fep3::arya::Jobs&),
                (override));
    MOCK_METHOD(fep3::Result, start, (), (override));
    MOCK_METHOD(fep3::Result, stop, (), (override));
    MOCK_METHOD(fep3::Result, deinitialize, (), (override));
};

} // namespace arya

namespace catelyn {

struct Scheduler : public fep3::catelyn::IScheduler {
    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(fep3::Result,
                initialize,
                (fep3::arya::IClockService&, const fep3::arya::Jobs&),
                (override));
    MOCK_METHOD(fep3::Result, initialize, (const fep3::IComponents&), (override));
    MOCK_METHOD(fep3::Result, start, (), (override));
    MOCK_METHOD(fep3::Result, stop, (), (override));
    MOCK_METHOD(fep3::Result, deinitialize, (), (override));
};

} // namespace catelyn

using catelyn::Scheduler;
} // namespace mock
} // namespace fep3

///@endcond nodoc
