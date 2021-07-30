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


#include <gtest/gtest.h>
#include "common/gtest_asserts.h"

#include "fep3/components/base/component_registry.h"
#include "fep3/components/service_bus/rpc/fep_rpc.h"
#include "fep3/native_components/configuration/configuration_service.h"
#include "fep3/native_components/logging/logging_service.h"
#include "fep3/native_components/service_bus/service_bus.h"
#include "fep3/native_components/service_bus/testing/service_bus_testing.hpp"
#include "fep3/rpc_services/logging/logging_service_rpc_intf_def.h"
#include "fep3/rpc_services/logging/logging_client_stub.h"

typedef fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCLoggingClientStub, fep3::rpc::IRPCLoggingServiceDef> LoggingServiceClient;

struct TestLoggingService : public ::testing::Test
{
    std::shared_ptr<fep3::native::LoggingService> _logging{ std::make_shared<fep3::native::LoggingService>() };
    std::shared_ptr<fep3::native::ServiceBus> _service_bus{ std::make_shared<fep3::native::ServiceBus>() };
    std::shared_ptr<fep3::native::ConfigurationService> _configuration_service{ std::make_shared<fep3::native::ConfigurationService>() };
    std::shared_ptr<fep3::ComponentRegistry> _component_registry{ std::make_shared<fep3::ComponentRegistry>() };
    std::unique_ptr<LoggingServiceClient> _logging_service_client;

    TestLoggingService()
    {
    }

    void SetUp()
    {
        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(_service_bus));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(_logging));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(_configuration_service));
        ASSERT_FEP3_NOERROR(_component_registry->create());

        _logging_service_client = std::make_unique<LoggingServiceClient>(fep3::rpc::IRPCLoggingServiceDef::getRPCDefaultName(),
            _service_bus->getRequester(fep3::native::testing::participant_name_default));
    }

    void TearDown()
    {
        // The Service Bus object will be destroyed by the component registry during destruction

    }
};

/**
* Test the Logging Configuration
* @req_id ???
*/
TEST_F(TestLoggingService, TestLoggingConfiguration)
{
    // Actual test
    std::shared_ptr<fep3::ILogger> logger_tester = _logging->createLogger("Tester");
    std::shared_ptr<fep3::ILogger> logger_a = _logging->createLogger("LoggerA.Tester");
    std::shared_ptr<fep3::ILogger> logger_b = _logging->createLogger("LoggerB.Tester");
    std::shared_ptr<fep3::ILogger> logger_c = _logging->createLogger("LoggerC");

    // set default configuration
    ASSERT_NO_THROW(_logging_service_client->setLoggerFilter("", "", static_cast<int>(fep3::LoggerSeverity::fatal)));
    // all loggers should use the default if no other configuration exists
    ASSERT_TRUE(logger_a->isFatalEnabled());
    ASSERT_TRUE(logger_b->isFatalEnabled());
    ASSERT_TRUE(logger_c->isFatalEnabled());
    ASSERT_TRUE(logger_tester->isFatalEnabled());
    ASSERT_FALSE(logger_a->isErrorEnabled());
    ASSERT_FALSE(logger_b->isErrorEnabled());
    ASSERT_FALSE(logger_c->isErrorEnabled());
    ASSERT_FALSE(logger_tester->isErrorEnabled());

    ASSERT_NO_THROW(_logging_service_client->setLoggerFilter("", "LoggerA.Tester", static_cast<int>(fep3::LoggerSeverity::error)));
    // all loggers except for A should still use the default
    ASSERT_TRUE(logger_a->isErrorEnabled());
    ASSERT_FALSE(logger_b->isErrorEnabled());
    ASSERT_FALSE(logger_c->isErrorEnabled());
    ASSERT_FALSE(logger_tester->isErrorEnabled());

    ASSERT_NO_THROW(_logging_service_client->setLoggerFilter("", "Tester", static_cast<int>(fep3::LoggerSeverity::warning)));
    // logger A and B should be set too
    ASSERT_TRUE(logger_a->isWarningEnabled());
    ASSERT_TRUE(logger_b->isWarningEnabled());
    ASSERT_FALSE(logger_c->isWarningEnabled());
    ASSERT_TRUE(logger_tester->isWarningEnabled());
}

