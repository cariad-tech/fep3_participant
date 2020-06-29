/**
 *
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */

#include <gtest/gtest.h>

//this is an internal test ... so we test the internal implementation and include it!
#include "./../../../../src/fep3/participant/interface/component_factories/cpp/cpp_plugin.h"
#include "./../../../../src/fep3/participant/interface/component_factories/cpp/component_factory_cpp_plugins.h"
#include "test_plugins/test_plugin_1_intf.h"
#include "test_plugins/test_plugin_2_intf.h"

const std::string test_plugin_1_path = PLUGIN_1;
const std::string test_plugin_2_path = PLUGIN_2;

/**
 * Test the loading and creating of a class from a CPPPlugin
 * @req_id FEPSDK-Plugin
 */
TEST(BaseCPPPluginTester, testLoading)
{    
    using namespace fep3::arya;
    std::unique_ptr<CPPPlugin> _plugin;
    ASSERT_NO_THROW
    (
        _plugin = std::make_unique<CPPPlugin>(test_plugin_1_path);
    );
    auto component = _plugin->createComponent(ITestPlugin1::getComponentIID());
    ASSERT_TRUE(component);
    ITestPlugin1* testinterface = reinterpret_cast<ITestPlugin1*>(component->getInterface(ITestPlugin1::getComponentIID()));

    testinterface->set1(5);
    ASSERT_EQ(testinterface->get1(), 5);

    testinterface->set1(2000);
    ASSERT_EQ(testinterface->get1(), 2000);
}

/**
 * Test the loading and creating of a class from a CPPPluginFactory
 * @req_id FEPSDK-Factory
 */
TEST(BaseCPPPluginTester, testComponentFactory)
{
    using namespace fep3::arya;
    std::unique_ptr<CPPPluginComponentFactory> _factory;
    std::vector<std::string> plugins = { test_plugin_1_path, test_plugin_2_path };
    
    ASSERT_NO_THROW
    (
        _factory = std::make_unique<CPPPluginComponentFactory>(plugins);
    );

    {
        //plugin 1
        std::unique_ptr<IComponent> component = _factory->createComponent(ITestPlugin1::getComponentIID());
        ASSERT_TRUE(component);
        ITestPlugin1* testinterface = reinterpret_cast<ITestPlugin1*>(component->getInterface(ITestPlugin1::getComponentIID()));

        testinterface->set1(5);
        ASSERT_EQ(testinterface->get1(), 5);

        testinterface->set1(2000);
        ASSERT_EQ(testinterface->get1(), 2000);
    }

    {
        //plugin 2
        std::unique_ptr<IComponent> component = _factory->createComponent(ITestPlugin2::getComponentIID());
        ASSERT_TRUE(component);
        ITestPlugin2* testinterface = reinterpret_cast<ITestPlugin2*>(component->getInterface(ITestPlugin2::getComponentIID()));

        testinterface->set2(5);
        ASSERT_EQ(testinterface->get2(), 5);

        testinterface->set2(2000);
        ASSERT_EQ(testinterface->get2(), 2000);
    }
};
