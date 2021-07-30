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
#include <gmock/gmock.h>

#include <a_util/filesystem.h>
#include <a_util/system.h>

#include <fstream>
#include <iostream>

#include <helper/copy_file.h>
#include <fep3/base/environment_variable/environment_variable.h>
#include <fep3/participant/component_registry_factory/component_registry_factory.h>
#include <fep3/components/logging/mock/mock_logger.h>
// interfaces of native components
#include <fep3/components/data_registry/data_registry_intf.h>
#include <fep3/native_components/data_registry/data_registry.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/job_registry/job_registry_intf.h>
#include <fep3/components/scheduler/scheduler_service_intf.h>
#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/native_components/service_bus/service_bus.h>
#include <fep3/components/simulation_bus/simulation_bus_intf.h>
#include <fep3/native_components/simulation_bus/simulation_bus.h>
#include <fep3/components/clock_sync/clock_sync_service_intf.h>
#include <fep3/components/simulation_bus/c_access_wrapper/simulation_bus_c_access_wrapper.h>
// interfaces of CPP plugins
#include <test_plugins/test_component_a_intf.h>
#include <test_plugins/test_component_b_intf.h>
#include <test_plugins/test_component_c_intf.h>
// interfaces of C plugins
#include <test_plugins/plugin_1/component_a_intf.h>

const std::string components_file_path_source = std::string(TEST_BUILD_DIR) + "/files/test.fep_components";
const std::string invalid_components_file_path_source = std::string(TEST_BUILD_DIR) + "/files/test_invalid_type.fep_components";

void unsetEnvironmentVariable(const std::string& name)
{
#ifdef WIN32
    ::SetEnvironmentVariable(name.c_str(), nullptr);
#else //WIN32
    unsetenv(name.c_str());
#endif
}

/**
 * Test the creation of a component registry with default components, i. e. not using a components configuration file
 * @req_id FEPSDK-2417
 */
TEST(ComponentRegistryFactoryTester, testComponentRegistryCreationDefault)
{
    // make sure the default components configuration file is not in the current working directory
    // (e. g. due to previous test runs)
    a_util::filesystem::remove(std::string(TEST_BUILD_DIR) + "/fep3_participant.fep_components");

    ::testing::StrictMock<fep3::mock::Logger> logger_mock;
    EXPECT_CALL(logger_mock, isInfoEnabled()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
        ("searching for default FEP Component Configuration File")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
        ("loading default FEP Components")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
        ("Created built-in FEP Component")))
        .WillRepeatedly(::testing::Return(fep3::Result{}));

    std::shared_ptr<fep3::arya::ComponentRegistry> registry;
    ASSERT_NO_THROW
    (
        registry = fep3::arya::ComponentRegistryFactory::createRegistry(&logger_mock);
    );

    // native components
    {
        auto test_interface = registry->getComponent<fep3::IDataRegistry>();
        ASSERT_NE(test_interface, nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::native::DataRegistry*>(test_interface));
    }
    {
        auto test_interface = registry->getComponent<fep3::IServiceBus>();
        ASSERT_NE(test_interface, nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::native::ServiceBus*>(test_interface));
    }
    {
        auto test_interface = registry->getComponent<fep3::ISimulationBus>();
        ASSERT_NE(test_interface, nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::native::SimulationBus*>(test_interface));
    }
    {
        auto test_interface = registry->getComponent<fep3::IClockService>();
        ASSERT_NE(test_interface, nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::IClockService*>(test_interface));
    }
    {
        auto test_interface = registry->getComponent<fep3::IClockSyncService>();
        ASSERT_NE(test_interface, nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::IClockSyncService*>(test_interface));
    }
    {
        auto test_interface = registry->getComponent<fep3::IJobRegistry>();
        ASSERT_NE(test_interface, nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::IJobRegistry*>(test_interface));
    }
    {
        auto test_interface = registry->getComponent<fep3::ISchedulerService>();
        ASSERT_NE(test_interface, nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::ISchedulerService*>(test_interface));
    }
}

/**
 * Test the creation of a component registry according to a components configuration file
 * given by an environment variable
 * @req_id FEPSDK-2417
 */