/**
* Test the default configuration if set neither by RPC or properties
* @req_id ???
*/
TEST_F(TestLoggingService, TestDefaultConfiguration)
{
    Json::Value default_filter = _logging_service_client->getLoggerFilter("");
    EXPECT_EQ(default_filter["enable_sinks"].asString(), "console,rpc");
    EXPECT_EQ(static_cast<fep3::LoggerSeverity>(default_filter["severity"].asInt()), fep3::LoggerSeverity::info);

    // The default stays the same after initialization
    _component_registry->initialize();

    default_filter = _logging_service_client->getLoggerFilter("");
    EXPECT_EQ(default_filter["enable_sinks"].asString(), "console,rpc");
    EXPECT_EQ(static_cast<fep3::LoggerSeverity>(default_filter["severity"].asInt()), fep3::LoggerSeverity::info);
}

/**
* Test the default configuration if set by RPC only before initialization
* @req_id ???
*/
TEST_F(TestLoggingService, TestDefaultConfigurationRPC)
{
    ASSERT_NO_THROW(_logging_service_client->setLoggerFilter("file", "", static_cast<int>(fep3::LoggerSeverity::error)));

    Json::Value default_filter = _logging_service_client->getLoggerFilter("");
    EXPECT_EQ(default_filter["enable_sinks"].asString(), "file");
    EXPECT_EQ(static_cast<fep3::LoggerSeverity>(default_filter["severity"].asInt()), fep3::LoggerSeverity::error);

    // The default stays the same after initialization and is not overwritten by the logging service properties
    _component_registry->initialize();

    default_filter = _logging_service_client->getLoggerFilter("");
    EXPECT_EQ(default_filter["enable_sinks"].asString(), "file");
    EXPECT_EQ(static_cast<fep3::LoggerSeverity>(default_filter["severity"].asInt()), fep3::LoggerSeverity::error);
}

/**
* Test the default configuration if set by properties only before initialization
* @req_id ???
*/
TEST_F(TestLoggingService, TestDefaultConfigurationProperties)
{
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(*_configuration_service, "logging/default_sinks", "file"));
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<int32_t>(*_configuration_service, "logging/default_severity", static_cast<int32_t>(fep3::LoggerSeverity::error)));

    // The default configuration has not been changed yet
    Json::Value default_filter = _logging_service_client->getLoggerFilter("");
    EXPECT_EQ(default_filter["enable_sinks"].asString(), "console,rpc");
    EXPECT_EQ(static_cast<fep3::LoggerSeverity>(default_filter["severity"].asInt()), fep3::LoggerSeverity::info);

    // Only at intialization the property values change the default filter
    _component_registry->initialize();

    default_filter = _logging_service_client->getLoggerFilter("");
    EXPECT_EQ(default_filter["enable_sinks"].asString(), "file");
    EXPECT_EQ(static_cast<fep3::LoggerSeverity>(default_filter["severity"].asInt()), fep3::LoggerSeverity::error);
}

/**
* Test the default configuration if set by properties and RPC before initialization
* @req_id ???
*/
TEST_F(TestLoggingService, TestDefaultConfigurationPropertiesAndRPC)
{
    ASSERT_NO_THROW(_logging_service_client->setLoggerFilter("file", "", static_cast<int>(fep3::LoggerSeverity::error)));

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(*_configuration_service, "logging/default_sinks", "console,file"));
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<int32_t>(*_configuration_service, "logging/default_severity", static_cast<int32_t>(fep3::LoggerSeverity::warning)));

    // The property values do not change the default filter before initialization but the RPC does.
    Json::Value default_filter = _logging_service_client->getLoggerFilter("");
    EXPECT_EQ(default_filter["enable_sinks"].asString(), "file");
    EXPECT_EQ(static_cast<fep3::LoggerSeverity>(default_filter["severity"].asInt()), fep3::LoggerSeverity::error);

    // During initialization the default filter doesn't change because RPC has a higher priority than the properties
    _component_registry->initialize();

    default_filter = _logging_service_client->getLoggerFilter("");
    EXPECT_EQ(default_filter["enable_sinks"].asString(), "file");
    EXPECT_EQ(static_cast<fep3::LoggerSeverity>(default_filter["severity"].asInt()), fep3::LoggerSeverity::error);
}

