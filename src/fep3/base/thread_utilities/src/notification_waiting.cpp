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

#include "notification_waiting.h"

namespace fep3::native {

NotificationWaiting::NotificationWaiting(bool auto_reset) : _auto_reset(auto_reset)
{
}

void NotificationWaiting::notify()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _notified = true;
    _cv.notify_all();
}

void NotificationWaiting::waitForNotification()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock, [this]() { return _notified; });
    if (_auto_reset) {
        _notified = false;
    }
}

bool NotificationWaiting::waitForNotificationWithTimeout(const Timestamp& timeout)
{
    std::unique_lock<std::mutex> lock(_mutex);
    auto wait_flag = _cv.wait_for(lock, timeout, [this]() { return _notified; });
    if (_auto_reset) {
        _notified = false;
    }
    return wait_flag;
}

void NotificationWaiting::reset()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _notified = false;
    _cv.notify_all();
}
} // namespace fep3::native
