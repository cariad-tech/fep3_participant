/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2023 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include "external_clock_intf.h"

class ExternalClock : public IExternalClock {
public:
    TimePoint now()
    {
        return std::chrono::steady_clock::now();
    }

    void waitUntil(std::function<void()> func)
    {
        func();
    }

    void notify(std::function<void()> func)
    {
        func();
    }
};
