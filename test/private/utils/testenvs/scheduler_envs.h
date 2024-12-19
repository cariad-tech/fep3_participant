/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/logging/mock/mock_logger_addons.h>

namespace fep3 {
namespace test {
namespace env {

struct SchedulerTestEnv {
    inline SchedulerTestEnv() : _logger(std::make_shared<fep3::mock::LoggerWithDefaultBehavior>())
    {
    }

    std::shared_ptr<fep3::mock::LoggerWithDefaultBehavior> _logger;
};

} // namespace env
} // namespace test
} // namespace fep3
