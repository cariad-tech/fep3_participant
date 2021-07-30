/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */


#include <scenario/scenario_fixtures.h>
#include <helper/gmock_async_helper.h>

using namespace fep3;
using namespace cpp;
using namespace std::literals::chrono_literals;
using namespace ::testing;
using namespace fep3::test::scenario;

struct MyCoreJob_50ms : NiceMock<mock::core::Job>
{
    MyCoreJob_50ms() : NiceMock<Job>("core_job_50ms", Duration{50ms})
    {
        setDefaultBehaviour();
    }
};

template <class T>
struct MyMasterSlaveSystem
    : public T
{
    void SetUp() override
    {
        T::SetUp();

        auto slave_clock = T::getWrapper("test_timing_slave")->clock_event_sink;
        auto master_clock = T::getWrapper("test_timing_master")->clock_event_sink;

        EXPECT_CALL(*slave_clock, timeResetBegin(_,_)).Times(T::_number_of_time_resets);
        EXPECT_CALL(*slave_clock, timeResetEnd(_)).Times(T::_number_of_time_resets);

        EXPECT_CALL(*master_clock, timeResetBegin(_,_)).Times(1);
        EXPECT_CALL(*master_clock, timeResetEnd(_)).Times(1);
    }

    virtual std::vector<std::shared_ptr<Participant>> createParticipants() const override
    {
        const std::string master_name {"test_timing_master"};
        const std::string slave_name {"test_timing_slave"};

        auto master = std::make_shared<Participant>(cpp::createParticipant<MyElement<MyCoreJob_50ms>>(
            master_name, T::_system_name));

        auto slave = std::make_shared<Participant>(cpp::createParticipant<MyElement<MyCoreJob_50ms>>(
            slave_name, T::_system_name));

        return  { slave, master };
    }
};

using MyDiscreteSystem = MyMasterSlaveSystem<MasterSlaveSystemDiscrete>;
using MyContinuousSystem = MyMasterSlaveSystem<MasterSlaveSystemContinuous>;

TEST_F(MyDiscreteSystem, twoParticipantsSynchronizedDiscrete)
{
    Initialized();

    const auto core_job = getWrapper("test_timing_slave")->getJob("core_job_50ms");
    const auto mock_job = dynamic_cast<MyCoreJob_50ms*>(core_job);
    ASSERT_TRUE(mock_job);

    auto done = std::make_shared<::test::helper::Notification>();

    EXPECT_CALL(*mock_job, execute(_)).Times(AnyNumber());
    EXPECT_CALL(*mock_job, execute(fep3::Timestamp(0))).Times(1);
    EXPECT_CALL(*mock_job, execute(fep3::Timestamp(50ms))).Times(1);
    EXPECT_CALL(*mock_job, execute(fep3::Timestamp(100ms))).Times(1);
    EXPECT_CALL(*mock_job, execute(fep3::Timestamp(150ms))).WillOnce(DoAll(
        Notify(done),
        Return(ERR_NOERROR)));

    Running();

    ASSERT_TRUE(done->waitForNotificationWithTimeout(std::chrono::seconds(5)));

    Initialized();
}

TEST_F(MyContinuousSystem, twoParticipantsSynchronizedContinuous)
{
    Initialized();

    const auto core_job = getWrapper("test_timing_slave")->getJob("core_job_50ms");
    const auto mock_job = dynamic_cast<MyCoreJob_50ms*>(core_job);
    ASSERT_TRUE(mock_job);

    auto done_slave = std::make_shared<::test::helper::Notification>();

    auto first_slave_timestamp = std::numeric_limits<int64_t>::max();

    EXPECT_CALL(*mock_job, execute(_)).Times(AnyNumber()).WillRepeatedly(
        Invoke([&first_slave_timestamp](Timestamp timestamp) {
            first_slave_timestamp = std::min(first_slave_timestamp, std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count());
        return Result{};
        }));
    EXPECT_CALL(*mock_job, execute(Gt(fep3::Timestamp(1500ms)))).WillRepeatedly(DoAll(
        Notify(done_slave),
        Invoke([&first_slave_timestamp](Timestamp timestamp) {
            first_slave_timestamp = std::min(first_slave_timestamp, std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count());
        }),
        Return(ERR_NOERROR)));

    const auto core_job_m = getWrapper("test_timing_master")->getJob("core_job_50ms");
    const auto mock_job_m = dynamic_cast<MyCoreJob_50ms*>(core_job_m);
    ASSERT_TRUE(mock_job_m);

    auto done_master = std::make_shared<::test::helper::Notification>();

    auto first_master_timestamp = std::numeric_limits<int64_t>::max();

    EXPECT_CALL(*mock_job_m, execute(_)).Times(AnyNumber()).WillRepeatedly(
        Invoke([&first_master_timestamp](Timestamp timestamp) {
            first_master_timestamp = std::min(first_master_timestamp, std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count());
            return Result{};
        }));
    EXPECT_CALL(*mock_job_m, execute(Gt(fep3::Timestamp(1500ms)))).WillRepeatedly(DoAll(
        Notify(done_master),
        Invoke([&first_master_timestamp](Timestamp timestamp) {
            first_master_timestamp = std::min(first_master_timestamp, std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count());
        }),
        Return(ERR_NOERROR)));

    Running();

    ASSERT_TRUE(done_slave->waitForNotificationWithTimeout(std::chrono::seconds(5)));
    ASSERT_TRUE(done_master->waitForNotificationWithTimeout(std::chrono::seconds(5)));

    EXPECT_LE(std::max(first_master_timestamp, first_slave_timestamp), 200);

    Initialized();
}
