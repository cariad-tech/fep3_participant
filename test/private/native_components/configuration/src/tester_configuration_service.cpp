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
#include <common/gtest_asserts.h>
#include <common/properties_test_helper.h>

#include <fep3/fep3_participant_version.h>
#include <fep3/native_components/configuration/configuration_service.h>
#include <fep3/rpc_services/configuration/configuration_rpc_intf_def.h>
#include <fep3/base/properties/properties.h>
#include <fep3/components/base/component_registry.h>
#include <fep3/components/service_bus/mock/mock_service_bus.h>

using namespace ::testing;
using namespace fep3;

using ServiceBusComponentMock = NiceMock<fep3::mock::ServiceBusComponent>;
using RPCServerMock = NiceMock<fep3::mock::RPCServer>;

struct NativeConfigurationService : public testing::Test
{
    NativeConfigurationService()
        : _component_registry(std::make_shared<fep3::ComponentRegistry>())
        , _service_bus(std::make_shared<ServiceBusComponentMock>())
        , _rpc_server(std::make_shared<RPCServerMock>())
        , _configuration_service_impl(std::make_shared<native::ConfigurationService>())
    {
    }

    void SetUp() override
    {
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(
            _service_bus, _dummy_component_version_info));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(
            _configuration_service_impl, _dummy_component_version_info));

        EXPECT_CALL(*_service_bus, getServer()).Times(1).WillOnce(::testing::Return(_rpc_server));
        EXPECT_CALL(*_rpc_server,
            registerService(fep3::rpc::arya::IRPCConfigurationDef::getRPCDefaultName(),
                _)).Times(1).WillOnce(::testing::Return(fep3::Result()));
                ::testing::Return(Result());

        ASSERT_FEP3_NOERROR(_component_registry->create());

        _configuration_service_intf = _component_registry->getComponent<IConfigurationService>();
    }

    std::shared_ptr<fep3::ComponentRegistry> _component_registry{nullptr};
    std::shared_ptr<ServiceBusComponentMock> _service_bus{ nullptr };
    std::shared_ptr<RPCServerMock> _rpc_server{nullptr};
    std::shared_ptr<fep3::native::ConfigurationService> _configuration_service_impl{ nullptr };
    fep3::IConfigurationService* _configuration_service_intf{ nullptr };
private:
    const fep3::ComponentVersionInfo _dummy_component_version_info{FEP3_PARTICIPANT_LIBRARY_VERSION_STR, "dummyPath", FEP3_PARTICIPANT_LIBRARY_VERSION_STR};
};

/**
 * @brief The method registerNode of the configuration service is tested
 *
 */
TEST(ConfigurationService, registerNode)
{
    fep3::native::ConfigurationService service;
    auto properties_clock = createTestProperties();
    ASSERT_FEP3_NOERROR(service.registerNode(properties_clock));

    std::shared_ptr<fep3::IPropertyNode> props;
    auto node = service.getNode("Clock");
    ASSERT_NE(node, nullptr);

    EXPECT_EQ(node->getChild("Clocks")->getChild("Clock1")->getChild("CycleTime")->getValue(), "1");
}

/**
 * @brief It is tested that with registerNode a node can not be registered twice
 *
 */
TEST(ConfigurationService, registerNodeTwiceFails)
{
    fep3::native::ConfigurationService service;

    auto properties_clock = createTestProperties();
    ASSERT_FEP3_NOERROR(service.registerNode(properties_clock));
    EXPECT_FEP3_RESULT(service.registerNode(properties_clock), ERR_RESOURCE_IN_USE);
}

/**
 * @brief The method unregisterNode of the configuration service is tested
 *
 */
TEST(ConfigurationService, unregisterNode)
{
    fep3::native::ConfigurationService service;

    auto properties_clock = createTestProperties();
    ASSERT_FEP3_NOERROR(service.registerNode(properties_clock));
    ASSERT_FEP3_NOERROR(service.unregisterNode(properties_clock->getName()));

    EXPECT_EQ(service.getNode(properties_clock->getName()), nullptr);
}

/**
 * @brief It is tested that unregisterNode returns an error if the property to unregister does not exist
 *
 */
TEST(ConfigurationService, unregisterNodeNotExisting)
{
    fep3::native::ConfigurationService service;
    ASSERT_FEP3_RESULT(service.unregisterNode("not_existing"), fep3::ERR_NOT_FOUND);
}

/**
 * @brief The method getNode of the configuration service is tested for a property name
 *
 */
TEST(ConfigurationService, getNode)
{
    fep3::native::ConfigurationService service;
    auto properties_clock = createTestProperties();
    service.registerNode(properties_clock);

    EXPECT_NE(service.getConstNode("Clock"), nullptr);

    EXPECT_NE(service.getNode("Clock"), nullptr);
}

/**
 * @brief The method getNode of the configuration service is tested for a property path
 *
 */
