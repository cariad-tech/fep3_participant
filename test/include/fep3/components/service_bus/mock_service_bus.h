/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/base/component.h>
#include <fep3/components/service_bus/service_bus_intf.h>

#include <gmock/gmock.h>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace arya {

struct RPCRequester : fep3::arya::IRPCRequester {
    struct RPCResponse : IRPCResponse {
        MOCK_METHOD(fep3::Result, set, (const std::string& response), (override));
    };

    MOCK_METHOD(fep3::Result,
                sendRequest,
                (const std::string& service_name,
                 const std::string& request_message,
                 IRPCResponse& response_callback),
                (const, override));
};

struct RPCServer : fep3::arya::IRPCServer {
    struct RPCService : IRPCService {
        MOCK_METHOD(std::string, getRPCServiceIIDs, (), (const, override));
        MOCK_METHOD(std::string, getRPCInterfaceDefinition, (), (const, override));
        MOCK_METHOD(fep3::Result,
                    handleRequest,
                    (const std::string& content_type,
                     const std::string& request_message,
                     fep3::arya::IRPCRequester::IRPCResponse&),
                    (override));
    };
    MOCK_METHOD(std::string, getUrl, (), (const, override));
    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(void, setName, (const std::string&), (override));
    MOCK_METHOD(fep3::Result,
                registerService,
                (const std::string&, const std::shared_ptr<IRPCService>&),
                (override));
    MOCK_METHOD(fep3::Result, unregisterService, (const std::string&), (override));
};

struct ServiceBus : public fep3::base::arya::Component<fep3::arya::IServiceBus> {
    MOCK_METHOD(fep3::Result,
                createSystemAccess,
                (const std::string&, const std::string&, bool),
                (override));
    MOCK_METHOD(fep3::Result, releaseSystemAccess, (const std::string&), (override));
    MOCK_METHOD(std::shared_ptr<IParticipantServer>, getServer, (), (const, override));
    MOCK_METHOD(std::shared_ptr<IParticipantRequester>,
                getRequester,
                (const std::string&),
                (const, override));
    MOCK_METHOD(std::shared_ptr<fep3::arya::IServiceBus::ISystemAccess>,
                getSystemAccess,
                (const std::string&),
                (const, override));
    MOCK_METHOD(std::shared_ptr<IParticipantRequester>,
                getRequester,
                (const std::string&, bool),
                (const, override));
};

} // namespace arya
using arya::RPCRequester;
using arya::RPCServer;

namespace catelyn {

struct ServiceBus : public fep3::base::arya::Component<fep3::catelyn::IServiceBus> {
    MOCK_METHOD(fep3::Result,
                createSystemAccess,
                (const std::string&, const std::string&, bool),
                (override));
    MOCK_METHOD(fep3::Result, releaseSystemAccess, (const std::string&), (override));
    MOCK_METHOD(std::shared_ptr<IParticipantServer>, getServer, (), (const, override));
    MOCK_METHOD(std::shared_ptr<IParticipantRequester>,
                getRequester,
                (const std::string&),
                (const, override));
    MOCK_METHOD(std::shared_ptr<ISystemAccess>,
                getSystemAccessCatelyn,
                (const std::string&),
                (const, override));
    MOCK_METHOD(std::shared_ptr<IParticipantRequester>,
                getRequester,
                (const std::string&, bool),
                (const, override));
    MOCK_METHOD(std::shared_ptr<fep3::arya::IServiceBus::ISystemAccess>,
                getSystemAccess,
                (const std::string&),
                (const, override));
};

struct ServiceUpdateEventSink : public fep3::IServiceBus::IServiceUpdateEventSink {
    MOCK_METHOD(void, updateEvent, (const fep3::IServiceBus::ServiceUpdateEvent&), (override));
};

} // namespace catelyn
using catelyn::ServiceBus;
using catelyn::ServiceUpdateEventSink;
} // namespace mock
} // namespace fep3

///@endcond nodoc