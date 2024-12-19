/**
 * Copyright 2024 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/components/service_bus/mock_service_bus.h>
#include <fep3/native_components/logging/sinks/registered_rpc_sink_services.h>

#include <gtest/gtest.h>
using namespace ::testing;
using namespace fep3::native;

struct RegisteredRpcSinkServicesTest : public ::testing::Test {
    std::shared_ptr<fep3::IRPCRequester> createRequester(const std::string&)
    {
        using RPCRequester = StrictMock<fep3::mock::RPCRequester>;
        return std::make_shared<RPCRequester>();
    }

    auto getRequesterFactory()
    {
        return [this](const std::string address) { return createRequester(address); };
    }

    template <typename... T>
    std::queue<SinkRequest> getRequests(T&&... requests)
    {
        std::queue<SinkRequest> ret;

        (ret.emplace(std::forward<T>(requests)), ...);

        return ret;
    }

    fep3::native::RegisteredRpcSinkServices _request_handler;
};

TEST_F(RegisteredRpcSinkServicesTest, processRequests_sinksRegistered)
{
    RegisterSinkRequest register_request_1 = {"address1", "filter1", 3};
    RegisterSinkRequest register_request_2 = {"address3", "filter2", 2};

    _request_handler.processRequests(getRequests(register_request_1, register_request_2),
                                     getRequesterFactory());

    ASSERT_THAT(
        _request_handler.getRegisteredSinkClients(),
        UnorderedElementsAreArray(
            {Pair("address1",
                  AllOf(Field(&ClientFilter::_name_filter, "filter1"),
                        Field(&ClientFilter::_severity_filter, fep3::LoggerSeverity::warning))),
             Pair("address3",
                  AllOf(Field(&ClientFilter::_name_filter, "filter2"),
                        Field(&ClientFilter::_severity_filter, fep3::LoggerSeverity::error)))}));
}

TEST_F(RegisteredRpcSinkServicesTest, processRequests_registerTwice_noError)
{
    RegisterSinkRequest register_request_1 = {"address1", "filter1", 3};
    RegisterSinkRequest register_request_2 = {"address3", "filter2", 2};

    _request_handler.processRequests(
        getRequests(register_request_1, register_request_2, register_request_2),
        getRequesterFactory());

    ASSERT_THAT(_request_handler.getRegisteredSinkClients(), SizeIs(2));
}

TEST_F(RegisteredRpcSinkServicesTest, processRequests_unRegisterNotExisting_noError)
{
    RegisterSinkRequest register_request_1 = {"address1", "filter1", 3};
    RegisterSinkRequest register_request_2 = {"address3", "filter2", 2};
    UnRegisterSinkRequest register_request_3 = {"addressNotExisting"};

    ASSERT_NO_THROW(_request_handler.processRequests(
        getRequests(register_request_1, register_request_2, register_request_3),
        getRequesterFactory()));
}

TEST_F(RegisteredRpcSinkServicesTest, processRequests_registerAndThenUnregister_sinkNotRegistered)
{
    RegisterSinkRequest register_request_1 = {"address1", "filter1", 3};
    RegisterSinkRequest register_request_2 = {"address3", "filter2", 2};
    UnRegisterSinkRequest register_request_3 = {"address1"};

    _request_handler.processRequests(
        getRequests(register_request_1, register_request_2, register_request_3),
        getRequesterFactory());

    ASSERT_THAT(_request_handler.getRegisteredSinkClients(),
                UnorderedElementsAreArray({Pair(
                    "address3",
                    AllOf(Field(&ClientFilter::_name_filter, "filter2"),
                          Field(&ClientFilter::_severity_filter, fep3::LoggerSeverity::error)))}));
}

TEST_F(RegisteredRpcSinkServicesTest, processRequests_registerUnregisterRegister_sinkRegistered)
{
    RegisterSinkRequest register_request_1 = {"address1", "filter1", 3};
    RegisterSinkRequest register_request_2 = {"address3", "filter2", 2};
    UnRegisterSinkRequest register_request_3 = {"address1"};
    RegisterSinkRequest register_request_4 = {"address1", "filter1", 3};

    _request_handler.processRequests(
        getRequests(register_request_1, register_request_2, register_request_3, register_request_4),
        getRequesterFactory());

    ASSERT_THAT(
        _request_handler.getRegisteredSinkClients(),
        UnorderedElementsAreArray(
            {Pair("address1",
                  AllOf(Field(&ClientFilter::_name_filter, "filter1"),
                        Field(&ClientFilter::_severity_filter, fep3::LoggerSeverity::warning))),
             Pair("address3",
                  AllOf(Field(&ClientFilter::_name_filter, "filter2"),
                        Field(&ClientFilter::_severity_filter, fep3::LoggerSeverity::error)))}));
}