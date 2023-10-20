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

#ifdef WIN32
    #include <Windows.h>
#endif

#include <fep3/base/component_registry/component_registry_factory.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>

#include <environment_variable.h>
#include <helper/copy_file.h>
#include <helper/platform_filesystem.h>

// interfaces of native components
#include <fep3/native_components/data_registry/data_registry.h>
#include <fep3/native_components/service_bus/include/service_bus.h>

// interfaces of CPP plugins
#include <test_plugins/test_component_a_intf.h>
#include <test_plugins/test_component_b_intf.h>
#include <test_plugins/test_component_c_intf.h>

class ComponentRegistryFactoryTestFixture : public ::testing::Test {
protected:
    ::testing::StrictMock<fep3::mock::LoggerWithDefaultBehavior> _logger_mock;
    const std::string _working_directory = std::string(TEST_BUILD_DIR);
    const std::string _components_path_env_var = "FEP3_PARTICIPANT_COMPONENTS_FILE_PATH";

    void SetUp() override
    {
        // JUST FOR DEBUGGING: Usually cleanup tasks should be done only in TearDown,
        // but when the debugger is stopped during an TC execution TearDown() won't be called and
        // the next TC may fail
        TearDown();
    }

    void TearDown() override
    {
        // make sure the default components configuration file is not in the current working
        // directory (e. g. due to previous test runs)
        removeComponentsFiles(_working_directory);
        // make sure the environment variable is not set (e. g. due to previous test runs)
        unsetEnvironmentVariable(_components_path_env_var);
    }

    void removeComponentsFiles(const fs::path& path)
    {
        // remove all components files
        for (auto& p: fs::directory_iterator(path)) {
            if (p.path().extension() == ".fep_components") {
                std::error_code ec;
                fs::remove(p.path(), ec);
                ASSERT_EQ(ec.value(), 0);
            }
        }
    }

    void unsetEnvironmentVariable(const std::string& name)
    {
#ifdef WIN32
        ::SetEnvironmentVariable(name.c_str(), nullptr);
#else // WIN32
        unsetenv(name.c_str());
#endif
    }
};

/**
 * Creation of a component registry with built in components is not possible, a fep_components file
 * should be always defined.
 * @req_id FEPSDK-3218
 */
