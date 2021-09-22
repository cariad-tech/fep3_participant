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
#include <gtest_asserts.h>
#include <properties_test_helper.h>

using namespace ::testing;
using namespace fep3;

using NativePropertyNode = fep3::base::NativePropertyNode;

const std::string default_type = fep3::base::PropertyType<std::string>::getTypeName();

/**
 * @brief The property name validation upon construction is tested
 *
 */
TEST(NativePropertyNode, propertyNameValidationOnConstruction)
{
    ASSERT_NO_THROW(NativePropertyNode property_name("my_name"));
    ASSERT_THROW(NativePropertyNode property_name("my name"), std::invalid_argument);
}

/**
 * @brief Method isEqual is tested
 *
 */
TEST(NativePropertyNode, isEqual)
{
    const auto main_node_name = "main_node";

    auto property_node = std::make_shared<base::NativePropertyNode>(main_node_name);

    {
        EXPECT_TRUE(NativePropertyNode("my_node", "value", "my_type").isEqual(NativePropertyNode("my_node", "value", "my_type")));
        EXPECT_FALSE(NativePropertyNode("my_node", "value", "my_type").isEqual(NativePropertyNode("my_node", "value_not", "my_type")));
        EXPECT_FALSE(NativePropertyNode("my_node", "value", "my_type").isEqual(NativePropertyNode("my_node", "value", "my_type_not")));
        EXPECT_TRUE(createTestProperties()->isEqual(*createTestProperties()));

        {
            const auto test_properties = createTestProperties();
            EXPECT_FEP3_NOERROR(test_properties->getChild("Clocks")->getChild("Clock1")->getChild("CycleTime")->setValue("2"));
            EXPECT_FALSE(test_properties->isEqual(*createTestProperties()));
        }
    }
}

/**
 * @brief Method setChild is tested for child that is not yet existing
 *
 */
TEST(NativePropertyNode, setChildThatIsNew)
{
    const auto main_node_name = "main_node";
    const auto node_name = "my_property";

    auto property_node = std::make_shared<base::NativePropertyNode>(main_node_name);
    {
        property_node->setChild(std::make_shared<base::NativePropertyNode>(node_name, "value", default_type));

        EXPECT_EQ(property_node->getNumberOfChildren(), 1);
        EXPECT_EQ(property_node->getChild(node_name)->getName(), node_name);
        EXPECT_EQ(property_node->getChild(node_name)->getValue(), "value");
        EXPECT_EQ(property_node->getChild(node_name)->getTypeName(), default_type.c_str());
    }
}

/**
 * @brief Method setChild is tested for child that is already existing.
 * The already existing child has to be overridden.
 *
 */
TEST(NativePropertyNode, setChildThatIsExisting)
{
    const auto main_node_name = "main_node";
    const auto node_name = "my_property";

    auto property_node = std::make_shared<base::NativePropertyNode>(main_node_name);
    {
        setChildImpl(property_node, std::make_shared<base::NativePropertyNode>(node_name, "value_old", default_type))
            ->setChild(std::make_shared<base::NativePropertyNode>("my_child_property", "value_child", default_type));

        auto property_to_add = std::make_shared<base::NativePropertyNode>(node_name, "value_new", default_type);
        property_node->setChild(property_to_add);

        EXPECT_TRUE(property_node->getChild(node_name)->isEqual(*property_to_add));
    }
}

/**
 * @brief The methods getName(), getValue(), getType() are tested
 *
 */
TEST(NativePropertyNode, getNameValueType)
{
    const auto test_properties = createTestProperties();
    EXPECT_EQ(test_properties->getName(), "Clock");
    EXPECT_EQ(test_properties->getValue(), "");
    EXPECT_EQ(test_properties->getTypeName(), base::PropertyType<base::NodePropertyType>::getTypeName().c_str());

    const auto cycle_time_node = test_properties->getChild("Clocks")->getChild("Clock1")->getChild("CycleTime");
    EXPECT_EQ(cycle_time_node->getName(), "CycleTime");
    EXPECT_EQ(cycle_time_node->getValue(), "1");
    EXPECT_EQ(cycle_time_node->getTypeName(), fep3::base::PropertyType<int32_t>::getTypeName().c_str());
}

/**
 * @brief The methods getChild(), getNumberOfChildren(), getChildren() are tested
 *
 */
