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


#include <iterator>

#include "test_plugin.h"

void fep3_plugin_getPluginVersion
    (void(*callback)(void*, const char*), void* destination)
{
    callback(destination, "clock interfaces test c plugin 0.0.1");
}

::fep3::mock::TransferableClockServiceWithAccessToClocks* g_mock_clock_service = nullptr;

void setMockClockService(::fep3::mock::TransferableClockServiceWithAccessToClocks* mock_clock_service)
{
    g_mock_clock_service = mock_clock_service;
}

fep3::IClock* getClock(size_t index)
{
    const auto& clocks = g_mock_clock_service->getRegisteredClocks();
    if(index < clocks.size())
    {
        auto clock_iterator = clocks.cbegin();
        std::advance(clock_iterator, index);
        return *clock_iterator;
    }
    else
    {
        return {};
    }
}