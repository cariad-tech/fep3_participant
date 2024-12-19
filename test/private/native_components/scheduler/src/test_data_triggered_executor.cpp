/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */
#include <fep3/components/scheduler/mock/sceduled_task_mock.h>
#include <fep3/native_components/scheduler/clock_based/data_triggered_executor.h>

#include <boost/thread/latch.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace ::testing;
using namespace fep3::native;
using namespace fep3::mock;

TEST(TestDataTriggeredExecutor, post__TaskAfterStartNotBlocking__successful)
{
    const size_t num_of_tasks = 5;
    std::array<bool, num_of_tasks> tasks_ready{false};

    ThreadPoolExecutor thread_pool(num_of_tasks);
    DataTriggeredExecutor executor(thread_pool);
    thread_pool.start();

    boost::barrier barrier(num_of_tasks + 1);
    boost::latch latch(num_of_tasks);

    executor.start();
    for (auto i = 0; i < 5; ++i) {
        executor.post([&, i]() {
            barrier.count_down_and_wait();
            tasks_ready[i] = true;
            latch.count_down();
        });
    }

    EXPECT_THAT(tasks_ready, Each(Eq(false)));
    barrier.count_down_and_wait();
    latch.wait();
    EXPECT_THAT(tasks_ready, Each(Eq(true)));
}

TEST(TestDataTriggeredExecutor, post__TaskAfterStop__failed)
{
    bool task_ready{false};
    MockThreadPoolExecutor thread_pool;
    DataTriggeredExecutor executor(thread_pool);
    boost::barrier barrier(2);
    boost::latch latch(1);

    executor.start();
    executor.post([&]() { task_ready = true; });
    EXPECT_TRUE(task_ready);

    // reset the flag and stop the executor
    executor.stop();
    task_ready = false;
    executor.post([&]() { task_ready = true; });
    EXPECT_FALSE(task_ready);
}
