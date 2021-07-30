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

#include <fep3/base/properties/propertynode_helper.h>

using namespace ::testing;
using namespace fep3;

template<typename T>
using PropertyType = fep3::base::PropertyType<T>;

/**
 * @brief The helper method validatePropertyName is tested
 *
 */
TEST(PropertiesHelper, propertyNameValidationOnConstruction)
{
    EXPECT_NO_THROW(base::validatePropertyName("my_name"));
    EXPECT_NO_THROW(base::validatePropertyName("myname2"));

    EXPECT_THROW(base::validatePropertyName("my-name"), std::invalid_argument);
    EXPECT_THROW(base::validatePropertyName("my name"), std::invalid_argument);
    EXPECT_THROW(base::validatePropertyName(""), std::invalid_argument);
    EXPECT_THROW(base::validatePropertyName("t/est"), std::invalid_argument);
    EXPECT_THROW(base::validatePropertyName("t.est"), std::invalid_argument);

    EXPECT_NO_THROW(base::validatePropertyName("validp_roperty"));
    EXPECT_NO_THROW(base::validatePropertyName("VALIDPROPERTY"));
    EXPECT_NO_THROW(base::validatePropertyName("v"));
    EXPECT_NO_THROW(base::validatePropertyName("property2"));
    EXPECT_NO_THROW(base::validatePropertyName("2property"));
    EXPECT_NO_THROW(base::validatePropertyName("superlongvalidpropertywithalotoftext"));

    EXPECT_THROW(base::validatePropertyName(""), std::invalid_argument);
    EXPECT_THROW(base::validatePropertyName("invalid property"), std::invalid_argument);
    EXPECT_THROW(base::validatePropertyName("invalid/property"), std::invalid_argument);
    EXPECT_THROW(base::validatePropertyName("invalid\\property"), std::invalid_argument);
    EXPECT_THROW(base::validatePropertyName("invalid.property"), std::invalid_argument);
}

/**
 * @brief The helper method base::setPropertyValue is tested
 *
 */
TEST(PropertiesHelper, setPropertyValue)
{
    {
        auto property_node = std::make_shared<base::NativePropertyNode>("my_node", a_util::strings::toString(0), PropertyType<int32_t>::getTypeName());
        ASSERT_FEP3_NOERROR(base::setPropertyValue<int32_t>(*property_node, 2));
        EXPECT_EQ(property_node->getValue(), a_util::strings::toString(2));
    }
    {
        auto property_node = std::make_shared<base::NativePropertyNode>("my_node", a_util::strings::toString(1.0), PropertyType<double>::getTypeName());
        ASSERT_FEP3_NOERROR(base::setPropertyValue<double>(*property_node, 2.0));
        EXPECT_EQ(property_node->getValue(), a_util::strings::toString(2.0));
    }
    {
        auto property_node = std::make_shared<base::NativePropertyNode>("my_node", a_util::strings::toString(false), PropertyType<bool>::getTypeName());
        ASSERT_FEP3_NOERROR(base::setPropertyValue<bool>(*property_node, true));
        EXPECT_EQ(property_node->getValue(), a_util::strings::toString(true));
    }
    {
        auto property_node = std::make_shared<base::NativePropertyNode>("my_node", "old_val", PropertyType<std::string>::getTypeName());
        ASSERT_FEP3_NOERROR(base::setPropertyValue<std::string>(*property_node, "new_val"));
        EXPECT_EQ(property_node->getValue(), "new_val");
    }
}

/**
 * @brief The helper method base::setPropertyValue is tested for array types
 *
 */
TEST(PropertiesHelper, setPropertyValue_arrayTypes)
{
    {
        const auto value = std::vector<int32_t>({ 1,2,3 });
        auto property_node = std::make_shared<base::NativePropertyNode>("my_node", "", PropertyType<std::vector<int32_t>>::getTypeName());
        ASSERT_FEP3_NOERROR(base::setPropertyValue<std::vector<int32_t>>(*property_node, value));
        EXPECT_EQ(property_node->getValue(), base::DefaultPropertyTypeConversion<std::vector<int32_t>>::toString(value));
    }

    {
        const auto value = std::vector<double>({ 1.0,2.1,3.2 });
        auto property_node = std::make_shared<base::NativePropertyNode>("my_node", "", PropertyType<std::vector<double>>::getTypeName());
        ASSERT_FEP3_NOERROR(base::setPropertyValue<std::vector<double>>(*property_node, value));
        EXPECT_EQ(property_node->getValue(), base::DefaultPropertyTypeConversion<std::vector<double>>::toString(value));
    }

    {
        const auto value = std::vector<bool>({ true, false, true });
        auto property_node = std::make_shared<base::NativePropertyNode>("my_node", "", PropertyType<std::vector<bool>>::getTypeName());
        ASSERT_FEP3_NOERROR(base::setPropertyValue<std::vector<bool>>(*property_node, value));
        EXPECT_EQ(property_node->getValue(), base::DefaultPropertyTypeConversion<std::vector<bool>>::toString(value));
    }

    {
        const auto value = std::vector<std::string>({ "ab", "cd", "ef" });
        auto property_node = std::make_shared<base::NativePropertyNode>("my_node", "", PropertyType<std::vector<std::string>>::getTypeName());
        ASSERT_FEP3_NOERROR(base::setPropertyValue<std::vector<std::string>>(*property_node, value));
        EXPECT_EQ(property_node->getValue(), base::DefaultPropertyTypeConversion<std::vector<std::string>>::toString(value));
    }
}