TEST(ComponentRegistryFactoryTester, testComponentRegistryCreationByEnvVar)
{
    // make sure the default components configuration file is not in the current working directory
    // (e. g. due to previous test runs)
    a_util::filesystem::remove(std::string(TEST_BUILD_DIR) + "/fep3_participant.fep_components");
    // copy source to the non-default file path
    const std::string non_default_file_path(std::string(TEST_BUILD_DIR) + "/non-default-file-name.fep_components");
    ASSERT_TRUE(test::helper::copyFile(components_file_path_source, non_default_file_path));
    ASSERT_EQ(::fep3::Result{}, ::fep3::environment_variable::set("FEP3_PARTICIPANT_COMPONENTS_FILE_PATH", non_default_file_path));

    ::testing::StrictMock<fep3::mock::Logger> logger_mock;
    EXPECT_CALL(logger_mock, isInfoEnabled()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
        ("Found environment variable")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
        ("FEP Component Configuration File found")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
        ("Created built-in FEP Component")))
        .Times(2)
        .WillRepeatedly(::testing::Return(fep3::Result{}));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
        ("Created FEP Component for interface \".*\" from CPP Plugin")))
        .Times(2)
        .WillRepeatedly(::testing::Return(fep3::Result{}));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
        ("Created FEP Component for interface \".*\" from C Plugin")))
        .Times(1)
        .WillRepeatedly(::testing::Return(fep3::Result{}));

    std::shared_ptr<fep3::arya::ComponentRegistry> registry;
    ASSERT_NO_THROW
    (
        registry = fep3::arya::ComponentRegistryFactory::createRegistry(&logger_mock);
    );

    // check one of the non-default components
    {
        auto test_interface = registry->getComponent<ITestComponentA>();
        ASSERT_NE(nullptr, test_interface);
    }
}

/**
 * Test the creation of a component registry according to a components configuration file
 * in the current working directory containing components of types
 * * native
 * * cpp-plugin
 * * c-plugin
 * @req_id FEPSDK-2417
 */
TEST(ComponentRegistryFactoryTester, testComponentRegistryCreationByFile)
{
    // make sure the environment variable is not set (e. g. due to previous test runs)
    unsetEnvironmentVariable("FEP3_PARTICIPANT_COMPONENTS_FILE_PATH");

    ASSERT_TRUE(test::helper::copyFile(components_file_path_source, std::string(TEST_BUILD_DIR) + "/fep3_participant.fep_components"));

    ::testing::StrictMock<fep3::mock::Logger> logger_mock;
    EXPECT_CALL(logger_mock, isInfoEnabled()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
        ("searching for default FEP Component Configuration File")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
        ("FEP Component Configuration File found")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
        ("Created built-in FEP Component")))
        .Times(2)
        .WillRepeatedly(::testing::Return(fep3::Result{}));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
        ("Created FEP Component for interface \".*\" from CPP Plugin")))
        .Times(2)
        .WillRepeatedly(::testing::Return(fep3::Result{}));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
        ("Created FEP Component for interface \".*\" from C Plugin")))
        .Times(1)
        .WillRepeatedly(::testing::Return(fep3::Result{}));

    std::shared_ptr<fep3::arya::ComponentRegistry> registry;
    ASSERT_NO_THROW
    (
        registry = fep3::arya::ComponentRegistryFactory::createRegistry(&logger_mock);
    );

    // native components
    {
        auto test_interface = registry->getComponent<fep3::IDataRegistry>();
        ASSERT_NE(test_interface, nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::native::DataRegistry*>(test_interface));
    }
    {
        auto test_interface = registry->getComponent<fep3::IServiceBus>();
        ASSERT_NE(test_interface, nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::native::ServiceBus*>(test_interface));
    }

    // components from CPP plugins
    {
        auto test_interface = registry->getComponent<ITestComponentA>();
        ASSERT_NE(test_interface, nullptr);

        EXPECT_EQ("test_cpp_plugin_1:component_a", test_interface->getIdentifier());

        test_interface->set(5);
        ASSERT_EQ(test_interface->get(), 5);

        test_interface->set(2000);
        ASSERT_EQ(test_interface->get(), 2000);
    }
    {
        //plugin 2
        auto test_interface = registry->getComponent<ITestComponentC>();
        ASSERT_NE(test_interface, nullptr);

        EXPECT_EQ("test_cpp_plugin_2:component_c", test_interface->getIdentifier());
    }

    // components from C plugins
    {
        auto test_interface = registry->getComponent<::fep3::ISimulationBus>();
        ASSERT_NE(test_interface, nullptr);
        EXPECT_NE(nullptr, dynamic_cast<::fep3::plugin::c::access::arya::SimulationBus*>(test_interface));
    }
}

const std::string component_a_from_plugin_1_file_path_source = std::string(TEST_BUILD_DIR) + "/files/test_a_from_cpp_plugin_1.fep_components";
const std::string component_a_from_plugin_2_file_path_source = std::string(TEST_BUILD_DIR) + "/files/test_a_from_cpp_plugin_2.fep_components";

/**
 * Test the creation of a component registry according to a components configuration file
 * where component implementations of a single interface are loaded from one or another plugin
 * @req_id FEPSDK-2417
 */
