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

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>

namespace fep3::native {

struct IThreadPoolExecutor {
    using StopToken = std::function<bool()>;

    virtual ~IThreadPoolExecutor(){};

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void post(std::function<void()> f) = 0;
    virtual void postAt(std::chrono::milliseconds delay_ms, std::function<void()> f) = 0;
    virtual uintptr_t postPeriodic(std::chrono::milliseconds period, std::function<bool()> f) = 0;
    [[nodiscard]] virtual std::future<void> postWithCompletionFuture(std::function<void()> f) = 0;

    virtual bool cancel(uintptr_t handle) = 0;
};
// uses shared from this because the timer can expire and the
// handler be called even if the destructor of the class is called
struct PeriodicTask : public std::enable_shared_from_this<PeriodicTask> {
    PeriodicTask(std::chrono::milliseconds delay,
                 std::function<bool()> f,
                 boost::asio::io_service& io_service)
        : _delay(delay), _timer(io_service, _delay), _f(f)
    {
    }

    ~PeriodicTask()
    {
        // also cancels:
        stop();
    }

    // shared_from_this not allowed in constructor
    void start()
    {
        auto strong_this = shared_from_this();
        boost::asio::post(_timer.get_executor(), [strong_this, this]() { run({}); });
    }

    void stop()
    {
        _canceled = true;
        _timer.cancel();
    }

    PeriodicTask(PeriodicTask&&) = default;
    PeriodicTask& operator=(PeriodicTask&&) = default;
    PeriodicTask() = delete;

private:
    void run(const boost::system::error_code& error)
    {
        switch (error.value()) {
        case boost::system::errc::success: {
            if (!_canceled) {
                auto res = _f();
                if (!_canceled && res) {
                    _timer.expires_from_now(_delay);
                    auto strongThis = shared_from_this();
                    _timer.async_wait(
                        [strongThis, this](const boost::system::error_code& ec) { run(ec); });
                }
            }
        } break;

        case boost::system::errc::operation_canceled: {
            // Timer cancelled
        } break;

        default: {
            // unexpected case
        } break;
        }
    }

    std::chrono::milliseconds _delay;
    boost::asio::basic_waitable_timer<std::chrono::steady_clock> _timer;
    std::function<bool()> _f;
    std::atomic<bool> _canceled = false;
};

struct ThreadPoolExecutor : IThreadPoolExecutor {
    using StopToken = std::function<bool()>;

    ThreadPoolExecutor(size_t thread_count = 1) : _thread_count(thread_count)
    {
        assert(_thread_count > 0);
    }

    void start() override
    {
        if (_running) {
            return;
        }

        if (_io_service.stopped()) {
            _io_service.restart();
        }

        _work = std::make_unique<boost::asio::io_service::work>(_io_service);
        for (unsigned long i = 0; i < _thread_count; ++i)
            _worker_threads.create_thread(boost::bind(&boost::asio::io_service::run, &_io_service));

        _running = true;
    }

    void stop() override
    {
        if (!_running) {
            return;
        }

        _work.reset();

        if (!_io_service.stopped()) {
            _io_service.stop();
        }

        for (auto& ref: _per_tasks)
            ref->stop();

        _worker_threads.join_all();
        _per_tasks.clear();

        _running = false;
    }

    ~ThreadPoolExecutor()
    {
        stop();
    }

    ThreadPoolExecutor(const ThreadPoolExecutor&) = delete;
    const ThreadPoolExecutor& operator=(const PeriodicTask&) = delete;

    void post(std::function<void()> f) override
    {
        boost::asio::post(_io_service, std::move(f));
    }

    void postAt(std::chrono::milliseconds delay_ms, std::function<void()> f) override
    {
        // lifetime is handled by io service
        auto timer = std::make_shared<boost::asio::steady_timer>(_io_service, delay_ms);
        timer->async_wait([timer, f](const boost::system::error_code&) { f(); });
    }

    uintptr_t postPeriodic(std::chrono::milliseconds period, std::function<bool()> f) override
    {
        auto task = std::make_shared<PeriodicTask>(period, f, _io_service);
        uintptr_t handle = reinterpret_cast<uintptr_t>(task.get());
        task->start();
        _per_tasks.emplace_back(std::move(task));
        return handle;
    }

    [[nodiscard]] std::future<void> postWithCompletionFuture(std::function<void()> f) override
    {
        return boost::asio::post(_io_service, std::packaged_task<void()>(f));
    }

    bool cancel(uintptr_t handle) override
    {
        auto it = std::find_if(_per_tasks.begin(), _per_tasks.end(), [handle](const auto& ptr) {
            return reinterpret_cast<uintptr_t>(ptr.get()) == handle;
        });

        if (it != _per_tasks.end()) {
            (*it)->stop();
            return true;
        }
        else {
            return false;
        }
    }

    size_t _thread_count;
    boost::thread_group _worker_threads;
    boost::asio::io_service _io_service;
    std::unique_ptr<boost::asio::io_service::work> _work;
    std::vector<std::shared_ptr<PeriodicTask>> _per_tasks;
    bool _running{false};
};
} // namespace fep3::native
