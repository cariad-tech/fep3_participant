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

#include <fep3/plugin/c/c_host_plugin.h>
#include <fep3/components/configuration/mock/mock_configuration_service.h>
#include <fep3/components/configuration/c_access_wrapper/configuration_service_c_access_wrapper.h>
#include <helper/component_c_plugin_helper.h>

const std::string test_plugin_path = PLUGIN;

using namespace fep3::plugin::c::arya;
using namespace fep3::plugin::c::access::arya;

struct Plugin1PathGetter
{
    std::string operator()() const
    {
        return test_plugin_path;
    }
};
struct SetMockComponentFunctionSymbolGetter
{
    std::string operator()() const
    {
        return "setMockConfigurationService";
    }
};

/**
 * Test fixture loading a mocked Configuration Service from within a C plugin
 */
using ConfigurationServiceLoader = MockedComponentCPluginLoader
    <fep3::IConfigurationService
    , fep3::mock::TransferableConfigurationService
    , fep3::plugin::c::access::arya::ConfigurationService
    , Plugin1PathGetter
    , SetMockComponentFunctionSymbolGetter
    >;
using ConfigurationServiceLoaderFixture = MockedComponentCPluginLoaderFixture<ConfigurationServiceLoader>;

/**
 * Test method fep3::IConfigurationService::registerNode of a Configuration Service
 * that resides in a C plugin
 * @req_id FEPSDK-2369
 */
TEST_F(ConfigurationServiceLoaderFixture, testMethod_registerNode)
{
    const auto& mock_property_node = std::make_shared<::testing::StrictMock<fep3::mock::PropertyNode>>();
    std::shared_ptr<fep3::arya::IPropertyNode> remote_property_node;
    const auto& test_property_node_name = std::string("foo");

    // setting of expectations
    {
        EXPECT_CALL(*mock_property_node.get(), getName())
            .WillOnce(::testing::Return(test_property_node_name));

        auto& mock_configuration_service = getMockComponent();

        EXPECT_CALL(mock_configuration_service, registerNode(::testing::_))
            .WillOnce(DoAll(::testing::SaveArg<0>(&remote_property_node), ::testing::Return(fep3::Result{})));
    }
    fep3::arya::IConfigurationService* configuration_service = getComponent();
    ASSERT_NE(nullptr, configuration_service);
    EXPECT_EQ(fep3::Result{}, configuration_service->registerNode(mock_property_node));
    // use any method of IPropertyNode to check whether the remote property node refers to the correct local property node
    EXPECT_EQ(test_property_node_name, remote_property_node->getName());
}

/**
 * Test method fep3::IConfigurationService::unregisterNode of a Configuration Service
 * that resides in a C plugin
 * @req_id FEPSDK-2369
 */
TEST_F(ConfigurationServiceLoaderFixture, testMethod_unregisterNode)
{
    std::shared_ptr<fep3::arya::IPropertyNode> remote_property_node;
    const auto& test_property_node_name = std::string("foo");

    // setting of expectations
    {
        auto& mock_configuration_service = getMockComponent();

        EXPECT_CALL(mock_configuration_service, unregisterNode(test_property_node_name))
            .WillOnce(::testing::Return(fep3::Result{}));
    }
    fep3::arya::IConfigurationService* configuration_service = getComponent();
    ASSERT_NE(nullptr, configuration_service);
    EXPECT_EQ(fep3::Result{}, configuration_service->unregisterNode(test_property_node_name));
}

/**
 * Test method fep3::IConfigurationService::isNodeRegistered of a Configuration Service
 * that resides in a C plugin
 * @req_id FEPSDK-2369
 */
TEST_F(ConfigurationServiceLoaderFixture, testMethod_isNodeRegistered)
{
    const auto& test_path = std::string("foo");

    // setting of expectations
    {
        auto& mock_configuration_service = getMockComponent();

        EXPECT_CALL(mock_configuration_service, isNodeRegistered(test_path))
            .WillOnce(::testing::Return(true))
            .WillOnce(::testing::Return(false))
            ;
    }
    fep3::arya::IConfigurationService* configuration_service = getComponent();
    ASSERT_NE(nullptr, configuration_service);
    EXPECT_EQ(true, configuration_service->isNodeRegistered(test_path));
    EXPECT_EQ(false, configuration_service->isNodeRegistered(test_path));
}

