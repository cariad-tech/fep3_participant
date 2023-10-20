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
#include <fep3/fep3_timestamp.h>
#include <fep3/native_components/scheduler/clock_based/task_executor_intf.h>

#include <gmock/gmock.h>

#include <notification_waiting.h>
#include <thread>
namespace fep3::mock {

class ScheduledTaskMock {
public:
    ScheduledTaskMock() = default;
    MOCK_METHOD(void, run, (fep3::Timestamp));

    void operator()(fep3::Timestamp time)
    {
        run(time);
        _thread_id = std::this_thread::get_id();
    }

    std::thread::id _thread_id;
};

class MockThreadPoolExecutor : public native::IThreadPoolExecutor {
public:
    // Simulate the ThreadPoolExecutor's same constructor
    MockThreadPoolExecutor(unsigned long thread_count = 1)
    {
        (void)thread_count;
    }

    void start() override
    {
    }

    void stop() override
    {
    }

    void post(std::function<void()> f) override
    {
        f();
    }

    void postAt(std::chrono::milliseconds, std::function<void()> f) override
    {
        f();
    }

    uintptr_t postPeriodic(std::chrono::milliseconds, std::function<bool()> f) override
    {
        f();
        return 0;
    }

    [[nodiscard]] virtual std::future<void> postWithCompletionFuture(
        std::function<void()> f) override
    {
        f();
        _promise = std::promise<void>();
        _promise.set_value();
        return _promise.get_future();
    }

    bool cancel(uintptr_t) override
    {
        return true;
    }

private:
    std::promise<void> _promise;
    std::shared_ptr<int> _task;
};

class MockClockFunction {
public:
    MOCK_METHOD(void, getTimeMock, ());

    fep3::Timestamp getTime()
    {
        getTimeMock();
        return _mock_time;
    }

    fep3::Timestamp _mock_time{0};
};

class AsyncTaskExecutorMock {
public:
    MOCK_METHOD(fep3::Duration, run, (fep3::Timestamp));
    MOCK_METHOD(void, timeReset, (fep3::Timestamp, fep3::Timestamp));
    MOCK_METHOD(void, start, ());
    MOCK_METHOD(void, stop, ());
    MOCK_METHOD(void, prepareForNextStart, ());
    MOCK_METHOD(fep3::Result,
                addTask,
                (std::function<void(fep3::Timestamp)>,
                 const std::string&,
                 fep3::Timestamp,
                 fep3::Duration,
                 fep3::Duration));
};

class AsyncTaskExecutorDummy {
public:
    AsyncTaskExecutorDummy(AsyncTaskExecutorMock* mock) : _mock(mock)
    {
    }

    fep3::Duration run(Timestamp current_time)
    {
        return _mock->run(current_time);
    }

    void start()
    {
        _mock->start();
    }

    void stop()
    {
        _mock->stop();
    }

    void prepareForNextStart()
    {
        _mock->prepareForNextStart();
    }

    void timeReset(Timestamp old_time, Timestamp new_time)
    {
        _mock->timeReset(old_time, new_time);
    }

    fep3::Result addTask(std::function<void(fep3::Timestamp)> task,
                         const std::string& name,
                         Timestamp next_instant,
                         Duration period,
                         Duration delay)
    {
        return _mock->addTask(task, name, next_instant, period, delay);
    }

private:
    AsyncTaskExecutorMock* _mock;
};

template <bool waitOnTimeOut = false>
class NotificationWaitingWithMockTimeout {
public:
    NotificationWaitingWithMockTimeout(bool auto_reset) : _waiting(auto_reset)
    {
    }

    void notify()
    {
        _waiting.notify();
    }

    void reset()
    {
        _waiting.reset();
    }

    void waitForNotification()
    {
        _waiting.waitForNotification();
    }

    bool waitForNotificationWithTimeout(const Timestamp&)
    {
        if constexpr (waitOnTimeOut) {
            _waiting.waitForNotification();
        }
        return true;
    }

private:
    fep3::native::NotificationWaiting _waiting;
};

} // namespace fep3::mock
