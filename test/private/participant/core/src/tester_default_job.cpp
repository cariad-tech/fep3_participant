/**
 * Copyright @ 2023 VW Group. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/components/base/mock_components.h>
#include <fep3/components/job_registry/mock_job.h>
#include <fep3/core.h>
#include <fep3/participant/mock/mock_data_io_container.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <common/gtest_asserts.h>
#include <helper/gmock_async_helper.h>

using namespace ::testing;
using namespace std::chrono_literals;

using Components = NiceMock<fep3::mock::Components>;

class MyJob : public fep3::core::DefaultJob {
public:
    MyJob() : fep3::core::DefaultJob("job_name")
    {
    }

    void createDataIOs(const fep3::arya::IComponents&,
                       fep3::core::IDataIOContainer& io_container,
                       const fep3::catelyn::JobConfiguration& job_config) override
    {
        io_container.addDataIn("signal_msg", fep3::base::StreamTypeString(), 1, job_config);
    }

    fep3::Result execute(fep3::Timestamp) override
    {
        return {};
    }
};

struct DefaultJobTest : public Test {
protected:
    void SetUp() override
    {
    }

    MyJob job = MyJob();
    std::shared_ptr<Components> components = std::make_shared<Components>();
};

using MyDataIOContainer = NiceMock<fep3::mock::DataIOContainer>;

TEST_F(DefaultJobTest, getName_successful)
{
    ASSERT_EQ(job.getName(), "job_name");
}

TEST_F(DefaultJobTest, executeDataIn_successful)
{
    auto io_container = std::make_shared<MyDataIOContainer>();
    job.setDataIOContainer(*io_container);
    EXPECT_CALL(*io_container, executeDataIn(_)).WillOnce(Return(fep3::Result{}));
    ASSERT_FEP3_NOERROR(job.executeDataIn(fep3::Timestamp{0}));
}

TEST_F(DefaultJobTest, execute_successful)
{
    auto io_container = std::make_shared<MyDataIOContainer>();
    job.setDataIOContainer(*io_container);
    ASSERT_FEP3_NOERROR(job.execute(fep3::Timestamp{0}));
}

TEST_F(DefaultJobTest, executeDataOut_successful)
{
    auto io_container = std::make_shared<MyDataIOContainer>();
    job.setDataIOContainer(*io_container);
    EXPECT_CALL(*io_container, executeDataOut(_)).WillOnce(Return(fep3::Result{}));
    ASSERT_FEP3_NOERROR(job.executeDataOut(fep3::Timestamp{0}));
}

TEST_F(DefaultJobTest, initialize_successful)
{
    ASSERT_FEP3_NOERROR(job.initialize(*components));
}

TEST_F(DefaultJobTest, start)
{
    ASSERT_FEP3_NOERROR(job.start());
}

TEST_F(DefaultJobTest, stop)
{
    ASSERT_FEP3_NOERROR(job.stop());
}

TEST_F(DefaultJobTest, createIOs_successful)
{
    auto io_container = std::make_shared<MyDataIOContainer>();
    job.setDataIOContainer(*io_container);

    EXPECT_CALL(
        *io_container,
        addDataIn("signal_msg",
                  _,
                  1,
                  Matcher<const fep3::JobConfiguration&>{fep3::mock::JobConfigurationMatcher(
                      fep3::ClockTriggeredJobConfiguration(100ms))}))
        .Times(1);
    job.createDataIOs(*components, *io_container, fep3::ClockTriggeredJobConfiguration(100ms));
}