TEST(NativePropertyNode, getChildRelatedMethods)
{
    const auto test_properties = createTestProperties();

    EXPECT_NE(test_properties->getChild("Clocks"), std::shared_ptr<IPropertyNode>());
    EXPECT_EQ(test_properties->getChild("Clocks_not"), std::shared_ptr<IPropertyNode>());

    const auto clocks_node = test_properties->getChild("Clocks");
    EXPECT_EQ(clocks_node->getNumberOfChildren(), 2);
    EXPECT_EQ(clocks_node->getChildren().at(0)->getName(), "Clock1");
    EXPECT_EQ(clocks_node->getChildren().at(1)->getName(), "Clock2");
}

/**
 * @brief The method copyDeepFrom is tested
 *
 */
TEST(NativePropertyNode, copyDeepFrom)
{
    const auto copy_source = createTestProperties();
    NativePropertyNode copy_target("some_name");

    {
    ASSERT_FALSE(copy_target.isEqual(*copy_source));
    copy_target.copyDeepFrom(*copy_source);
    EXPECT_TRUE(copy_target.isEqual(*copy_source));
    }
}


/**
* @brief The method setProperty is tested
*
*/
TEST(NativePropertyNode, setProperty)
{
    const auto main_node_name = "main_node";

    auto properties = NativePropertyNode(main_node_name, "value", "old_type");
    ASSERT_FEP3_NOERROR(properties.setValue("new_value", "old_type"));

    EXPECT_EQ(properties.getTypeName(), "old_type");
    EXPECT_EQ(properties.getValue(), "new_value");
}


struct Observer : public fep3::base::IPropertyObserver
{
    MOCK_METHOD1(onUpdate, void(fep3::IPropertyNode&));
};

/**
* @brief The observer registration and updating of an observer is tested
* @req_id
*/
TEST(NativePropertyNode, observerRegisterAndUpdate)
{
    const auto property_node_name = "name";

    auto observer = std::make_shared<Observer>();
    EXPECT_CALL(*observer, onUpdate(_)).WillOnce(Invoke([](fep3::IPropertyNode& property) {
        EXPECT_EQ(property.getValue(), "new_value");
    }));

    {
        auto property_node = NativePropertyNode(property_node_name, "old_value", default_type);
        property_node.registerObserver(observer);

        ASSERT_FEP3_NOERROR(property_node.setValue("new_value"));
        property_node.updateObservers();

        EXPECT_EQ(property_node.getValue(), "new_value");
    }
}

/**
 * @brief The unregistration of an observer is tested
 *
 */
TEST(NativePropertyNode, observerUnregister)
{
    const auto property_node_name = "name";

    auto observer = std::make_shared<Observer>();
    /// we expect only one call because of the unregistration
    EXPECT_CALL(*observer, onUpdate(_)).WillOnce(Return());
    {
        auto property_node = NativePropertyNode(property_node_name, "old_value", default_type);

        // set value when observer registered
        {
            property_node.registerObserver(observer);
            EXPECT_FEP3_NOERROR(property_node.setValue("old_value"));
            property_node.updateObservers();
        }

        // set value when observer not registered
        {
            property_node.unregisterObserver(observer);
            EXPECT_FEP3_NOERROR(property_node.setValue("new_value"));
            property_node.updateObservers();
        }
    }
}

/**
 * @brief It is tested that a PropertyVariable can be created with all supported no array types.
 *
 */
TEST(PropertyVariable, PropertyVariableTypes)
{
    base::PropertyVariable<double> variable_1 = 0.0;
    base::PropertyVariable<int32_t> variable_3 = 0;
    base::PropertyVariable<std::string> variable_4 = std::string("");
    base::PropertyVariable<bool> variable_5 = true;
    /// success it it compiles
    EXPECT_TRUE(true);
}

struct PropertyVariableRegistrationFixture
    : public ::testing::Test
{
    PropertyVariableRegistrationFixture()
    {
        property_node = std::make_shared<base::NativePropertyNode>(main_node_name
            , base::DefaultPropertyTypeConversion<double>::toString(init_value)
            , base::PropertyType<double>::getTypeName());
    }

    double init_value = 0.0;
    double new_value = 1.0;
    const std::string main_node_name = "main_node";
    std::shared_ptr<base::NativePropertyNode> property_node;
};

/**
 * @brief The method registerVariable is tested
 *
 */