TEST(ConfigurationService, getNodeByPath)
{
    fep3::native::ConfigurationService service;
    auto properties_clock = createTestProperties();
    service.registerNode(properties_clock);

    auto get_node_const = service.getConstNode("Clock/Clocks/Clock1");
    ASSERT_NE(get_node_const, nullptr);
    EXPECT_EQ(get_node_const->getName(), "Clock1");

    auto get_node = service.getNode("Clock/Clocks/Clock1");
    ASSERT_NE(get_node, nullptr);
    EXPECT_EQ(get_node->getName(), "Clock1");
}

/**
 * @brief It is tested that getConstNode returns the root node if no path is provided
 * and that getNode returns a nulltpr if no path is provided
 *
 */
TEST(ConfigurationService, getNodeRoot)
{
    fep3::native::ConfigurationService service;
    auto properties_clock = createTestProperties();

    ASSERT_FEP3_NOERROR(service.registerNode(properties_clock));
    ASSERT_FEP3_NOERROR(service.registerNode(std::make_shared<base::NativePropertyNode>("some_node")));

    {
        auto root_node = service.getConstNode();
        ASSERT_NE(root_node, nullptr);

        EXPECT_TRUE(root_node->isChild("some_node"));
        EXPECT_TRUE(root_node->isChild("Clock"));
    }
    {
        std::shared_ptr<IPropertyNode> root_node;
        root_node = service.getNode("");
        EXPECT_EQ(root_node, nullptr);
    }
}


/**
 * @brief The method isNodeRegistered of the configuration service is tested
 *
 */
TEST(ConfigurationService, isNodeRegistered)
{
    fep3::native::ConfigurationService service;
    auto properties_clock = createTestProperties();
    ASSERT_FEP3_NOERROR(service.registerNode(properties_clock));

    {
        EXPECT_FALSE(service.isNodeRegistered(""));
        EXPECT_FALSE(service.isNodeRegistered("/"));
        EXPECT_FALSE(service.isNodeRegistered("not_existing"));
        EXPECT_FALSE(service.isNodeRegistered("not_existing/not_existing"));
        EXPECT_FALSE(service.isNodeRegistered("Clocks"));

        EXPECT_TRUE(service.isNodeRegistered("Clock"));
        EXPECT_TRUE(service.isNodeRegistered("Clock/Clocks"));
        EXPECT_TRUE(service.isNodeRegistered("Clock/Clocks/Clock1"));
        EXPECT_TRUE(service.isNodeRegistered("Clock/Clocks/Clock1/CycleTime"));
        EXPECT_TRUE(service.isNodeRegistered("Clock/Clocks/Clock2"));
    }

    {
        ASSERT_FEP3_NOERROR(service.unregisterNode(properties_clock->getName()));
        EXPECT_FALSE(service.isNodeRegistered("Clock/Clocks"));
        EXPECT_FALSE(service.isNodeRegistered("not_existing"));
    }
}

/**
 * @brief The helper function getPropertyValue taking the configuration service is tested
 *
 */
TEST(PropertiesHelper, getPropertyValue)
{
    fep3::native::ConfigurationService service;
    auto properties_clock = createTestProperties();
    service.registerNode(properties_clock);

    EXPECT_EQ(base::getPropertyValue<int32_t>(service, "Clock/Clocks/Clock1/CycleTime").value(), 1);
    EXPECT_EQ(base::getPropertyValue<int32_t>(service, "Clock/Clocks/Clock2/CycleTime").value(), 2);
    EXPECT_EQ(base::getPropertyValue<std::string>(service, "Clock").value(), "");

    EXPECT_EQ(base::getPropertyValue<int32_t>(service, "").has_value(), false);
    EXPECT_EQ(base::getPropertyValue<int32_t>(service, "/").has_value(), false);
    EXPECT_EQ(base::getPropertyValue<int32_t>(service, "not_existing").has_value(), false);
}

/**
 * @brief The helper function fep3::base::setPropertyValue for a node is tested
 *
 */
TEST(PropertiesHelper, setPropertyValue)
{
   fep3::native::ConfigurationService service;
   auto properties_clock = createTypeTestProperties();
   service.registerNode(properties_clock);

   EXPECT_FEP3_NOERROR(fep3::base::setPropertyValue<int32_t>(service, "types/int", 3));
   EXPECT_EQ(service.getConstNode("types/int")->getValue(), base::DefaultPropertyTypeConversion<int32_t>::toString(3));

   EXPECT_FEP3_RESULT(fep3::base::setPropertyValue<double>(service, "types/int", 3), ERR_INVALID_TYPE);

   EXPECT_FEP3_RESULT(fep3::base::setPropertyValue<double>(service, "types/not_existing", 3), ERR_NOT_FOUND);
   EXPECT_FEP3_RESULT(fep3::base::setPropertyValue<double>(service, "/", 3), ERR_NOT_FOUND);
   EXPECT_FEP3_RESULT(fep3::base::setPropertyValue<double>(service, "", 3), ERR_NOT_FOUND);
}

/**
 * @brief The helper function makeNativePropertyNode is tested
 *
 */
TEST(PropertiesHelper, makeNativePropertyNode)
{
    auto node = fep3::base::makeNativePropertyNode("node_name", 3);
    EXPECT_EQ(node->getValue(), "3");
    EXPECT_EQ(node->getTypeName(), fep3::base::PropertyType<int32_t>::getTypeName());
}