/**
 * Test method fep3::IConfigurationService::getNode of a Configuration Service
 * that resides in a C plugin
 * @req_id FEPSDK-2369
 */
TEST_F(ConfigurationServiceLoaderFixture, testMethod_getNode)
{
    const auto& mock_property_node = std::make_shared<::testing::StrictMock<fep3::mock::PropertyNode>>();
    std::shared_ptr<fep3::arya::IPropertyNode> remote_property_node;
    const auto& test_path = std::string("foo");
    const auto& test_property_node_name = std::string("bar");

    // setting of expectations
    {
        EXPECT_CALL(*mock_property_node.get(), getName())
            .WillOnce(::testing::Return(test_property_node_name));

        auto& mock_configuration_service = getMockComponent();

        EXPECT_CALL(mock_configuration_service, getNode(test_path))
            .WillOnce(::testing::Return(mock_property_node))
            .WillOnce(::testing::Return(std::shared_ptr<fep3::arya::IPropertyNode>{}))
            ;
    }
    fep3::arya::IConfigurationService* configuration_service = getComponent();
    ASSERT_NE(nullptr, configuration_service);
    const auto& property_node = configuration_service->getNode(test_path);
    ASSERT_NE(nullptr, property_node);
    EXPECT_EQ(test_property_node_name, property_node->getName());
    EXPECT_EQ(nullptr, configuration_service->getNode(test_path));
}

/**
 * Test method fep3::IConfigurationService::getConstNode of a Configuration Service
 * that resides in a C plugin
 * @req_id FEPSDK-2369
 */
TEST_F(ConfigurationServiceLoaderFixture, testMethod_getConstNode)
{
    const auto& mock_property_node = std::make_shared<::testing::StrictMock<fep3::mock::PropertyNode>>();
    std::shared_ptr<fep3::arya::IPropertyNode> remote_property_node;
    const auto& test_path = std::string("foo");
    const auto& test_property_node_name = std::string("bar");

    // setting of expectations
    {
        EXPECT_CALL(*mock_property_node.get(), getName())
            .WillOnce(::testing::Return(test_property_node_name));

        auto& mock_configuration_service = getMockComponent();

        EXPECT_CALL(mock_configuration_service, getConstNode(test_path))
            .WillOnce(::testing::Return(mock_property_node))
            .WillOnce(::testing::Return(std::shared_ptr<fep3::arya::IPropertyNode>{}))
            ;
    }
    fep3::arya::IConfigurationService* configuration_service = getComponent();
    ASSERT_NE(nullptr, configuration_service);
    const auto& property_node = configuration_service->getConstNode(test_path);
    ASSERT_NE(nullptr, property_node);
    EXPECT_EQ(test_property_node_name, property_node->getName());
    EXPECT_EQ(nullptr, configuration_service->getConstNode(test_path));
}

/**
 * Test IPropertyNode interface through C plugin
 * @req_id FEPSDK-2369
 */
