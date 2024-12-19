/**
 * Copyright 2024 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/native_components/logging/sinks/logging_sink_service_connection_strategy.h>
#include <fep3/native_components/logging/sinks/registered_rpc_sink_services.h>

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/range/adaptors.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;
using namespace fep3::native;

struct FakeConnectionStatus {
    template <typename UrlContainer>
    auto getUnreachable(const UrlContainer& urls)
    {
        return boost::adaptors::filter(
            urls, [&](const auto& x) { return boost::algorithm::any_of_equal(_unreachable, x); });
    }

    std::vector<std::string> _unreachable;
};

struct LogginkSinkConnectionHandleStrategyTest : public ::testing::Test {
    ClientFilter getFilter()
    {
        return ClientFilter{std::string("bla"), fep3::LoggerSeverity::debug, nullptr};
    }

    template <typename T>
    std::vector<T> toVector(std::queue<T>& q)
    {
        std::vector<T> v;
        while (!q.empty()) {
            v.push_back(q.front());
            q.pop();
        }

        return v;
    }
};

TEST_F(LogginkSinkConnectionHandleStrategyTest, checkConnectivity_removesUnreachable)
{
    std::vector<std::string> unreachable = {"Unreach1", "Unreach2", "Unreach3"};

    std::map<std::string, std::string> filters = {{"Unreach1", "Filter1"},
                                                  {"Unreach2", "Filter2"},
                                                  {"Unreach3", "Filter3"},
                                                  {"Reach1", "Filter4"},
                                                  {"Reach2", "Filter5"},
                                                  {"Reach3", "Filter6"}};
    FakeConnectionStatus connection_status{unreachable};
    LogginkSinkConnectionHandleStrategy strategy(connection_status);

    auto requests = strategy.checkConnectivity(filters);
    ASSERT_THAT(
        toVector(requests),
        ::testing::UnorderedElementsAreArray(
            {VariantWith<UnRegisterSinkRequest>(Field(&UnRegisterSinkRequest::address, "Unreach1")),
             VariantWith<UnRegisterSinkRequest>(Field(&UnRegisterSinkRequest::address, "Unreach2")),
             VariantWith<UnRegisterSinkRequest>(
                 Field(&UnRegisterSinkRequest::address, "Unreach3"))}));
}

TEST_F(LogginkSinkConnectionHandleStrategyTest, handleFailedTransmissions_removesFailedTransmitions)
{
    FakeConnectionStatus connection_status{};
    LogginkSinkConnectionHandleStrategy strategy(connection_status);
    std::vector<std::string> filters = {
        {"Unreach1"}, {"Unreach2"}, {"Unreach3"}, {"Reach1"}, {"Reach2"}, {"Reach3"}};

    auto requests = strategy.handleFailedTransmissions(filters);
    ASSERT_THAT(
        toVector(requests),
        ::testing::UnorderedElementsAreArray(
            {VariantWith<UnRegisterSinkRequest>(Field(&UnRegisterSinkRequest::address, "Unreach1")),
             VariantWith<UnRegisterSinkRequest>(Field(&UnRegisterSinkRequest::address, "Unreach2")),
             VariantWith<UnRegisterSinkRequest>(Field(&UnRegisterSinkRequest::address, "Unreach3")),
             VariantWith<UnRegisterSinkRequest>(Field(&UnRegisterSinkRequest::address, "Reach1")),
             VariantWith<UnRegisterSinkRequest>(Field(&UnRegisterSinkRequest::address, "Reach2")),
             VariantWith<UnRegisterSinkRequest>(
                 Field(&UnRegisterSinkRequest::address, "Reach3"))}));
}