TEST(ComponentRegistryFactoryTester, testComponentRegistrySelectImplFromSpecificPlugin)
{
    // make sure the environment variable is not set (e. g. due to previous test runs)
    unsetEnvironmentVariable("FEP3_PARTICIPANT_COMPONENTS_FILE_PATH");

    { // load ITestComponentA from plugin_1
        ASSERT_TRUE(test::helper::copyFile(component_a_from_plugin_1_file_path_source, std::string(TEST_BUILD_DIR) + "/fep3_participant.fep_components"));

        ::testing::StrictMock<fep3::mock::Logger> logger_mock;
        EXPECT_CALL(logger_mock, isInfoEnabled()).WillRepeatedly(::testing::Return(true));
        EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
            ("searching for default FEP Component Configuration File")))
            .WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
            ("FEP Component Configuration File found")))
            .WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
            ("Created FEP Component for interface \".*\" from CPP Plugin")))
            .Times(3)
            .WillRepeatedly(::testing::Return(fep3::Result{}));

        std::shared_ptr<fep3::arya::ComponentRegistry> registry;
        ASSERT_NO_THROW
        (
            registry = fep3::arya::ComponentRegistryFactory::createRegistry(&logger_mock);
        );

        auto test_component_a = registry->getComponent<ITestComponentA>();
        ASSERT_NE(test_component_a, nullptr);
        EXPECT_EQ("test_cpp_plugin_1:component_a", test_component_a->getIdentifier());

        auto test_component_b = registry->getComponent<ITestComponentB>();
        ASSERT_NE(test_component_b, nullptr);
        EXPECT_EQ("test_cpp_plugin_1:component_b", test_component_b->getIdentifier());

        auto test_component_c = registry->getComponent<ITestComponentC>();
        ASSERT_NE(test_component_c, nullptr);
        EXPECT_EQ("test_cpp_plugin_2:component_c", test_component_c->getIdentifier());
    }

    { // load ITestComponentA from plugin_2
    ASSERT_TRUE(test::helper::copyFile(component_a_from_plugin_2_file_path_source, std::string(TEST_BUILD_DIR) + "/fep3_participant.fep_components"));

        ::testing::StrictMock<fep3::mock::Logger> logger_mock;
        EXPECT_CALL(logger_mock, isInfoEnabled()).WillRepeatedly(::testing::Return(true));
        EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
            ("searching for default FEP Component Configuration File")))
            .WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
            ("FEP Component Configuration File found")))
            .WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
            ("Created FEP Component for interface \".*\" from CPP Plugin")))
            .Times(3)
            .WillRepeatedly(::testing::Return(fep3::Result{}));

        std::shared_ptr<fep3::arya::ComponentRegistry> registry;
        ASSERT_NO_THROW
        (
            registry = fep3::arya::ComponentRegistryFactory::createRegistry(&logger_mock);
        );

        auto test_component_a = registry->getComponent<ITestComponentA>();
        ASSERT_NE(test_component_a, nullptr);
        EXPECT_EQ("test_cpp_plugin_2:component_a", test_component_a->getIdentifier());

        auto test_component_b = registry->getComponent<ITestComponentB>();
        ASSERT_NE(test_component_b, nullptr);
        EXPECT_EQ("test_cpp_plugin_1:component_b", test_component_b->getIdentifier());

        auto test_component_c = registry->getComponent<ITestComponentC>();
        ASSERT_NE(test_component_c, nullptr);
        EXPECT_EQ("test_cpp_plugin_2:component_c", test_component_c->getIdentifier());
    }
}

/**
 * Test failure of component registry creation if the components configuration file does not exist
 * @req_id FEPSDK-2417
 */
TEST(ComponentRegistryFactoryTester, testComponentRegistryCreationFailureOnNonExistingFileInEnvVar)
{
    ::fep3::environment_variable::set("FEP3_PARTICIPANT_COMPONENTS_FILE_PATH", "non-existing-file-path");

    ::testing::StrictMock<fep3::mock::Logger> logger_mock;
    EXPECT_CALL(logger_mock, isInfoEnabled()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher("Found environment variable"))).WillOnce(::testing::Return(fep3::Result{}));

    ASSERT_ANY_THROW
    (
        auto registry = fep3::arya::ComponentRegistryFactory::createRegistry(&logger_mock);
    );
}

/**
 * Test failure of component registry creation if the components configuration file is invalid
 * @req_id FEPSDK-2417
 */
TEST(ComponentRegistryFactoryTester, testComponentRegistryCreationFailureOnInvalidFile)
{
    // make sure the environment variable is not set (e. g. due to previous test runs)
    unsetEnvironmentVariable("FEP3_PARTICIPANT_COMPONENTS_FILE_PATH");

    ASSERT_TRUE(test::helper::copyFile(invalid_components_file_path_source, std::string(TEST_BUILD_DIR) + "/fep3_participant.fep_components"));

    ::testing::StrictMock<fep3::mock::Logger> logger_mock;
    EXPECT_CALL(logger_mock, isInfoEnabled()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
        ("searching for default FEP Component Configuration File")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
        ("FEP Component Configuration File found")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
        ("Created FEP Component for interface \".*\" from CPP Plugin")))
        .WillOnce(::testing::Return(fep3::Result{}));

    ASSERT_ANY_THROW
    (
        auto registry = fep3::arya::ComponentRegistryFactory::createRegistry(&logger_mock);
    );

}


