/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */


#include <gtest/gtest.h>
#include <common/gtest_asserts.h>

#include <fep3/native_components/health/health_service.h>

using namespace ::testing;
using namespace fep3;
using namespace experimental;

/**
 * @brief Test whether the built-in health state component returns the correct default health state (ok) after creation.
 *
 */
TEST(HealthService, getDefaultHealthState)
{
    native::HealthService health_service;

    ASSERT_EQ(experimental::HealthState::ok, health_service.getHealth());
}

/**
 * @brief Test whether the bult-in health state component may correctly set/get error/ok states.
 *
 */
TEST(HealthService, getSetHealthState)
{
    native::HealthService health_service;

    ASSERT_EQ(experimental::HealthState::ok, health_service.getHealth());

    ASSERT_FEP3_NOERROR(health_service.setHealthToError(""));
    ASSERT_EQ(experimental::HealthState::error, health_service.getHealth());

    ASSERT_FEP3_NOERROR(health_service.resetHealth(""));
    ASSERT_EQ(experimental::HealthState::ok, health_service.getHealth());
}
