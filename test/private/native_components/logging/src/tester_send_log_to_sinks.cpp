/**
 * Copyright 2024 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/native_components/logging/sinks/send_log_to_rpc_sink.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <gtest_asserts.h>

using namespace ::testing;
using namespace fep3::native;

struct MockRpcClient {
    MOCK_METHOD(int,
                onLog,
                (const std::string& description,
                 const std::string& logger_name,
                 const std::string& participant,
                 int severity,
                 const std::string& timestamp),
                ());
};

struct FaceClientFilter {
    std::unique_ptr<MockRpcClient> _client = std::make_unique<NiceMock<MockRpcClient>>();
};

struct SendLogToSinksTest : public ::testing::Test {
    SendLogToSinksTest()
    {
        _client_filters.emplace("Good1", FaceClientFilter{});
        _client_filters.emplace("Good2", FaceClientFilter{});
        _client_filters.emplace("Bad1", FaceClientFilter{});
        _client_filters.emplace("Good3", FaceClientFilter{});
        _client_filters.emplace("Bad2", FaceClientFilter{});
        _client_filters.emplace("Bad3", FaceClientFilter{});
    }

    std::map<std::string, FaceClientFilter> _client_filters;
};

TEST_F(SendLogToSinksTest, sendLogToSinks_failedRequests_returnsErrorAndFailedRequesterNames)
{
    ON_CALL(*(_client_filters["Good1"]._client), onLog).WillByDefault(Return(0));
    ON_CALL(*(_client_filters["Good2"]._client), onLog).WillByDefault(Return(0));
    ON_CALL(*(_client_filters["Good3"]._client), onLog).WillByDefault(Return(0));
    ON_CALL(*(_client_filters["Bad1"]._client), onLog).WillByDefault(Return(-3));
    ON_CALL(*(_client_filters["Bad2"]._client), onLog).WillByDefault(Return(-3));
    ON_CALL(*(_client_filters["Bad3"]._client), onLog).WillByDefault(Return(-3));

    auto [res, failed_names] = sendLogToSinks(fep3::LogMessage{}, _client_filters);

    ASSERT_FEP3_RESULT(res, fep3 ::Result{fep3::ERR_UNEXPECTED});
    ASSERT_THAT(failed_names, UnorderedElementsAreArray({Eq("Bad1"), Eq("Bad2"), Eq("Bad3")}));
}

TEST_F(SendLogToSinksTest, sendLogToSinks_throwRequests_returnsErrorAndFailedRequesterNames)
{
    ON_CALL(*(_client_filters["Good1"]._client), onLog).WillByDefault(Return(0));
    ON_CALL(*(_client_filters["Good2"]._client), onLog).WillByDefault(Return(0));
    ON_CALL(*(_client_filters["Good3"]._client), onLog).WillByDefault(Return(0));
    ON_CALL(*(_client_filters["Bad1"]._client), onLog).WillByDefault(Return(0));
    ON_CALL(*(_client_filters["Bad2"]._client), onLog).WillByDefault(Return(0));
    ON_CALL(*(_client_filters["Bad3"]._client), onLog).WillByDefault(Throw(std::runtime_error{""}));

    auto [res, failed_names] = sendLogToSinks(fep3::LogMessage{}, _client_filters);

    ASSERT_FALSE(res);
    ASSERT_THAT(failed_names, UnorderedElementsAreArray({Eq("Bad3")}));
}

TEST_F(SendLogToSinksTest, sendLogToSinks_onLog_logMessageForwarded)
{
    fep3::LogMessage message{"_timestamp",
                             fep3::arya::LoggerSeverity::error,
                             "participant_name",
                             "_logger_name",
                             "message"};

    EXPECT_CALL(
        *(_client_filters["Good1"]._client),
        onLog(message._message,
              message._logger_name,
              message._participant_name,
              static_cast<std::underlying_type_t<fep3::arya::LoggerSeverity>>(message._severity),
              message._timestamp))
        .WillOnce(Return(0));

    sendLogToSinks(message, _client_filters);
}
