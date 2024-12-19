/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "tester_health_service_common.h"

#include <fep3/components/health_service/mock_health_service.h>

using namespace ::testing;
using namespace fep3;

TEST_F(HealthServiceTest, iterateStates)
{
    _component_registry->initialize();

    {
        EXPECT_CALL(*_job_registry_mock, getJobsCatelyn()).WillOnce(::testing::Return(_jobs));
        EXPECT_CALL(*_job_health_registry_mock, initialize(fep3::mock::JobsMatcher(_jobs), _));
        ASSERT_FEP3_NOERROR(_component_registry->tense());
        ASSERT_FEP3_NOERROR(_component_registry->start());
    }

    {
        ASSERT_FEP3_NOERROR(_component_registry->stop());
        EXPECT_CALL(*_job_health_registry_mock, deinitialize());
        ASSERT_FEP3_NOERROR(_component_registry->relax());
    }
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

TEST_F(HealthServiceTest, ResetHealth)
{
    ASSERT_NO_FATAL_FAILURE(startComponents());

    EXPECT_CALL(*_job_health_registry_mock, resetHealth())
        .WillOnce(::testing::Return(fep3::Result{}));

    auto health_service_intf = getHealthService();
    ASSERT_NE(health_service_intf, nullptr);
    health_service_intf->resetHealth();
    ASSERT_NO_FATAL_FAILURE(stopComponents());
}

TEST_F(HealthServiceTest, updateJobStatus)
{
    using namespace std::chrono_literals;

    ASSERT_NO_FATAL_FAILURE(startComponents());

    const std::string job_name = "job_name";
    const auto err_message_data_in = CREATE_ERROR_DESCRIPTION(-32, "some error data in");
    const auto err_message_data_out = CREATE_ERROR_DESCRIPTION(-21, "some error data out");

    EXPECT_CALL(
        *_job_health_registry_mock,
        updateJobStatus(StrEq(job_name),
                        fep3::mock::JobExecuteResultMatcher(fep3::IHealthService::JobExecuteResult{
                            100ns, err_message_data_in, {}, err_message_data_out})))
        .WillOnce(::testing::Return(fep3::Result{}));

    auto health_service_intf = getHealthService();
    ASSERT_NE(health_service_intf, nullptr);

    using namespace std::chrono_literals;

    health_service_intf->updateJobStatus(job_name,
                                         {100ns, err_message_data_in, {}, err_message_data_out});
    ASSERT_NO_FATAL_FAILURE(stopComponents());
}
