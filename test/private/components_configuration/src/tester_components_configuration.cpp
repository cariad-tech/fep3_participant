/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/base/component_registry/include/component_registry_factory/components_configuration.h>

#include <gtest/gtest.h>

/**
 * Test the loading and creating of a class from a CPPPlugin
 * @req_id FEPSDK-2417
 */
TEST(TestComponentsFile, testLoadingValidFiles)
{
    std::unique_ptr<fep3::arya::ComponentsConfiguration> components_configuration_to_test;
    ASSERT_NO_THROW(components_configuration_to_test =
                        std::make_unique<fep3::arya::ComponentsConfiguration>(
                            CURRENT_TEST_DIR "files/valid.fep_components"););
    const auto& items = components_configuration_to_test->getItems();
    ASSERT_EQ(items.size(), 8);

    // check number of entries by origin
    std::map<fep3::arya::ComponentSourceType, uint32_t> number_of_entries_by_origin;
    for (const auto& item: items) {
        number_of_entries_by_origin[item.second._source_type]++;
    }
    EXPECT_EQ(number_of_entries_by_origin[fep3::arya::ComponentSourceType::unknown], 0u);
    EXPECT_EQ(number_of_entries_by_origin[fep3::arya::ComponentSourceType::built_in], 3u);
    EXPECT_EQ(number_of_entries_by_origin[fep3::arya::ComponentSourceType::c_plugin], 3u);
    EXPECT_EQ(number_of_entries_by_origin[fep3::arya::ComponentSourceType::cpp_plugin], 2u);

    // check that plugin file path is
    // * empty for source type "built_in"
    // * not empty for source types "c_plugin" and "cpp_plugin"
    for (const auto& item: items) {
        switch (item.second._source_type) {
        case fep3::arya::ComponentSourceType::built_in:
            EXPECT_TRUE(item.second._plugin_file_path.empty());
            break;
        case fep3::arya::ComponentSourceType::c_plugin:
            EXPECT_FALSE(item.second._plugin_file_path.empty());
            break;
        case fep3::arya::ComponentSourceType::cpp_plugin:
            EXPECT_FALSE(item.second._plugin_file_path.empty());
            break;
        default:
            FAIL() << "invalid source type";
        }
    }
}

/**
 * Test the loading and creating of a class from a CPPPlugin
 * @req_id FEPSDK-2417
 */
TEST(TestComponentsFile, testLoadingInvalidFiles)
{
    ASSERT_ANY_THROW(fep3::arya::ComponentsConfiguration components_configuration_to_test(
                         CURRENT_TEST_DIR "files/does_not_exist.fep_components"););

    ASSERT_ANY_THROW(fep3::arya::ComponentsConfiguration components_configuration_to_test(
                         CURRENT_TEST_DIR "files/invalid_xml_syntax.fep_components"););

    ASSERT_ANY_THROW(fep3::arya::ComponentsConfiguration components_configuration_to_test(
                         CURRENT_TEST_DIR "files/invalid_sematic.fep_components"););

    ASSERT_ANY_THROW(fep3::arya::ComponentsConfiguration components_configuration_to_test(
                         CURRENT_TEST_DIR "files/invalid_schema_version.fep_components"););
}
