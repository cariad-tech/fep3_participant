/**
 * Copyright 2023 CARIAD SE.
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
using MyDataIOContainer = NiceMock<fep3::mock::DataIOContainer>;

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
    std::shared_ptr<MyDataIOContainer> io_container = std::make_shared<MyDataIOContainer>();
};

struct DataIOContainerTest : public Test {
protected:
    void SetUp() override
    {
    }

    MyJob job = MyJob();
    std::shared_ptr<Components> components = std::make_shared<Components>();
    std::shared_ptr<fep3::core::DataIOContainer> io_container =
        std::make_shared<fep3::core::DataIOContainer>();
};

MATCHER_P(EqDataIOContainerConfiguration,
          other,
          "Equality matcher for type fep3::core::DataIOContainerConfiguration")
{
    return std::tie(arg.purged_samples_log_capacity, arg.clear_input_signal_queues) ==
           std::tie(other.purged_samples_log_capacity, other.clear_input_signal_queues);
}

TEST_F(DefaultJobTest, getName_successful)
{
    ASSERT_EQ(job.getName(), "job_name");
}

TEST_F(DefaultJobTest, executeDataIn_successful)
{
    job.setDataIOContainer(*io_container);
    EXPECT_CALL(*io_container, executeDataIn(_)).WillOnce(Return(fep3::Result{}));
    ASSERT_FEP3_NOERROR(job.executeDataIn(fep3::Timestamp{0}));
}

TEST_F(DefaultJobTest, execute_successful)
{
    job.setDataIOContainer(*io_container);
    ASSERT_FEP3_NOERROR(job.execute(fep3::Timestamp{0}));
}

TEST_F(DefaultJobTest, executeDataOut_successful)
{
    job.setDataIOContainer(*io_container);
    EXPECT_CALL(*io_container, executeDataOut(_)).WillOnce(Return(fep3::Result{}));
    ASSERT_FEP3_NOERROR(job.executeDataOut(fep3::Timestamp{0}));
}

TEST_F(DefaultJobTest, initialize_successful)
{
    ASSERT_FEP3_NOERROR(job.initialize(*components));
}

TEST_F(DefaultJobTest, start_successful)
{
    ASSERT_FEP3_NOERROR(job.start());
}

TEST_F(DefaultJobTest, stop_successful)
{
    ASSERT_FEP3_NOERROR(job.stop());
}

TEST_F(DefaultJobTest, deinitialize_successful)
{
    ASSERT_FEP3_NOERROR(job.deinitialize());
}

TEST_F(DefaultJobTest, createIOs_successful)
{
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

TEST_F(DefaultJobTest, createDefaultPropertyVariables__successful)
{
    ASSERT_FEP3_NOERROR(job.createDefaultPropertyVariables());
    const auto purged_sample_log_capacity_node =
        job.getNode()->getChild(FEP3_PURGED_SAMPLES_LOG_CAPACITY_PROPERTY);
    ASSERT_TRUE(purged_sample_log_capacity_node);
    EXPECT_EQ(purged_sample_log_capacity_node->getValue(),
              std::to_string(FEP3_PURGED_SAMPLES_LOG_CAPACITY_DEFAULT_VALUE));
    const auto clear_input_signal_queues_node =
        job.getNode()->getChild(FEP3_CLEAR_INPUT_SIGNALS_QUEUES_PROPERTY);
    ASSERT_TRUE(clear_input_signal_queues_node);

    EXPECT_EQ(clear_input_signal_queues_node->getValue(), "false");
}

TEST_F(DefaultJobTest, removeDefaultPropertyVariables__successful)
{
    ASSERT_FEP3_NOERROR(job.createDefaultPropertyVariables());
    ASSERT_FEP3_NOERROR(job.removeDefaultPropertyVariables());
}

TEST_F(DefaultJobTest, removeDefaultPropertyVariables__fail__propertyNoExisting)
{
    ASSERT_FEP3_RESULT(job.removeDefaultPropertyVariables(), fep3::Result(-20));
}

TEST_F(DefaultJobTest, logIOInfo__successful)
{
    job.setDataIOContainer(*io_container);

    EXPECT_CALL(*io_container, logIOInfo(_)).Times(1);
    job.logIOInfo();
}

TEST_F(DefaultJobTest, applyDefaultPropertyVariables__successful)
{
    job.setDataIOContainer(*io_container);
    ASSERT_FEP3_NOERROR(job.createDefaultPropertyVariables());

    fep3::core::DataIOContainerConfiguration configuration = {
        FEP3_PURGED_SAMPLES_LOG_CAPACITY_DEFAULT_VALUE, false};

    EXPECT_CALL(*io_container, setConfiguration(EqDataIOContainerConfiguration(configuration)))
        .Times(1);

    ASSERT_FEP3_NOERROR(job.applyDefaultPropertyVariables());
}

TEST_F(DefaultJobTest, applyDefaultPropertyVariables__failed__invalidValuePurgeCapacity)
{
    job.setDataIOContainer(*io_container);
    ASSERT_FEP3_NOERROR(job.createDefaultPropertyVariables());

    const auto purged_sample_log_capacity_node =
        job.getNode()->getChild(FEP3_PURGED_SAMPLES_LOG_CAPACITY_PROPERTY);
    ASSERT_TRUE(purged_sample_log_capacity_node);
    ASSERT_FEP3_NOERROR(purged_sample_log_capacity_node->setValue("-1"));
    job.updatePropertyVariables();

    EXPECT_CALL(*io_container, setConfiguration(_)).Times(0);
    ASSERT_FEP3_RESULT(job.applyDefaultPropertyVariables(), fep3::ERR_INVALID_ARG);
}