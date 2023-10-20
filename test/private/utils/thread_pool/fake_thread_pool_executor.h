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

#include "threaded_executor.h"

#include <chrono>
#include <future>

namespace fep3::native::mock {

struct FakeThreadPoolExecutor : IThreadPoolExecutor {
private:
    struct TaskWrapper {
        void run(std::chrono::milliseconds time)
        {
            if (next_exec >= time) {
                _task();
                next_exec = next_exec + step;
            }
        }

        uintptr_t index;
        std::chrono::milliseconds step;
        std::function<void()> _task;
        std::chrono::milliseconds next_exec{0};
    };

public:
    FakeThreadPoolExecutor(std::chrono::milliseconds step_ms = std::chrono::milliseconds{1000})
        : step(step_ms)
    {
    }

    using Task = std::function<void()>;

    void start() override
    {
    }

    void stop() override
    {
    }

    void post(std::function<void()> f) override
    {
        _one_execution_tasks.push_back(std::move(f));
    }

    [[nodiscard]] std::future<void> postWithCompletionFuture(std::function<void()> f) override
    {
        _one_execution_tasks_promise.push_back(std::make_pair(std::promise<void>(), f));
        return _one_execution_tasks_promise.back().first.get_future();
    }

    void postAt(std::chrono::milliseconds delay_ms, std::function<void()> f) override
    {
        using namespace std::chrono_literals;
        auto index = index_counter++;
        TaskWrapper task = {index,
                            0ms,
                            [&, _f = std::move(f)]() {
                                _f();
                                cancel(index);
                            },
                            delay_ms};
        _periodic_execution_tasks.push_back(std::move(task));
    }

    uintptr_t postPeriodic(std::chrono::milliseconds period, std::function<bool()> f) override
    {
        auto index = index_counter++;
        _periodic_execution_tasks.push_back(TaskWrapper{index, period, std::move(f)});
        return index;
    }

    bool cancel(uintptr_t task_index) override
    {
        _periodic_execution_tasks.erase(
            std::remove_if(_periodic_execution_tasks.begin(),
                           _periodic_execution_tasks.end(),
                           [&](auto& task) { return task.index == task_index; }),
            _periodic_execution_tasks.end());

        return true;
    }

    void one_execution()
    {
        for (auto& task: _one_execution_tasks) {
            task();
        }
        for (auto& [p, task]: _one_execution_tasks_promise) {
            task();
            p.set_value();
        }

        _one_execution_tasks.clear();
        _one_execution_tasks_promise.clear();
    }

    void periodic_execution()
    {
        for (auto& task: _periodic_execution_tasks) {
            task.run(prev_exec);
        }
        prev_exec = prev_exec + step;
    }

    void run_until(std::chrono::milliseconds time)
    {
        while (prev_exec < time) {
            one_execution();
            periodic_execution();
        }
    }

    void run_for(std::chrono::milliseconds time)
    {
        run_until(prev_exec + time);
    }

    std::vector<std::pair<std::promise<void>, Task>> _one_execution_tasks_promise;
    std::vector<Task> _one_execution_tasks;
    std::vector<TaskWrapper> _periodic_execution_tasks;
    uintptr_t index_counter = 0;
    std::chrono::milliseconds step;
    std::chrono::milliseconds prev_exec{0};
};

} // namespace fep3::native::mock