TEST_F(PropertyVariableRegistrationFixture, registerVariable)
{
    base::PropertyVariable<double> variable = init_value;
    ASSERT_FEP3_NOERROR(property_node->registerVariable(variable));

    EXPECT_EQ(base::getPropertyValue<double>(*property_node), init_value);
    fep3::base::setPropertyValue<double>(*property_node, new_value);
    property_node->updateObservers();

    EXPECT_EQ(static_cast<double>(variable), new_value);
}

/**
 * @brief The method registerVariable is tested for the case that the variable has different type than the property
 * In this case an error is expected.
 *
 */
TEST_F(PropertyVariableRegistrationFixture, registerVariableWrongType)
{
    base::PropertyVariable<int32_t> variable = 1;
    EXPECT_FEP3_RESULT(property_node->registerVariable(variable), ERR_INVALID_TYPE);
}

/**
 * @brief The method registerVariable is tested for the case that a child node has to be created
 *
 */
TEST_F(PropertyVariableRegistrationFixture, registerVariableAsChild)
{
    const auto child_name = "child";

    base::PropertyVariable<double> variable = init_value;
    ASSERT_FEP3_NOERROR(property_node->registerVariable(variable, child_name));

    const auto created_child = property_node->getChild(child_name);

    EXPECT_EQ(base::getPropertyValue<double>(*created_child), init_value);
    fep3::base::setPropertyValue<double>(*created_child, new_value);
    property_node->updateObservers();

    EXPECT_EQ(static_cast<double>(variable), new_value);
}

/**
 * @brief The method registerVariable is tested for the case that a child node is already existing
 * The property value has to be updated on registration.
 *
 */
TEST_F(PropertyVariableRegistrationFixture, registerVariableAsChildChildExisting)
{
    const auto child_name = "child";

    property_node->setChild(std::make_shared<base::NativePropertyNode>(child_name
        , base::DefaultPropertyTypeConversion<double>::toString(init_value)
        , base::PropertyType<double>::getTypeName()));

    const auto new_init_value = 2.0;
    base::PropertyVariable<double> variable = new_init_value;

    // register variable and check that the value of the property was set
    ASSERT_FEP3_NOERROR(property_node->registerVariable(variable, child_name));
    const auto created_child = property_node->getChild(child_name);
    EXPECT_EQ(base::getPropertyValue<double>(*created_child), new_init_value);

    // set new value and update observer
    fep3::base::setPropertyValue<double>(*created_child, new_value);
    property_node->updateObservers();
    EXPECT_EQ(static_cast<double>(variable), new_value);
}

/**
 * @brief The method unregisterVariable is tested
 *
 */
TEST_F(PropertyVariableRegistrationFixture, unregisterVariable)
{
    base::PropertyVariable<double> variable = init_value;
    ASSERT_FEP3_NOERROR(property_node->registerVariable(variable));
    ASSERT_FEP3_NOERROR(property_node->unregisterVariable(variable));

    EXPECT_EQ(base::getPropertyValue<double>(*property_node), init_value);
    fep3::base::setPropertyValue<double>(*property_node, new_value);
    property_node->updateObservers();

    EXPECT_EQ(static_cast<double>(variable), init_value);
}

/**
 * @brief The method unregisterVariable for a child node is tested
 *
 */
TEST_F(PropertyVariableRegistrationFixture, unregisterVariableAsChild)
{
    const auto child_name = "child";

    base::PropertyVariable<double> variable = init_value;
    ASSERT_FEP3_NOERROR(property_node->registerVariable(variable, child_name));
    ASSERT_FEP3_NOERROR(property_node->unregisterVariable(variable, child_name));

    const auto created_child = property_node->getChild(child_name);

    EXPECT_EQ(base::getPropertyValue<double>(*created_child), init_value);
    fep3::base::setPropertyValue<double>(*created_child, new_value);
    property_node->updateObservers();

    EXPECT_EQ(static_cast<double>(variable), init_value);
}

/**
 * @brief The method unregisterVariable for a child node is tested if the child is not existing
 *
 */
TEST_F(PropertyVariableRegistrationFixture, unregisterVariableAsChildWhichIsNotExisting)
{
    const auto child_name = "child";

    base::PropertyVariable<double> variable = init_value;
    EXPECT_FEP3_RESULT(property_node->unregisterVariable(variable, child_name), ERR_NOT_FOUND);
}