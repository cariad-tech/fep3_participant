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

#include "threaded_executor.h"

#include <boost/thread/latch.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace fep3::native;
using namespace std::chrono_literals;

namespace {
bool function_called = false;
boost::latch func_latch(1);

void free_function()
{
    function_called = true;
    func_latch.count_down();
}

bool free_function_with_ret()
{
    function_called = true;
    func_latch.count_down();
    return true;
}

} // namespace

struct functor {
    void operator()()
    {
        function_called = true;
        func_latch.count_down();
    }

    bool& function_called;
    boost::latch& func_latch;
};

struct functorWithRet {
    bool operator()()
    {
        function_called = true;
        func_latch.count_down();
        return true;
    }

    bool& function_called;
    boost::latch& func_latch;
};

TEST(FepThreadedExecutor, testPostFree)
{
    ThreadPoolExecutor executor;
    executor.start();

    function_called = false;
    func_latch.reset(1);

    executor.post(free_function);

    func_latch.wait();
    ASSERT_TRUE(function_called);
}

TEST(FepThreadedExecutor, testPostAtFree)
{
    ThreadPoolExecutor executor;
    executor.start();

    function_called = false;
    func_latch.reset(1);

    executor.postAt(50ms, free_function);

    func_latch.wait();
    ASSERT_TRUE(function_called);
}

TEST(FepThreadedExecutor, testPostFreePeriodic)
{
    ThreadPoolExecutor executor;
    executor.start();

    function_called = false;
    func_latch.reset(1);

    executor.postPeriodic(50ms, free_function_with_ret);

    func_latch.wait();
    ASSERT_TRUE(function_called);
}

TEST(FepThreadedExecutor, testPostFunctor)
{
    ThreadPoolExecutor executor;
    executor.start();

    bool task_called = false;
    boost::latch task_latch(1);
    functor f{task_called, task_latch};

    executor.post(f);

    task_latch.wait();
    ASSERT_TRUE(task_called);
}

TEST(FepThreadedExecutor, testPostAtFunctor)
{
    ThreadPoolExecutor executor;
    executor.start();

    bool task_called = false;
    boost::latch task_latch(1);
    functor f{task_called, task_latch};

    executor.postAt(50ms, f);

    task_latch.wait();
    ASSERT_TRUE(task_called);
}

TEST(FepThreadedExecutor, testPostFunctorPeriodic)
{
    ThreadPoolExecutor executor;
    executor.start();

    bool task_called = false;
    boost::latch task_latch(1);
    functorWithRet f{task_called, task_latch};

    executor.postPeriodic(50ms, f);

    task_latch.wait();
    ASSERT_TRUE(task_called);
}

TEST(FepThreadedExecutor, testPost)
{
    bool task_called = false;
    boost::latch task_latch(1);
    task_called = true;
    ThreadPoolExecutor executor;
    executor.start();

    executor.post([&]() {
        task_called = true;
        task_latch.count_down();
    });

    task_latch.wait();
    ASSERT_TRUE(task_called);
}

TEST(FepThreadedExecutor, testPost_10Functions)
{
    std::atomic<int> task_called = 0;
    boost::latch task_latch(10);
    ThreadPoolExecutor executor;
    executor.start();

    for (auto i = 0; i < 10; ++i) {
        executor.post([&]() {
            ++task_called;
            task_latch.count_down();
        });
    }

    task_latch.wait();
    ASSERT_EQ(task_called, 10);
}

TEST(FepThreadedExecutor, testpostAt)
{
    bool task_called = false;
    boost::latch task_latch(1);
    task_called = true;
    ThreadPoolExecutor executor;
    executor.start();

    executor.postAt(100ms, [&]() {
        task_called = true;
        task_latch.count_down();
    });

    task_latch.wait();
    ASSERT_TRUE(task_called);
}

TEST(FepThreadedExecutor, periodic)
{
    bool task_called = false;
    boost::latch task_latch(4);
    task_called = true;
    ThreadPoolExecutor executor;
    executor.start();

    executor.postPeriodic(50ms, [&]() {
        task_called = true;
        task_latch.count_down();
        return true;
    });

    task_latch.wait();
    ASSERT_TRUE(task_called);
}

TEST(FepThreadedExecutor, periodic_failed)
{
    boost::latch task_latch(5);
    int times_called = 0;
    ThreadPoolExecutor executor;
    executor.start();

    executor.postPeriodic(50ms, [&]() -> bool {
        ++times_called;
        if (times_called == 2) {
            return false;
        }
        else {
            return true;
        }
    });

    executor.postPeriodic(50ms, [&]() -> bool {
        task_latch.count_down();
        return true;
    });

    task_latch.wait();
    ASSERT_EQ(times_called, 2);
}

// not easy to test without sleep
TEST(FepThreadedExecutor, cancel_periodic)
{
    boost::latch task_latch(5);
    int times_called = 0;
    int stop_task_times_called = 0;
    ThreadPoolExecutor executor;
    executor.start();

    auto handle = executor.postPeriodic(50ms, [&]() -> bool {
        ++stop_task_times_called;
        return true;
    });

    executor.postPeriodic(50ms, [&]() -> bool {
        ++times_called;
        if (times_called == 2) {
            executor.cancel(handle);
        }
        task_latch.count_down();
        return true;
    });

    task_latch.wait();
    ASSERT_LT(stop_task_times_called, times_called);
}