/**
 * @brief The helper method base::setPropertyValue is tested with a different type.
 * The base::setPropertyValue will try to set a different type than the node was created with. An Error is returned
 */
TEST(PropertiesHelper, setPropertyValue_differentType)
{
    {
        auto property_node = std::make_shared<base::NativePropertyNode>("my_node", "some_string", PropertyType<std::string>::getTypeName());
        ASSERT_FEP3_RESULT(base::setPropertyValue<int32_t>(*property_node, 2), ERR_INVALID_TYPE);
        EXPECT_EQ(property_node->getValue(), "some_string");
    }

    {
        auto property_node = std::make_shared<base::NativePropertyNode>("my_node", "0.0", PropertyType<double>::getTypeName());
        ASSERT_FEP3_RESULT(base::setPropertyValue<int32_t>(*property_node, 2), ERR_INVALID_TYPE);
        EXPECT_EQ(property_node->getValue(), "0.0");
    }
}

/**
 * @brief The helper method base::getPropertyValue is tested
 *
 */
TEST(PropertiesHelper, getPropertValue)
{
    EXPECT_EQ(1
        , base::getPropertyValue<int32_t>(
            *std::make_shared<base::NativePropertyNode>("my_node"
                , a_util::strings::toString(1)
                , PropertyType<int32_t>::getTypeName())));

    EXPECT_EQ(1.1
        , base::getPropertyValue<double>(
            *std::make_shared<base::NativePropertyNode>("my_node"
                , a_util::strings::toString(1.1)
                , PropertyType<double>::getTypeName())));

    EXPECT_EQ("my_val"
        , base::getPropertyValue<std::string>(
            *std::make_shared<base::NativePropertyNode>("my_node"
                , "my_val"
                , PropertyType<std::string>::getTypeName())));

    EXPECT_EQ(false
        , base::getPropertyValue<bool>(
            *std::make_shared<base::NativePropertyNode>("my_node"
                , a_util::strings::toString(false)
                , PropertyType<bool>::getTypeName())));
}

/**
 * @brief The helper method base::getPropertyValue is tested with a wrong type.
 * It is tested that if base::getPropertyValue is called with a non convertible type, the default value for that type is returned
 */
TEST(PropertiesHelper, getPropertyValue_wrongType)
{
    auto string_property = std::make_shared<base::NativePropertyNode>("my_node"
        , "some_value"
        , PropertyType<std::string>::getTypeName());

    EXPECT_EQ(0.0, base::getPropertyValue<double>(*string_property));
    EXPECT_EQ(0, base::getPropertyValue<int32_t>(*string_property));
    EXPECT_EQ(false, base::getPropertyValue<bool>(*string_property));
    EXPECT_EQ(std::vector<std::string>({"some_value"}), base::getPropertyValue<std::vector<std::string>>(*string_property));
}


/**
 * @brief  The helper method base::getPropertyValue is tested with the array types
 *
 */
TEST(PropertiesHelper, getProperty_arrayTypes)
{
    EXPECT_EQ(std::vector<int32_t>({ 1,2,3 }),
        base::getPropertyValue<std::vector<int32_t>>(
            *std::make_shared<base::NativePropertyNode>("my_node"
                , base::DefaultPropertyTypeConversion<std::vector<int32_t>>::toString({ 1,2,3 })
                , PropertyType<std::vector<int32_t>>::getTypeName())));

    EXPECT_EQ(std::vector<bool>({ true,false,true }),
        base::getPropertyValue<std::vector<bool>>(
            *std::make_shared<base::NativePropertyNode>("my_node"
                , base::DefaultPropertyTypeConversion<std::vector<bool>>::toString({ true,false,true })
                , PropertyType<std::vector<bool>>::getTypeName())));

    EXPECT_EQ(std::vector<double>({ 1.2,2.3,3.4 }),
        base::getPropertyValue<std::vector<double>>(
            *std::make_shared<base::NativePropertyNode>("my_node"
                , base::DefaultPropertyTypeConversion<std::vector<double>>::toString({ 1.2,2.3,3.4 })
                , PropertyType<std::vector<double>>::getTypeName())));

    EXPECT_EQ(std::vector<std::string>({ "ab","cd" }),
        base::getPropertyValue<std::vector<std::string>>(
            *std::make_shared<base::NativePropertyNode>("my_node"
            , base::DefaultPropertyTypeConversion<std::vector<std::string>>::toString({ "ab","cd" })
                , PropertyType<std::vector<std::string>>::getTypeName())));
}


