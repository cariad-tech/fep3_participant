/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

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
                 boost::asio::io_service& io_service);

    ~PeriodicTask();
    // shared_from_this not allowed in constructor
    void start();

    /// @brief it does not cover the case where a call is under execution
    void stop();

    PeriodicTask(PeriodicTask&&) = default;
    PeriodicTask& operator=(PeriodicTask&&) = default;
    PeriodicTask() = delete;

private:
    void run(const boost::system::error_code& error);

    std::chrono::milliseconds _delay;
    boost::asio::basic_waitable_timer<std::chrono::steady_clock> _timer;
    std::function<bool()> _f;
    std::atomic<bool> _canceled = false;
};

struct ThreadPoolExecutor : IThreadPoolExecutor {
    using StopToken = std::function<bool()>;

    ThreadPoolExecutor(size_t thread_count = 1);

    void start() override;

    void stop() override;

    ~ThreadPoolExecutor();

    ThreadPoolExecutor(const ThreadPoolExecutor&) = delete;
    const ThreadPoolExecutor& operator=(const PeriodicTask&) = delete;

    void post(std::function<void()> f) override;

    void postAt(std::chrono::milliseconds delay_ms, std::function<void()> f) override;

    uintptr_t postPeriodic(std::chrono::milliseconds period, std::function<bool()> f) override;

    [[nodiscard]] std::future<void> postWithCompletionFuture(std::function<void()> f) override;

    bool cancel(uintptr_t handle) override;

    size_t _thread_count;
    boost::thread_group _worker_threads;
    boost::asio::io_service _io_service;
    std::unique_ptr<boost::asio::io_service::work> _work;
    std::vector<std::shared_ptr<PeriodicTask>> _per_tasks;
    bool _running{false};
};
} // namespace fep3::native