/**
* Test the default configuration if set by RPC after initialization
* @req_id ???
*/
TEST_F(TestLoggingService, TestDefaultConfigurationPropertiesAndRPCAfterInit)
{
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(*_configuration_service, "logging/default_sinks", "console,file"));
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<int32_t>(*_configuration_service, "logging/default_severity", static_cast<int32_t>(fep3::LoggerSeverity::warning)));

    // The property values do not change the default filter before initialization
    Json::Value default_filter = _logging_service_client->getLoggerFilter("");
    EXPECT_EQ(default_filter["enable_sinks"].asString(), "console,rpc");
    EXPECT_EQ(static_cast<fep3::LoggerSeverity>(default_filter["severity"].asInt()), fep3::LoggerSeverity::info);

    // Only at intialization the property values change the default filter
    _component_registry->initialize();

    default_filter = _logging_service_client->getLoggerFilter("");
    EXPECT_EQ(default_filter["enable_sinks"].asString(), "console,file");
    EXPECT_EQ(static_cast<fep3::LoggerSeverity>(default_filter["severity"].asInt()), fep3::LoggerSeverity::warning);

    // RPC will override default filter set by properties
    ASSERT_NO_THROW(_logging_service_client->setLoggerFilter("file", "", static_cast<int>(fep3::LoggerSeverity::error)));

    default_filter = _logging_service_client->getLoggerFilter("");
    EXPECT_EQ(default_filter["enable_sinks"].asString(), "file");
    EXPECT_EQ(static_cast<fep3::LoggerSeverity>(default_filter["severity"].asInt()), fep3::LoggerSeverity::error);
}

/**
* Tests that default configuration if set by properties does not delete other filters set before intilization but still changes the root filter
* @req_id ???
*/
TEST_F(TestLoggingService, TestDefaultConfigurationPropertiesNotDeletingOtherFilters)
{
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(*_configuration_service, "logging/default_sinks", "file"));
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<int32_t>(*_configuration_service, "logging/default_severity", static_cast<int32_t>(fep3::LoggerSeverity::error)));

    // Change a logging filter that is not the root
    ASSERT_NO_THROW(_logging_service_client->setLoggerFilter("rpc", "participant", static_cast<int>(fep3::LoggerSeverity::off)));

    // The default configuration has not been changed yet
    Json::Value default_filter = _logging_service_client->getLoggerFilter("");
    EXPECT_EQ(default_filter["enable_sinks"].asString(), "console,rpc");
    EXPECT_EQ(static_cast<fep3::LoggerSeverity>(default_filter["severity"].asInt()), fep3::LoggerSeverity::info);

    // But the other filter has
    Json::Value participant_filter = _logging_service_client->getLoggerFilter("participant");
    EXPECT_EQ(participant_filter["enable_sinks"].asString(), "rpc");
    EXPECT_EQ(static_cast<fep3::LoggerSeverity>(participant_filter["severity"].asInt()), fep3::LoggerSeverity::off);

    // During intialization the property values change the default filter
    _component_registry->initialize();

    default_filter = _logging_service_client->getLoggerFilter("");
    EXPECT_EQ(default_filter["enable_sinks"].asString(), "file");
    EXPECT_EQ(static_cast<fep3::LoggerSeverity>(default_filter["severity"].asInt()), fep3::LoggerSeverity::error);

    // And the other filter did not get deleted/overwritten
    participant_filter = _logging_service_client->getLoggerFilter("participant");
    EXPECT_EQ(participant_filter["enable_sinks"].asString(), "rpc");
    EXPECT_EQ(static_cast<fep3::LoggerSeverity>(participant_filter["severity"].asInt()), fep3::LoggerSeverity::off);
}
