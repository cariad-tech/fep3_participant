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

#pragma warning(push)
#pragma warning(disable : 4290)
#pragma warning(pop)

#include "tester_health_service_common.h"

#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/native_components/service_bus/testing/service_bus_testing.hpp>

#include <test_health_proxy_stub.h>

using namespace fep3;
using namespace ::testing;
using namespace a_util::strings;
using namespace std::chrono_literals;

class TestClient : public fep3::rpc::RPCServiceClient<::test::rpc_stubs::TestHealthServiceProxy,
                                                      fep3::rpc::catelyn::IRPCHealthServiceDef> {
private:
    using base_type = fep3::rpc::RPCServiceClient<::test::rpc_stubs::TestHealthServiceProxy,
                                                  fep3::rpc::catelyn::IRPCHealthServiceDef>;

public:
    using base_type::GetStub;

    TestClient(const char* server_object_name, std::shared_ptr<fep3::IRPCRequester> rpc)
        : base_type(server_object_name, rpc)
    {
    }
};

struct NativeHealthServiceRPC : public HealthServiceTest {
    using JobHealthiness = fep3::native::IJobHealthRegistry::JobHealthiness;
    using ExecuteError = fep3::native::IJobHealthRegistry::JobHealthiness::ExecuteError;

    NativeHealthServiceRPC() : _service_bus{std::make_shared<fep3::native::ServiceBus>()}
    {
    }

    void SetUp() override
    {
        _health_service = std::make_shared<fep3::native::HealthService>();
        _job_health_registry_mock = nullptr;
        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(
            _service_bus, _dummy_component_version_info));
        HealthServiceTest::SetUp();

        _client = std::make_unique<TestClient>(
            fep3::rpc::catelyn::IRPCHealthServiceDef::getRPCDefaultName(),
            _service_bus->getRequester(fep3::native::testing::participant_name_default));

        _job_cycle_times = {{"job1", 100ns}, {"job2", 10ns}};
        _job_trigger_signals = {{"job3", "signal_name"}};
    }

    void checkLastErrorStruct(const Json::Value last_execution_error,
                              const ExecuteError& expected_last_execution_error)
    {
        ASSERT_EQ(last_execution_error["simulation_timestamp"].asInt64(),
                  expected_last_execution_error.simulation_time.count());
        ASSERT_EQ(last_execution_error["error_count"].asUInt64(),
                  expected_last_execution_error.error_count);

        ASSERT_EQ(last_execution_error["last_error"]["error_code"].asInt(),
                  expected_last_execution_error.last_error.getErrorCode());
        if (expected_last_execution_error.last_error.getErrorCode() ==
            fep3::ERR_NOERROR.getCode()) {
            ASSERT_EQ(last_execution_error["last_error"]["description"].asString(),
                      fep3::Result{fep3::ERR_NOERROR}.getDescription());
        }
        else {
            ASSERT_EQ(last_execution_error["last_error"]["description"].asString(),
                      expected_last_execution_error.last_error.getDescription());
        }
    }

    void checkHealthinessJsonArray(const std::string& job_name,
                                   fep3::Timestamp simulation_time,
                                   ExecuteError execute_data_in_error,
                                   ExecuteError execute_error,
                                   ExecuteError execute_data_out_error,
                                   JobType job_type = JobType::ClockTriggered)
    {
        auto json_value = _client->getHealth();

        auto json_value_array = json_value["jobs_healthiness"];
        ASSERT_TRUE(json_value_array.isArray());
        for (auto json_value_job_health = json_value_array.begin();
             json_value_job_health != json_value_array.end();
             ++json_value_job_health) {
            if ((*json_value_job_health)["job_name"].asString() == job_name) {
                switch (job_type) {
                case JobType::ClockTriggered:
                    ASSERT_EQ((*json_value_job_health)["cycle_time"].asInt64(),
                              _job_cycle_times[job_name].count());
                    break;
                case JobType::DataTriggered:
                    ASSERT_TRUE((*json_value_job_health)["trigger_signals"].isArray());
                    ASSERT_EQ((*json_value_job_health)["trigger_signals"].size(),
                              _job_trigger_signals.size());
                    ASSERT_EQ((*json_value_job_health)["trigger_signals"][0].asString(),
                              _job_trigger_signals[job_name]);
                    break;
                default:
                    break;
                }

                ASSERT_EQ((*json_value_job_health)["simulation_timestamp"].asInt64(),
                          simulation_time.count());
                ASSERT_NO_FATAL_FAILURE(checkLastErrorStruct(
                    (*json_value_job_health)["last_execute_data_in_error"], execute_data_in_error));
                ASSERT_NO_FATAL_FAILURE(checkLastErrorStruct(
                    (*json_value_job_health)["last_execute_error"], execute_error));
                ASSERT_NO_FATAL_FAILURE(
                    checkLastErrorStruct((*json_value_job_health)["last_execute_data_out_error"],
                                         execute_data_out_error));
                return;
            }
        }
        FAIL() << "Job " << job_name << " not found in json";
    }

    std::shared_ptr<fep3::native::ServiceBus> _service_bus{};
    std::unique_ptr<TestClient> _client;
    std::map<std::string, fep3::Timestamp> _job_cycle_times;
    std::map<std::string, std::string> _job_trigger_signals;
};