TEST_F(ComponentRegistryFactoryTestFixture, createRegistry__failOnNoDefaultComponentsFile)
{
    using ::testing::_;

    EXPECT_CALL(_logger_mock, isDebugEnabled()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(_logger_mock,
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "searching for default FEP Component Configuration File")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(_logger_mock, isErrorEnabled()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(_logger_mock, logError(_)).WillOnce(::testing::Return(fep3::Result{}));

    EXPECT_THROW(fep3::arya::ComponentRegistryFactory::createRegistry(&_logger_mock,
                                                                      _components_path_env_var),
                 std::runtime_error);
}

/**
 * Test the creation of a component registry according to a components configuration file
 * given by an environment variable
 * @req_id FEPSDK-2417
 */
TEST_F(ComponentRegistryFactoryTestFixture, createRegistry__successOnNonDefaultComponentsFile)
{
    // copy source to the non-default file path
    const std::string components_file_path_source =
        _working_directory + "/files/test.fep_components";
    const std::string non_default_file_path(_working_directory +
                                            "/non-default-file-name.fep_components");
    ASSERT_TRUE(test::helper::copyFile(components_file_path_source, non_default_file_path));
    ASSERT_EQ(::fep3::Result{},
              ::fep3::environment_variable::set(_components_path_env_var, non_default_file_path));

    EXPECT_CALL(_logger_mock, isDebugEnabled()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(_logger_mock,
                logDebug(fep3::mock::LogStringRegexMatcher("Found environment variable")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(
        _logger_mock,
        logDebug(fep3::mock::LogStringRegexMatcher("FEP Component Configuration File found")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(_logger_mock,
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Created FEP Component for interface \".*\" from CPP Plugin")))
        .Times(4)
        .WillRepeatedly(::testing::Return(fep3::Result{}));

    std::shared_ptr<fep3::arya::ComponentRegistry> registry;
    ASSERT_NO_THROW(registry = fep3::arya::ComponentRegistryFactory::createRegistry(
                        &_logger_mock, _components_path_env_var););

    // check one of the non-default components
    {
        auto test_interface = registry->getComponent<ITestComponentA>();
        ASSERT_NE(nullptr, test_interface);
    }
}

/**
 * Test the creation of a component registry according to a components configuration file
 * where component implementations of a single interface are loaded from one or another plugin
 * @req_id FEPSDK-2417
 */
TEST_F(ComponentRegistryFactoryTestFixture, createRegistry__successOnPluginSelection1)
{
    { // load ITestComponentA from plugin_1
        const std::string component_a_from_plugin_1_file_path_source =
            _working_directory + "/files/test_a_from_cpp_plugin_1.fep_components";
        ASSERT_TRUE(
            test::helper::copyFile(component_a_from_plugin_1_file_path_source,
                                   _working_directory + "/fep3_participant.fep_components"));

        EXPECT_CALL(_logger_mock, isDebugEnabled()).WillRepeatedly(::testing::Return(true));
        EXPECT_CALL(_logger_mock,
                    logDebug(fep3::mock::LogStringRegexMatcher(
                        "searching for default FEP Component Configuration File")))
            .WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(
            _logger_mock,
            logDebug(fep3::mock::LogStringRegexMatcher("FEP Component Configuration File found")))
            .WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(_logger_mock,
                    logDebug(fep3::mock::LogStringRegexMatcher(
                        "Created FEP Component for interface \".*\" from CPP Plugin")))
            .Times(3)
            .WillRepeatedly(::testing::Return(fep3::Result{}));

        std::shared_ptr<fep3::arya::ComponentRegistry> registry;
        ASSERT_NO_THROW(registry = fep3::arya::ComponentRegistryFactory::createRegistry(
                            &_logger_mock, _components_path_env_var););

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
}

/**
 * Test the creation of a component registry according to a components configuration file
 * where component implementations of a single interface are loaded from one or another plugin
 * @req_id FEPSDK-2417
 */
TEST_F(ComponentRegistryFactoryTestFixture, createRegistry__successOnPluginSelection2)
{
    { // load ITestComponentA from plugin_2
        const std::string component_a_from_plugin_2_file_path_source =
            _working_directory + "/files/test_a_from_cpp_plugin_2.fep_components";
        ASSERT_TRUE(
            test::helper::copyFile(component_a_from_plugin_2_file_path_source,
                                   _working_directory + "/fep3_participant.fep_components"));

        EXPECT_CALL(_logger_mock, isDebugEnabled()).WillRepeatedly(::testing::Return(true));
        EXPECT_CALL(_logger_mock,
                    logDebug(fep3::mock::LogStringRegexMatcher(
                        "searching for default FEP Component Configuration File")))
            .WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(
            _logger_mock,
            logDebug(fep3::mock::LogStringRegexMatcher("FEP Component Configuration File found")))
            .WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(_logger_mock,
                    logDebug(fep3::mock::LogStringRegexMatcher(
                        "Created FEP Component for interface \".*\" from CPP Plugin")))
            .Times(3)
            .WillRepeatedly(::testing::Return(fep3::Result{}));

        std::shared_ptr<fep3::arya::ComponentRegistry> registry;
        ASSERT_NO_THROW(registry = fep3::arya::ComponentRegistryFactory::createRegistry(
                            &_logger_mock, _components_path_env_var););

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
TEST_F(ComponentRegistryFactoryTestFixture, createRegistry__failOnNoComponentsFile)
{
    ::fep3::environment_variable::set("FEP3_PARTICIPANT_COMPONENTS_FILE_PATH",
                                      "non-existing-file-path");

    EXPECT_CALL(_logger_mock, isDebugEnabled()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(_logger_mock,
                logDebug(fep3::mock::LogStringRegexMatcher("Found environment variable")))
        .WillOnce(::testing::Return(fep3::Result{}));

    ASSERT_THROW(fep3::arya::ComponentRegistryFactory::createRegistry(&_logger_mock,
                                                                      _components_path_env_var),
                 std::runtime_error);
}

/**
 * Test failure of component registry creation if the components configuration file is invalid
 * @req_id FEPSDK-2417
 */
TEST_F(ComponentRegistryFactoryTestFixture, createRegistry__failOnInvalidDefaultComponentsFile)
{
    const std::string invalid_components_file_path_source =
        _working_directory + "/files/test_invalid_type.fep_components";
    ASSERT_TRUE(test::helper::copyFile(invalid_components_file_path_source,
                                       _working_directory + "/fep3_participant.fep_components"));

    EXPECT_CALL(_logger_mock, isDebugEnabled()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(_logger_mock,
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "searching for default FEP Component Configuration File")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(
        _logger_mock,
        logDebug(fep3::mock::LogStringRegexMatcher("FEP Component Configuration File found")))
        .WillOnce(::testing::Return(fep3::Result{}));

    ASSERT_THROW(fep3::arya::ComponentRegistryFactory::createRegistry(&_logger_mock,
                                                                      _components_path_env_var),
                 std::runtime_error);
}

/**
 * Test the creation of a component registry according to a components configuration file
 * in the current working directory containing components of types
 * * native
 * * cpp-plugin
 * * c-plugin
 * NOTE: The component configuration file itself is still in the current working directory but the
 * pathes in this file are relative
 * @req_id FEPSDK-2417
 */
TEST_F(ComponentRegistryFactoryTestFixture, createRegistry__successOnRelativeComponentPaths)
{
    const std::string relative_path_components_file_path_source =
        _working_directory + "/files/test_rel_path.fep_components";
    ASSERT_TRUE(test::helper::copyFile(relative_path_components_file_path_source,
                                       _working_directory + "/fep3_participant.fep_components"));

    EXPECT_CALL(_logger_mock, isDebugEnabled()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(_logger_mock,
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "searching for default FEP Component Configuration File")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(
        _logger_mock,
        logDebug(fep3::mock::LogStringRegexMatcher("FEP Component Configuration File found")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(_logger_mock,
                logDebug(fep3::mock::LogStringRegexMatcher(
                    "Created FEP Component for interface \".*\" from CPP Plugin")))
        .Times(4)
        .WillRepeatedly(::testing::Return(fep3::Result{}));

    std::shared_ptr<fep3::arya::ComponentRegistry> registry;
    ASSERT_NO_THROW(registry = fep3::arya::ComponentRegistryFactory::createRegistry(
                        &_logger_mock, _components_path_env_var););

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
        // plugin 2
        auto test_interface = registry->getComponent<ITestComponentC>();
        ASSERT_NE(test_interface, nullptr);

        EXPECT_EQ("test_cpp_plugin_2:component_c", test_interface->getIdentifier());
    }
}
