/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <boost/thread/latch.hpp>
#include <gmock/gmock.h>

#include <condition_variable>

namespace test {
namespace helper {

/**
 * @brief Simple action notifying the passed notification object
 */
ACTION_P(CountDown, notification_latch)
{
    notification_latch->countDown();
}

class NotificationLatch {
public:
    /**
     * CTOR
     */
    NotificationLatch(size_t call_num) : _timer_reset(true), _latch(call_num)
    {
    }

    /**
     * DTOR
     */
    virtual ~NotificationLatch() = default;

    /**
     * Wait for latch count down
     *
     * @return elapsed time, starting from the first count down to the last
     */
    std::chrono::nanoseconds wait()
    {
        _latch.wait();
        return std::chrono::steady_clock::now() - _start;
    }

    /**
     * Count down latch
     * If it is the first count down, it will start the timer.
     */
    void countDown()
    {
        {
            std::lock_guard<std::mutex> lk(_mutex);
            if (_timer_reset) {
                _timer_reset = false;
                _start = std::chrono::steady_clock::now();
            }
        }

        _latch.count_down();
    }

    /**
     * Reset latch
     * @param calls_num number of calls latch should wait for
     */
    void reset(size_t calls_num)
    {
        _latch.reset(calls_num);
        _timer_reset = true;
    }

private:
    std::mutex _mutex;
    std::chrono::time_point<std::chrono::steady_clock> _start;
    boost::latch _latch;
    bool _timer_reset;
};

} // namespace helper
} // namespace test