// The RPC has to return valid empty value when no jobs are registered
TEST_F(NativeHealthServiceRPC, testEmptyJobHealthiness)
{
    clearJobs();
    ASSERT_NO_FATAL_FAILURE(startComponents());
    auto json_value = _client->getHealth();
    ASSERT_TRUE(json_value.empty());
    ASSERT_NO_FATAL_FAILURE(stopComponents());
}

TEST_F(NativeHealthServiceRPC, testJobHealthinessUpdate)
{
    ASSERT_NO_FATAL_FAILURE(startComponents());

    const auto health_service_intf = getHealthService();
    ASSERT_NE(health_service_intf, nullptr);

    for (uint8_t i = 0; i < 11; ++i) {
        ASSERT_FEP3_NOERROR(health_service_intf->updateJobStatus(
            "job1",
            {std::chrono::nanoseconds(100 * i), fep3::Result{}, fep3::Result{}, fep3::Result{}}));
        ASSERT_FEP3_NOERROR(health_service_intf->updateJobStatus(
            "job2",
            {std::chrono::nanoseconds(10 * i), fep3::Result{}, fep3::Result{}, fep3::Result{}}));
        ASSERT_FEP3_NOERROR(health_service_intf->updateJobStatus(
            "job3",
            {std::chrono::nanoseconds(10 * i), fep3::Result{}, fep3::Result{}, fep3::Result{}}));
    }

    ASSERT_NO_FATAL_FAILURE(checkHealthinessJsonArray("job1", 1000ns, {}, {}, {}));
    ASSERT_NO_FATAL_FAILURE(checkHealthinessJsonArray("job2", 100ns, {}, {}, {}));
    ASSERT_NO_FATAL_FAILURE(
        checkHealthinessJsonArray("job3", 100ns, {}, {}, {}, JobType::DataTriggered));

    for (uint8_t i = 11; i < 21; ++i) {
        ASSERT_FEP3_NOERROR(health_service_intf->updateJobStatus(
            "job1",
            {std::chrono::nanoseconds(100 * i), fep3::Result{}, fep3::Result{}, fep3::Result{}}));
    }

    ASSERT_NO_FATAL_FAILURE(checkHealthinessJsonArray("job1", 2000ns, {}, {}, {}));
    ASSERT_NO_FATAL_FAILURE(checkHealthinessJsonArray("job2", 100ns, {}, {}, {}));

    // make job return error
    auto execute_error_description = CREATE_ERROR_DESCRIPTION(-20, "Job1 went wrong");
    health_service_intf->updateJobStatus(
        "job1", {2100ns, fep3::Result{}, execute_error_description, fep3::Result{}});
    ASSERT_NO_FATAL_FAILURE(
        checkHealthinessJsonArray("job1", 2100ns, {}, {1, 2100ns, execute_error_description}, {}));
    ASSERT_NO_FATAL_FAILURE(checkHealthinessJsonArray("job2", 100ns, {}, {}, {}));

    // always last error is returned
    execute_error_description = CREATE_ERROR_DESCRIPTION(-22, "Job1 went wrong again");
    health_service_intf->updateJobStatus(
        "job1", {2200ns, fep3::Result{}, execute_error_description, fep3::Result{}});
    ASSERT_NO_FATAL_FAILURE(
        checkHealthinessJsonArray("job1", 2200ns, {}, {2, 2200ns, execute_error_description}, {}));
    ASSERT_NO_FATAL_FAILURE(checkHealthinessJsonArray("job2", 100ns, {}, {}, {}));

    // after a successful job execute, last error is kept
    health_service_intf->updateJobStatus(
        "job1", {std::chrono::nanoseconds(2300ns), fep3::Result{}, fep3::Result{}, fep3::Result{}});
    ASSERT_NO_FATAL_FAILURE(
        checkHealthinessJsonArray("job1", 2300ns, {}, {2, 2200ns, execute_error_description}, {}));
    ASSERT_NO_FATAL_FAILURE(checkHealthinessJsonArray("job2", 100ns, {}, {}, {}));

    // for a good measure test job2 also
    for (uint8_t i = 11; i < 21; ++i) {
        ASSERT_FEP3_NOERROR(health_service_intf->updateJobStatus(
            "job2",
            {std::chrono::nanoseconds(10 * i), fep3::Result{}, fep3::Result{}, fep3::Result{}}));
    }
    ASSERT_NO_FATAL_FAILURE(checkHealthinessJsonArray("job2", 200ns, {}, {}, {}));
    // make job return error
    execute_error_description = CREATE_ERROR_DESCRIPTION(-22, "Job2 went wrong");
    health_service_intf->updateJobStatus(
        "job2", {210ns, execute_error_description, fep3::Result{}, fep3::Result{}});
    ASSERT_NO_FATAL_FAILURE(
        checkHealthinessJsonArray("job2", 210ns, {1, 210ns, execute_error_description}, {}, {}));

    ASSERT_NO_FATAL_FAILURE(stopComponents());
}