TEST_F(ConfigurationServiceLoaderFixture, testIPropertyNode)
{
    const auto& mock_property_node = std::make_shared<::testing::StrictMock<fep3::mock::PropertyNode>>();
    std::shared_ptr<fep3::arya::IPropertyNode> remote_property_node;
    const auto& test_name = std::string("foo");
    const auto& test_value = std::string("bar");
    const auto& test_type_name = std::string("baz");

    const auto& test_child_name_1 = std::string("foo_child");
    const auto& test_child_name_2 = std::string("bar_child");
    const auto& remote_child_property_node_1 = std::make_shared<::testing::StrictMock<fep3::mock::PropertyNode>>();
    const auto& remote_child_property_node_2 = std::make_shared<::testing::StrictMock<fep3::mock::PropertyNode>>();

    const auto& number_of_children = size_t{3};

    // setting of expectations
    {
        ::testing::InSequence sequence;

        auto& mock_configuration_service = getMockComponent();
        EXPECT_CALL(mock_configuration_service, registerNode(::testing::_))
            .WillOnce(DoAll(::testing::SaveArg<0>(&remote_property_node), ::testing::Return(fep3::Result{})));

        EXPECT_CALL(*mock_property_node.get(), getName())
            .WillOnce(::testing::Return(test_name));
        EXPECT_CALL(*mock_property_node.get(), getValue())
            .WillOnce(::testing::Return(test_value));
        EXPECT_CALL(*mock_property_node.get(), getTypeName())
            .WillOnce(::testing::Return(test_type_name));
        EXPECT_CALL(*mock_property_node.get(), setValueImpl(test_value, test_type_name))
            .WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(*mock_property_node.get(), isEqual(::testing::_))
            .WillOnce(::testing::Return(true))
            .WillOnce(::testing::Return(false))
            ;
        EXPECT_CALL(*mock_property_node.get(), reset())
            .Times(1);
        EXPECT_CALL(*mock_property_node.get(), getChildren())
            .WillOnce(::testing::Return(std::vector<std::shared_ptr<fep3::arya::IPropertyNode>>
                {remote_child_property_node_1
                , remote_child_property_node_2
                }));
        EXPECT_CALL(*remote_child_property_node_1.get(), getName())
            .WillOnce(::testing::Return(test_child_name_1));
        EXPECT_CALL(*remote_child_property_node_2.get(), getName())
            .WillOnce(::testing::Return(test_child_name_2));
        EXPECT_CALL(*mock_property_node.get(), getNumberOfChildren())
            .WillOnce(::testing::Return(number_of_children));
        EXPECT_CALL(*mock_property_node.get(), getChild(test_child_name_1))
            .WillOnce(::testing::Return(remote_child_property_node_1));
        EXPECT_CALL(*remote_child_property_node_1.get(), getName())
            .WillOnce(::testing::Return(test_child_name_1));
        EXPECT_CALL(*mock_property_node.get(), isChild(test_child_name_1))
            .WillOnce(::testing::Return(true))
            .WillOnce(::testing::Return(false))
            ;
    }
    fep3::arya::IConfigurationService* configuration_service = getComponent();
    ASSERT_NE(nullptr, configuration_service);
    // note: we want to test the IPropertyNode interface here, so we can just use any method of the IConfigurationService
    // that transfers a property node through the C interface
    EXPECT_EQ(fep3::Result{}, configuration_service->registerNode(mock_property_node));

    // now test all methods of IPropertyNode
    EXPECT_EQ(test_name, remote_property_node->getName());
    EXPECT_EQ(test_value, remote_property_node->getValue());
    EXPECT_EQ(test_type_name, remote_property_node->getTypeName());
    EXPECT_EQ(fep3::Result{}, remote_property_node->setValue(test_value, test_type_name));
    EXPECT_TRUE(remote_property_node->isEqual(*mock_property_node.get()));
    EXPECT_FALSE(remote_property_node->isEqual(*mock_property_node.get()));
    remote_property_node->reset();
    const auto& children = remote_property_node->getChildren();
    ASSERT_EQ(2, children.size());
    // use any method of IPropertyNode to check whether the remote property node refers to the correct local property node
    EXPECT_EQ(test_child_name_1, (*children.begin())->getName());
    EXPECT_EQ(test_child_name_2, (*(children.begin() + 1))->getName());
    EXPECT_EQ(number_of_children, remote_property_node->getNumberOfChildren());
    const auto& child_1 = remote_property_node->getChild(test_child_name_1);
    EXPECT_EQ(test_child_name_1, child_1->getName());
    EXPECT_TRUE(remote_property_node->isChild(test_child_name_1));
    EXPECT_FALSE(remote_property_node->isChild(test_child_name_1));

    ::testing::Mock::VerifyAndClearExpectations(remote_child_property_node_1.get());
    ::testing::Mock::VerifyAndClearExpectations(remote_child_property_node_2.get());
}
