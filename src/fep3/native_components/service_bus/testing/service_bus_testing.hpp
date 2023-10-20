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
#include "../include/service_bus.h"

#include <a_util/result/result_util.h>

#include <gtest/gtest.h>

namespace fep3 {
namespace native {
namespace testing {

static constexpr const char* participant_name_default = "test_participant_name";

inline ::testing::AssertionResult prepareServiceBusForTestingDefault(
    ServiceBus& service_bus,
    const std::string& test_participant_name = participant_name_default,
    const uint32_t test_participant_port = 0)
{
    const std::string participant_host = "http://localhost:";
    const std::string system_name = "test_with_service_bus_default";

    auto res = service_bus.createSystemAccess(system_name, "", true);
    if (!res) {
        return ::testing::AssertionFailure()
               << __FILE__ << ":" << __LINE__
               << " createSystemAccess: " + a_util::result::toString(res);
    }

    auto sysaccess = service_bus.getSystemAccess(system_name);
    if (!sysaccess) {
        return ::testing::AssertionFailure()
               << __FILE__ << ":" << __LINE__ << " getSystemAccess() failed";
    }

    auto participant_url = participant_host + a_util::strings::toString(test_participant_port);
    res = sysaccess->createServer(test_participant_name, participant_url);

    if (!res) {
        return ::testing::AssertionFailure()
               << __FILE__ << ":" << __LINE__ << " createServer: " + a_util::result::toString(res);
    }
    return ::testing::AssertionSuccess();
}

} // namespace testing
} // namespace native
} // namespace fep3