TEST_F(NativeHealthServiceRPC, testResetHealth)
{
    ASSERT_NO_FATAL_FAILURE(startComponents());

    const auto health_service_intf = getHealthService();
    ASSERT_NE(health_service_intf, nullptr);

    for (uint8_t i = 0; i < 21; ++i) {
        ASSERT_FEP3_NOERROR(health_service_intf->updateJobStatus(
            "job1",
            {std::chrono::nanoseconds(100 * i), fep3::Result{}, fep3::Result{}, fep3::Result{}}));
        ASSERT_FEP3_NOERROR(health_service_intf->updateJobStatus(
            "job2",
            {std::chrono::nanoseconds(10 * i), fep3::Result{}, fep3::Result{}, fep3::Result{}}));
    }
    auto execute_error_description = CREATE_ERROR_DESCRIPTION(-20, "Job1 went wrong");
    health_service_intf->updateJobStatus(
        "job1", {2100ns, fep3::Result{}, execute_error_description, fep3::Result{}});
    ASSERT_NO_FATAL_FAILURE(
        checkHealthinessJsonArray("job1", 2100ns, {}, {1, 2100ns, execute_error_description}, {}));

    execute_error_description = CREATE_ERROR_DESCRIPTION(-22, "Job2 went wrong");
    health_service_intf->updateJobStatus(
        "job2", {210ns, fep3::Result{}, fep3::Result{}, execute_error_description});
    ASSERT_NO_FATAL_FAILURE(
        checkHealthinessJsonArray("job2", 210ns, {}, {}, {1, 210ns, execute_error_description}));

    health_service_intf->resetHealth();
    ASSERT_NO_FATAL_FAILURE(checkHealthinessJsonArray("job1", 2100ns, {}, {}, {}));
    ASSERT_NO_FATAL_FAILURE(checkHealthinessJsonArray("job2", 210ns, {}, {}, {}));

    ASSERT_NO_FATAL_FAILURE(stopComponents());
}
