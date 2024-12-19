/**
 * @copyright
 * @verbatim
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @endverbatim
 */
#include "threaded_executor.h"

namespace fep3::native {

PeriodicTask::PeriodicTask(std::chrono::milliseconds delay,
                           std::function<bool()> f,
                           boost::asio::io_service& io_service)
    : _delay(delay), _timer(io_service, _delay), _f(f)
{
}

PeriodicTask::~PeriodicTask()
{
    // also cancels:
    stop();
}

// shared_from_this not allowed in constructor
void PeriodicTask::start()
{
    auto strong_this = shared_from_this();
    boost::asio::post(_timer.get_executor(), [strong_this, this]() { run({}); });
}

/// @brief it does not cover the case where a call is under execution
void PeriodicTask::stop()
{
    _canceled = true;
    _timer.cancel();
}

void PeriodicTask::run(const boost::system::error_code& error)
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

ThreadPoolExecutor::ThreadPoolExecutor(size_t thread_count) : _thread_count(thread_count)
{
    assert(_thread_count > 0);
}

void ThreadPoolExecutor::start()
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

void ThreadPoolExecutor::stop()
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

ThreadPoolExecutor::~ThreadPoolExecutor()
{
    stop();
}

void ThreadPoolExecutor::post(std::function<void()> f)
{
    boost::asio::post(_io_service, std::move(f));
}

void ThreadPoolExecutor::postAt(std::chrono::milliseconds delay_ms, std::function<void()> f)
{
    // lifetime is handled by io service
    auto timer = std::make_shared<boost::asio::steady_timer>(_io_service, delay_ms);
    timer->async_wait([timer, f](const boost::system::error_code&) { f(); });
}

uintptr_t ThreadPoolExecutor::postPeriodic(std::chrono::milliseconds period,
                                           std::function<bool()> f)
{
    auto task = std::make_shared<PeriodicTask>(period, f, _io_service);
    uintptr_t handle = reinterpret_cast<uintptr_t>(task.get());
    task->start();
    _per_tasks.emplace_back(std::move(task));
    return handle;
}

[[nodiscard]] std::future<void> ThreadPoolExecutor::postWithCompletionFuture(
    std::function<void()> f)
{
    return boost::asio::post(_io_service, std::packaged_task<void()>(f));
}

bool ThreadPoolExecutor::cancel(uintptr_t handle)
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

} // namespace fep3::native
