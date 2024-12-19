/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <fep3/fep3_timestamp.h>

#include <condition_variable>
#include <mutex>

namespace fep3::native {

class NotificationWaiting {
public:
    NotificationWaiting(bool auto_reset = false);
    /**
     * @brief Notifies the notification
     * This method causes any threads currently waiting for a notification
     * in @ref WaitForNotification or @ref WaitForNotificationWithTimeout
     * to return.
     */
    void notify();

    /**
     * Waits for the @ref Notify method to be called
     */
    void waitForNotification();

    bool waitForNotificationWithTimeout(const Timestamp& timeout);

    void reset();

private:
    std::mutex _mutex;
    std::condition_variable _cv;
    bool _notified{false};
    bool _auto_reset;
};
} // namespace fep3::native
