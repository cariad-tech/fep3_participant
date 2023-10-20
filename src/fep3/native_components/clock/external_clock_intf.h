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

#include <chrono>
#include <functional>

class IExternalClock {
public:
    using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

    virtual ~IExternalClock() = default;
    virtual TimePoint now() = 0;
    virtual void waitUntil(std::function<void()> func) = 0;
    virtual void notify(std::function<void()> func) = 0;
};
