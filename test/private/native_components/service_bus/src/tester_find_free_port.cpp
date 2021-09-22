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

#include "fep3/native_components/service_bus/rpc/http/find_free_port.h"

/**
 * @detail Test the findFreePortFunction
 * @req_id FEPSDK-HTTP-Find-port
 *
 */
TEST(PortFinderTest, testfindFreePort)
{
    //This test seems dangerous for test stability :
    //imagine, one day we have a load test that tests 100 participants on a system;
    //if such test is executed on jenkins, no free port will be found in this test
    //(if running in another job on the same jenkins slave).
    //Todo: find a solution for that
    auto port_found = fep3::helper::findFreeSocketPort(9091, 100);
    ASSERT_GE(port_found, 9091);
}

/**
 * @detail Test the findFreePortFunction
 * @req_id FEPSDK-HTTP-Find-port
 *
 */
TEST(PortFinderTest, testfindFreePortInvalid)
{
    auto port_found = fep3::helper::findFreeSocketPort(0, 100);
    ASSERT_LT(port_found, 0);

    port_found = fep3::helper::findFreeSocketPort(70, -1);
    ASSERT_LT(port_found, 0);
}
