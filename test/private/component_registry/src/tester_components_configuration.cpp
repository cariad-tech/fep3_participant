/**
 * Copyright 2024 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/fep3_filesystem.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <component_registry_factory/components_configuration.h>
#include <fstream>
#include <regex>

namespace {

const std::string source_placeholder = "#FILE_LOCATION#";

const std::string xml_string = R"(<?xml version="1.0" encoding="utf-8"?> 
        <components xmlns = "http://fep.vwgroup.com/fep_sdk/3.0/components">
            <schema_version> 1.0.0 </schema_version>
            <component>
                <source type = "cpp-plugin">)" +
                               source_placeholder +
                               R"(</source>
                <iid> logging_service.arya.fep3.iid</iid>
            </component>
        </components>)";
} // namespace

class TestComponentsConfiguration : public ::testing::Test {
protected:
    void SetUp() override
    {
        fs::create_directory(_test_dir);
        fs::create_directory(_components_file_directory);
        // convert this to an os specific path so we can compare when testing
        _components_file_directory = fs::canonical(_components_file_directory);
    }

    void TearDown() override
    {
        fs::remove_all(_test_dir);
    }

    void writeComponentsFile(const std::string& plugin_path)
    {
        std::regex source_placeholder_re(source_placeholder);

        std::ofstream output(_components_file_path);
        std::ostream_iterator<char> out_it(output, "");

        std::regex_replace(
            out_it, xml_string.begin(), xml_string.end(), source_placeholder_re, plugin_path);
    }

    const std::string _components_file_dir_name = "components_file_dir";
    const fs::path _test_dir = TEST_FILES_DIR;
    fs::path _components_file_directory = _test_dir / _components_file_dir_name;

    const fs::path _components_file_path = _components_file_directory / "test.fep_components";
};

TEST_F(TestComponentsConfiguration, pluginPathDotFwdSlash_success)
{
    writeComponentsFile("./fep_components_plugin");
    fep3::arya::ComponentsConfiguration conf(_components_file_path.string());
    fs::path expected_resolved_path(_components_file_directory / "fep_components_plugin");
    ASSERT_EQ(conf.getItems().front().second._plugin_file_path, expected_resolved_path.string());
}

TEST_F(TestComponentsConfiguration, pluginPathPlain_success)
{
    writeComponentsFile("fep_components_plugin");
    fep3::arya::ComponentsConfiguration conf(_components_file_path.string());
    fs::path expected_resolved_path(_components_file_directory / "fep_components_plugin");
    ASSERT_EQ(conf.getItems().front().second._plugin_file_path, expected_resolved_path.string());
}

#ifdef _WIN32
// backslash is a valid character in Linux paths
TEST_F(TestComponentsConfiguration, pluginPathDotBackSlash_success)
{
    writeComponentsFile(R"(.\fep_components_plugin)");
    fep3::arya::ComponentsConfiguration conf(_components_file_path.string());
    fs::path expected_resolved_path(_components_file_directory / "fep_components_plugin");
    ASSERT_EQ(conf.getItems().front().second._plugin_file_path, expected_resolved_path.string());
}

TEST_F(TestComponentsConfiguration, pluginPathDotDoubleBackSlash_success)
{
    writeComponentsFile(R"(.\\fep_components_plugin)");
    fep3::arya::ComponentsConfiguration conf(_components_file_path.string());
    fs::path expected_resolved_path(_components_file_directory / "fep_components_plugin");
    ASSERT_EQ(conf.getItems().front().second._plugin_file_path, expected_resolved_path.string());
}
#endif

TEST_F(TestComponentsConfiguration, pluginPathRelativeParent_success)
{
    writeComponentsFile(R"(../)" + _components_file_dir_name + R"(/fep_components_plugin)");
    fep3::arya::ComponentsConfiguration conf(_components_file_path.string());
    fs::path expected_resolved_path(_components_file_directory / "fep_components_plugin");
    ASSERT_EQ(conf.getItems().front().second._plugin_file_path, expected_resolved_path.string());
}

TEST_F(TestComponentsConfiguration, pluginPathRelativeChild_success)
{
    const std::string child_folder_name = "child_folder";
    writeComponentsFile(R"(./)" + child_folder_name + R"(/fep_components_plugin)");
    fs::create_directory(_components_file_directory / child_folder_name);

    fep3::arya::ComponentsConfiguration conf(_components_file_path.string());
    fs::path expected_resolved_path(_components_file_directory / child_folder_name /
                                    "fep_components_plugin");
    ASSERT_EQ(conf.getItems().front().second._plugin_file_path, expected_resolved_path.string());
}

TEST_F(TestComponentsConfiguration, pluginPathAbsolute_success)
{
    writeComponentsFile((_components_file_directory / "fep_components_plugin").string());

    fep3::arya::ComponentsConfiguration conf(_components_file_path.string());
    fs::path expected_resolved_path(_components_file_directory / "fep_components_plugin");
    ASSERT_EQ(conf.getItems().front().second._plugin_file_path, expected_resolved_path.string());
}

TEST_F(TestComponentsConfiguration, pluginPathAbsoluteNoExisting_success)
{
    auto non_existing_path =
        (_components_file_directory / "NonExistingFolder" / "fep_components_plugin").string();

    writeComponentsFile(non_existing_path);
    fep3::arya::ComponentsConfiguration conf(_components_file_path.string());

    ASSERT_EQ(conf.getItems().front().second._plugin_file_path, non_existing_path);
}

TEST_F(TestComponentsConfiguration, pluginPathRelativeNotExisting_exception)
{
    auto non_existing_path =
        (_components_file_directory / "NonExistingFolder" / "fep_components_plugin").string();

    writeComponentsFile("./NonExistingFolder/fep_components_plugin");
    EXPECT_THROW(fep3::arya::ComponentsConfiguration{_components_file_path.string()},
                 std::runtime_error);
}

TEST_F(TestComponentsConfiguration, componentsFileNotExisting_exception)
{
    auto non_existing_path =
        (_components_file_directory / "NonExistingFolder" / "test.fep_components").string();

    EXPECT_THROW(fep3::arya::ComponentsConfiguration{non_existing_path}, std::runtime_error);
}
