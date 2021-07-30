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

#include <fep3/participant/component_factories/cpp/component_factory_cpp_plugins.h>
#include <test_plugins/test_component_a_intf.h>
#include <test_plugins/test_component_b_intf.h>
#include <test_plugins/test_component_c_intf.h>
#include <fep3/components/logging/mock/mock_logger.h>

const std::string test_plugin_1_path = PLUGIN_1;
const std::string test_plugin_2_path = PLUGIN_2;

/**
 * Test the loading and creating of a class from a CPPPluginFactory
 * @req_id
*/
TEST(ComponentFactoryCPPPluginTester, testComponentFactory)
{
    using namespace fep3::arya;

    { // plugin 1
        ::testing::StrictMock<fep3::mock::Logger> logger_mock;
        EXPECT_CALL(logger_mock, isInfoEnabled())
            .Times(2)
            .WillRepeatedly(::testing::Return(true))
            ;
        EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
            (std::string() + "Created FEP Component for interface \"" + ITestComponentA::getComponentIID() + "\" from CPP Plugin")))
            .WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
            (std::string() + "Created FEP Component for interface \"" + ITestComponentB::getComponentIID() + "\" from CPP Plugin")))
            .WillOnce(::testing::Return(fep3::Result{}));

        std::unique_ptr<fep3::arya::IComponent> component;
        ASSERT_NO_THROW
        (
            component = ComponentFactoryCPPPlugin(test_plugin_1_path).createComponent(ITestComponentA::getComponentIID(), &logger_mock);
        );
        ASSERT_TRUE(component);
        ITestComponentA* test_interface = static_cast<ITestComponentA*>(component->getInterface(ITestComponentA::getComponentIID()));
        EXPECT_EQ("test_cpp_plugin_1:component_a", test_interface->getIdentifier());

        test_interface->set(4);
        ASSERT_EQ(test_interface->get(), 4);

        test_interface->set(1000);
        ASSERT_EQ(test_interface->get(), 1000);

        std::unique_ptr<fep3::arya::IComponent> component_b;
        ASSERT_NO_THROW
        (
            component_b = ComponentFactoryCPPPlugin(test_plugin_1_path).createComponent(ITestComponentB::getComponentIID(), &logger_mock);
        );
        ASSERT_TRUE(component_b);
        ITestComponentB* test_interface_b = static_cast<ITestComponentB*>(component_b->getInterface(ITestComponentB::getComponentIID()));
        EXPECT_EQ("test_cpp_plugin_1:component_b", test_interface_b->getIdentifier());

        test_interface_b->set(5);
        ASSERT_EQ(test_interface_b->get(), 5);

        test_interface_b->set(2000);
        ASSERT_EQ(test_interface_b->get(), 2000);
    }

    { // plugin 2
        ::testing::StrictMock<fep3::mock::Logger> logger_mock;
        EXPECT_CALL(logger_mock, isInfoEnabled())
            .Times(2)
            .WillRepeatedly(::testing::Return(true))
            ;
        EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
            (std::string() + "Created FEP Component for interface \"" + ITestComponentA::getComponentIID() + "\" from CPP Plugin")))
            .WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(logger_mock, logInfo(fep3::mock::LogStringRegexMatcher
            (std::string() + "Created FEP Component for interface \"" + ITestComponentC::getComponentIID() + "\" from CPP Plugin")))
            .WillOnce(::testing::Return(fep3::Result{}));

        std::unique_ptr<fep3::arya::IComponent> component_a;
        ASSERT_NO_THROW
        (
            component_a = ComponentFactoryCPPPlugin(test_plugin_2_path).createComponent(ITestComponentA::getComponentIID(), &logger_mock);
        );
        ASSERT_TRUE(component_a);
        ITestComponentA* test_interface_a = static_cast<ITestComponentA*>(component_a->getInterface(ITestComponentA::getComponentIID()));
        EXPECT_EQ("test_cpp_plugin_2:component_a", test_interface_a->getIdentifier());

        test_interface_a->set(6);
        ASSERT_EQ(test_interface_a->get(), 6);

        test_interface_a->set(3000);
        ASSERT_EQ(test_interface_a->get(), 3000);

        std::unique_ptr<fep3::arya::IComponent> component_c;
        ASSERT_NO_THROW
        (
            component_c = ComponentFactoryCPPPlugin(test_plugin_2_path).createComponent(ITestComponentC::getComponentIID(), &logger_mock);
        );
        ASSERT_TRUE(component_c);
        ITestComponentC* test_interface_c = static_cast<ITestComponentC*>(component_c->getInterface(ITestComponentC::getComponentIID()));
        EXPECT_EQ("test_cpp_plugin_2:component_c", test_interface_c->getIdentifier());
    }
}