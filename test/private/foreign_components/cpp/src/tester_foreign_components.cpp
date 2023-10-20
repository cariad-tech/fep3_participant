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

#include "test_plugins/test_component_a_intf.h"
#include "test_plugins/test_component_b_intf.h"

#include <fep3/base/component_registry/include/plugin/cpp/cpp_host_plugin.h>
#include <fep3/plugin/cpp/cpp_plugin_component_factory_intf.h>
#include <fep3/plugin/cpp/cpp_plugin_intf.h>

#include <gtest/gtest.h>

const std::string test_plugin_1_path = PLUGIN_1;
const std::string test_plugin_3_path = PLUGIN_3;

/**
 * Test the loading and creating of a class from a CPPPlugin
 * @req_id
 */
TEST(ForeignComponentsCPPPluginTester, testLoading)
{
    using namespace fep3::arya;
    std::unique_ptr<::fep3::plugin::cpp::HostPlugin> plugin;
    ASSERT_NO_THROW(plugin =
                        std::make_unique<::fep3::plugin::cpp::HostPlugin>(test_plugin_1_path););
    ASSERT_TRUE(plugin);
    EXPECT_EQ(plugin->getPluginVersion(), "0.0.1");
    EXPECT_EQ(plugin->getParticipantLibraryVersion(),
              (::fep3::plugin::ParticipantLibraryVersion{
                  FEP3_PARTICIPANT_LIBRARY_VERSION_ID,
                  FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR,
                  FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR,
                  FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH,
                  FEP3_PARTICIPANT_LIBRARY_VERSION_BUILD +
                      0 // + 0 because macro value is not set in developer versions
              }));

    const auto& component_factory = plugin->create<fep3::plugin::cpp::catelyn::IComponentFactory>(
        SYMBOL_fep3_plugin_cpp_catelyn_getFactory);
    ASSERT_NE(nullptr, component_factory);
    auto component = component_factory->createComponent(ITestComponentA::getComponentIID());
    ASSERT_TRUE(component);
    ITestComponentA* test_interface =
        static_cast<ITestComponentA*>(component->getInterface(ITestComponentA::getComponentIID()));

    test_interface->set(5);
    ASSERT_EQ(test_interface->get(), 5);

    test_interface->set(2000);
    ASSERT_EQ(test_interface->get(), 2000);

    EXPECT_EQ("test_cpp_plugin_1:component_a", test_interface->getIdentifier());
}

/**
 * Test the loading and creation of a FEP Super Component (i. e. a single class implementing
 * multiple FEP Component interfaces) from a FEP Component CPP Plugin as well as the
 * interface state synchronicity.
 * @req_id
 */
TEST(ComponentFactoryCPPPluginTester, testSuperComponent)
{
    using namespace fep3::arya;
    std::unique_ptr<::fep3::plugin::cpp::HostPlugin> plugin;
    ASSERT_NO_THROW(plugin =
                        std::make_unique<::fep3::plugin::cpp::HostPlugin>(test_plugin_3_path););
    ASSERT_TRUE(plugin);
    EXPECT_EQ(plugin->getPluginVersion(), "0.0.3");
    EXPECT_EQ(plugin->getParticipantLibraryVersion(),
              (::fep3::plugin::ParticipantLibraryVersion{
                  FEP3_PARTICIPANT_LIBRARY_VERSION_ID,
                  FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR,
                  FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR,
                  FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH,
                  FEP3_PARTICIPANT_LIBRARY_VERSION_BUILD +
                      0 // + 0 because macro value is not set in developer versions
              }));

    const auto& component_factory = plugin->create<fep3::plugin::cpp::catelyn::IComponentFactory>(
        SYMBOL_fep3_plugin_cpp_catelyn_getFactory);
    ASSERT_NE(nullptr, component_factory);

    auto component_a = component_factory->createComponent(ITestComponentA::getComponentIID());
    ASSERT_TRUE(component_a);
    ITestComponentA* test_interface_a = static_cast<ITestComponentA*>(
        component_a->getInterface(ITestComponentA::getComponentIID()));

    auto component_b = component_factory->createComponent(ITestComponentB::getComponentIID());
    ASSERT_TRUE(component_b);
    ITestComponentB* test_interface_b = static_cast<ITestComponentB*>(
        component_b->getInterface(ITestComponentB::getComponentIID()));

    // changing the super component's state through interface ITestComponentA
    // must be observable through interface ITestComponentB ...
    test_interface_a->set(5);
    ASSERT_EQ(test_interface_a->get(), 5);
    ASSERT_EQ(test_interface_b->get(), 5);

    // ... and vice versa
    test_interface_b->set(2000);
    ASSERT_EQ(test_interface_b->get(), 2000);
    ASSERT_EQ(test_interface_a->get(), 2000);

    EXPECT_EQ("test_cpp_plugin_3:super_component_a_b", test_interface_a->getIdentifier());
    EXPECT_EQ("test_cpp_plugin_3:super_component_a_b", test_interface_b->getIdentifier());
}