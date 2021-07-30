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

#include <fep3/fep3_macros.h>

#include <fep3/plugin/cpp/cpp_host_plugin.h>
#include <fep3/participant/component_factories/cpp/component_creator_cpp_plugin.h>
#include "test_plugins/test_component_a_intf.h"

const std::string test_plugin_1_path = PLUGIN_1;

/**
 * Test the loading and creating of a class from a CPPPlugin
 * @req_id
*/
TEST(ForeignComponentsCPPPluginTester, testLoading)
{
    using namespace fep3::arya;
    std::unique_ptr<::fep3::plugin::cpp::HostPlugin> plugin;
    ASSERT_NO_THROW
    (
        plugin = std::make_unique<::fep3::plugin::cpp::HostPlugin>(test_plugin_1_path);
    );
    ASSERT_TRUE(plugin);
    EXPECT_EQ(plugin->getVersionNamespace(), "arya");
    EXPECT_EQ(plugin->getPluginVersion(), "0.0.1");
    EXPECT_EQ(plugin->getParticipantLibraryVersion()
        , (::fep3::plugin::arya::ParticipantLibraryVersion
            {FEP3_PARTICIPANT_LIBRARY_VERSION_ID
            , FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR
            , FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR
            , FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH
            , FEP3_PARTICIPANT_LIBRARY_VERSION_BUILD + 0 // + 0 because macro value is not set in developer versions
            })
        );

    auto component = ComponentCreatorCPPPlugin()(*plugin.get(), ITestComponentA::getComponentIID());
    ASSERT_TRUE(component);
    ITestComponentA* test_interface = static_cast<ITestComponentA*>(component->getInterface(ITestComponentA::getComponentIID()));

    test_interface->set(5);
    ASSERT_EQ(test_interface->get(), 5);

    test_interface->set(2000);
    ASSERT_EQ(test_interface->get(), 2000);

    EXPECT_EQ("test_cpp_plugin_1:component_a", test_interface->getIdentifier());
}