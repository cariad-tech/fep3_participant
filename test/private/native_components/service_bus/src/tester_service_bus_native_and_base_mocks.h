/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/service_bus/system_access_base.hpp>
#include <fep3/native_components/service_bus/rpc/service_discovery/service_discovery_intf/service_discovery_factory_intf.h>

#include <gmock/gmock.h>

struct SystemAccessDefaultUrlsMock
    : public fep3::base::arya::SystemAccessBase::ISystemAccessBaseDefaultUrls {
    MOCK_METHOD(std::string, getDefaultSystemUrl, (), (const, override));
    MOCK_METHOD(std::string, getDefaultServerUrl, (), (const, override));
};

struct ServiceDiscoveryFactoryMock : public fep3::native::IServiceDiscoveryFactory {
    using url_pair = std::pair<std::string, bool>;
    MOCK_METHOD(std::unique_ptr<fep3::native::IServiceDiscovery>,
                getServiceDiscovery,
                (std::string,
                 std::string,
                 std::chrono::seconds,
                 url_pair,
                 std::string,
                 std::string,
                 std::string,
                 std::string,
                 std::string,
                 std::string),
                (const, override));
    MOCK_METHOD(std::unique_ptr<fep3::native::IServiceFinder>,
                getServiceFinder,
                (std::shared_ptr<fep3::ILogger>,
                 const std::string&,
                 const std::string&,
                 const std::string&,
                 const std::string&,
                 const std::string&),
                (const, override));
};

struct ServiceFinderMock : public fep3::native::IServiceFinder {
    MOCK_METHOD(std::string, getLastSendErrors, (), (const, override));
    MOCK_METHOD(bool, sendMSearch, (), (override));
    MOCK_METHOD(bool,
                checkForServices,
                (const std::function<void(const fep3::native::ServiceUpdateEvent&)>&,
                 std::chrono::milliseconds),
                (override));
    MOCK_METHOD(void, disableDiscovery, (), (override));
};

struct ServiceDiscoveryMock : public fep3::native::IServiceDiscovery {
    MOCK_METHOD(std::string, getLastSendErrors, (), (const, override));
    MOCK_METHOD(bool, sendNotifyAlive, (), (override));
    MOCK_METHOD(bool, sendNotifyByeBye, (), (override));
    MOCK_METHOD(bool, checkForMSearchAndSendResponse, (std::chrono::milliseconds), (override));
};
