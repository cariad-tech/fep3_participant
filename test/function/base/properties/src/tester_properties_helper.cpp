/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <gtest_asserts.h>
#include <properties_test_helper.h>

using namespace ::testing;
using namespace fep3;

template <typename T>
using PropertyType = fep3::base::PropertyType<T>;

/**
 * @brief The helper method validatePropertyName is tested
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
 */
TEST(PropertiesHelper, setPropertyValue)
{
    {
        auto property_node = std::make_shared<base::NativePropertyNode>("my_node", int32_t{0});
        ASSERT_FEP3_NOERROR(base::setPropertyValue<int32_t>(*property_node, 2));
        EXPECT_EQ(property_node->getValue(), std::to_string(2));
    }
    {
        auto property_node = std::make_shared<base::NativePropertyNode>("my_node", double{1.0});
        ASSERT_FEP3_NOERROR(base::setPropertyValue<double>(*property_node, 2.0));
        EXPECT_EQ(property_node->getValue(), std::to_string(2.0));
    }
    {
        auto property_node = std::make_shared<base::NativePropertyNode>("my_node", false);
        ASSERT_FEP3_NOERROR(base::setPropertyValue<bool>(*property_node, true));
        EXPECT_EQ(property_node->getValue(), "true");
    }
    {
        auto property_node =
            std::make_shared<base::NativePropertyNode>("my_node", std::string{"old_val"});
        ASSERT_FEP3_NOERROR(base::setPropertyValue<std::string>(*property_node, "new_val"));
        EXPECT_EQ(property_node->getValue(), "new_val");
    }
    {
        auto property_node = std::make_shared<base::NativePropertyNode>("my_node", uint32_t{0});
        ASSERT_FEP3_NOERROR(base::setPropertyValue<uint32_t>(*property_node, 2));
        EXPECT_EQ(property_node->getValue(), std::to_string(2));
    }
    {
        auto property_node = std::make_shared<base::NativePropertyNode>("my_node", uint64_t{0});
        ASSERT_FEP3_NOERROR(base::setPropertyValue<uint64_t>(*property_node, 2));
        EXPECT_EQ(property_node->getValue(), std::to_string(2));
    }
}

/**
 * @brief The helper method base::setPropertyValue is tested for array types
 */
TEST(PropertiesHelper, setPropertyValue_arrayTypes)
{
    {
        const auto value = std::vector<int32_t>({1, 2, 3});
        auto property_node =
            std::make_shared<base::NativePropertyNode>("my_node", std::vector<int32_t>{});
        ASSERT_FEP3_NOERROR(base::setPropertyValue<std::vector<int32_t>>(*property_node, value));
        EXPECT_EQ(property_node->getValue(),
                  base::DefaultPropertyTypeConversion<std::vector<int32_t>>::toString(value));
    }

    {
        const auto value = std::vector<double>({1.0, 2.1, 3.2});
        auto property_node =
            std::make_shared<base::NativePropertyNode>("my_node", std::vector<double>{});
        ASSERT_FEP3_NOERROR(base::setPropertyValue<std::vector<double>>(*property_node, value));
        EXPECT_EQ(property_node->getValue(),
                  base::DefaultPropertyTypeConversion<std::vector<double>>::toString(value));
    }

    {
        const auto value = std::vector<bool>({true, false, true});
        auto property_node =
            std::make_shared<base::NativePropertyNode>("my_node", std::vector<bool>{});
        ASSERT_FEP3_NOERROR(base::setPropertyValue<std::vector<bool>>(*property_node, value));
        EXPECT_EQ(property_node->getValue(),
                  base::DefaultPropertyTypeConversion<std::vector<bool>>::toString(value));
    }

    {
        const auto value = std::vector<std::string>({"ab", "cd", "ef"});
        auto property_node =
            std::make_shared<base::NativePropertyNode>("my_node", std::vector<std::string>{});
        ASSERT_FEP3_NOERROR(
            base::setPropertyValue<std::vector<std::string>>(*property_node, value));
        EXPECT_EQ(property_node->getValue(),
                  base::DefaultPropertyTypeConversion<std::vector<std::string>>::toString(value));
    }
    {
        const auto value = std::vector<uint32_t>({1, 2, 3});
        auto property_node =
            std::make_shared<base::NativePropertyNode>("my_node", std::vector<uint32_t>{});

        ASSERT_FEP3_NOERROR(base::setPropertyValue<std::vector<uint32_t>>(*property_node, value));
        EXPECT_EQ(property_node->getValue(),
                  base::DefaultPropertyTypeConversion<std::vector<uint32_t>>::toString(value));
    }
    {
        const auto value = std::vector<uint64_t>({1, 2, 3});
        auto property_node =
            std::make_shared<base::NativePropertyNode>("my_node", std::vector<uint64_t>{});
        ASSERT_FEP3_NOERROR(base::setPropertyValue<std::vector<uint64_t>>(*property_node, value));
        EXPECT_EQ(property_node->getValue(),
                  base::DefaultPropertyTypeConversion<std::vector<uint64_t>>::toString(value));
    }
}

/**
 * @brief The helper method base::setPropertyValue is tested with a different type.
 * The base::setPropertyValue will try to set a different type than the node was created with. An
 * Error is returned
 */
TEST(PropertiesHelper, setPropertyValue_differentType)
{
    {
        auto property_node =
            std::make_shared<base::NativePropertyNode>("my_node", std::string{"some_string"});
        ASSERT_FEP3_RESULT(base::setPropertyValue<int32_t>(*property_node, 2), ERR_INVALID_TYPE);
        EXPECT_EQ(property_node->getValue(), "some_string");
    }

    {
        auto property_node = std::make_shared<base::NativePropertyNode>("my_node", double{0.0});
        ASSERT_FEP3_RESULT(base::setPropertyValue<int32_t>(*property_node, 2), ERR_INVALID_TYPE);
        EXPECT_EQ(property_node->getValue(),
                  base::DefaultPropertyTypeConversion<double>::toString(double{0.0}));
    }
}

/**
 * @brief The helper method base::getPropertyValue is tested
 */
TEST(PropertiesHelper, getPropertValue)
{
    EXPECT_EQ(1,
              base::getPropertyValue<int32_t>(
                  *std::make_shared<base::NativePropertyNode>("my_node", int32_t{1})));

    EXPECT_EQ(1.1,
              base::getPropertyValue<double>(
                  *std::make_shared<base::NativePropertyNode>("my_node", double{1.1})));

    EXPECT_EQ("my_val",
              base::getPropertyValue<std::string>(
                  *std::make_shared<base::NativePropertyNode>("my_node", std::string{"my_val"})));

    EXPECT_EQ(false,
              base::getPropertyValue<bool>(
                  *std::make_shared<base::NativePropertyNode>("my_node", false)));

    EXPECT_EQ(static_cast<uint32_t>(2),
              base::getPropertyValue<uint32_t>(
                  *std::make_shared<base::NativePropertyNode>("my_node", uint32_t{2})));

    EXPECT_EQ(static_cast<uint64_t>(3),
              base::getPropertyValue<uint64_t>(
                  *std::make_shared<base::NativePropertyNode>("my_node", uint64_t{3})));
}

/**
 * @brief The helper method base::getPropertyValue is tested with a wrong type.
 * It is tested that if base::getPropertyValue is called with a non convertible type, the default
 * value for that type is returned
 */
TEST(PropertiesHelper, getPropertyValue_wrongType)
{
    auto string_property =
        std::make_shared<base::NativePropertyNode>("my_node", std::string{"some_value"});

    EXPECT_EQ(0.0, base::getPropertyValue<double>(*string_property));
    EXPECT_EQ(0, base::getPropertyValue<int32_t>(*string_property));
    EXPECT_EQ(false, base::getPropertyValue<bool>(*string_property));
    EXPECT_EQ(std::vector<std::string>({"some_value"}),
              base::getPropertyValue<std::vector<std::string>>(*string_property));
}

/**
 * @brief  The helper method base::getPropertyValue is tested with the array types
 */
TEST(PropertiesHelper, getProperty_arrayTypes)
{
    EXPECT_EQ(
        std::vector<int32_t>({1, 2, 3}),
        base::getPropertyValue<std::vector<int32_t>>(
            *std::make_shared<base::NativePropertyNode>("my_node", std::vector<int32_t>{1, 2, 3})));

    EXPECT_EQ(std::vector<bool>({true, false, true}),
              base::getPropertyValue<std::vector<bool>>(*std::make_shared<base::NativePropertyNode>(
                  "my_node", std::vector<bool>{true, false, true})));

    EXPECT_EQ(
        std::vector<double>({1.2, 2.3, 3.4}),
        base::getPropertyValue<std::vector<double>>(*std::make_shared<base::NativePropertyNode>(
            "my_node", std::vector<double>{1.2, 2.3, 3.4})));

    EXPECT_EQ(std::vector<std::string>({"ab", "cd"}),
              base::getPropertyValue<std::vector<std::string>>(
                  *std::make_shared<base::NativePropertyNode>(
                      "my_node", std::vector<std::string>{"ab", "cd"})));

    EXPECT_EQ(
        std::vector<uint32_t>({1, 2, 3}),
        base::getPropertyValue<std::vector<uint32_t>>(*std::make_shared<base::NativePropertyNode>(
            "my_node", std::vector<uint32_t>{1, 2, 3})));

    EXPECT_EQ(
        std::vector<uint64_t>({1, 2, 3}),
        base::getPropertyValue<std::vector<uint64_t>>(*std::make_shared<base::NativePropertyNode>(
            "my_node", std::vector<uint64_t>{1, 2, 3})));
}

/**
 * @brief  Tests DefaultPropertyTypeConversion toString method
 */
TEST(PropertiesHelper, test_DefaultPropertyTypeConversion_toString)
{
    EXPECT_EQ("true", fep3::base::DefaultPropertyTypeConversion<bool>::toString(true));
    EXPECT_EQ("165", fep3::base::DefaultPropertyTypeConversion<int32_t>::toString(165));
    EXPECT_EQ("858", fep3::base::DefaultPropertyTypeConversion<int64_t>::toString(858));
    EXPECT_EQ("165", fep3::base::DefaultPropertyTypeConversion<uint32_t>::toString(165));
    EXPECT_EQ("858", fep3::base::DefaultPropertyTypeConversion<uint64_t>::toString(858));
    EXPECT_EQ("1.340000", fep3::base::DefaultPropertyTypeConversion<double>::toString(1.34));
    EXPECT_EQ("testValue",
              fep3::base::DefaultPropertyTypeConversion<std::string>::toString("testValue"));

    EXPECT_EQ("true;false;true",
              fep3::base::DefaultPropertyTypeConversion<std::vector<bool>>::toString(
                  {true, false, true}));
    EXPECT_EQ("165;6554;-323;-987412",
              fep3::base::DefaultPropertyTypeConversion<std::vector<int32_t>>::toString(
                  {165, 6554, -323, -987412}));
    EXPECT_EQ("165;6554;-323;-987412",
              fep3::base::DefaultPropertyTypeConversion<std::vector<int64_t>>::toString(
                  {165, 6554, -323, -987412}));
    EXPECT_EQ("165;6554;323;987412",
              fep3::base::DefaultPropertyTypeConversion<std::vector<uint32_t>>::toString(
                  {165, 6554, 323, 987412}));
    EXPECT_EQ("165;6554;323;987412",
              fep3::base::DefaultPropertyTypeConversion<std::vector<uint64_t>>::toString(
                  {165, 6554, 323, 987412}));
    EXPECT_EQ(
        "1.340000;-2.850000",
        fep3::base::DefaultPropertyTypeConversion<std::vector<double>>::toString({1.34, -2.85}));
    EXPECT_EQ("testValue;testValue2",
              fep3::base::DefaultPropertyTypeConversion<std::vector<std::string>>::toString(
                  {"testValue", "testValue2"}));

    EXPECT_EQ("",
              fep3::base::DefaultPropertyTypeConversion<std::vector<std::string>>::toString({}));
    EXPECT_EQ("", fep3::base::DefaultPropertyTypeConversion<std::vector<uint32_t>>::toString({}));
}

/**
 * @brief  Tests DefaultPropertyTypeConversion fromString method
 */
TEST(PropertiesHelper, test_DefaultPropertyTypeConversion_fromString)
{
    EXPECT_EQ(true, fep3::base::DefaultPropertyTypeConversion<bool>::fromString("true"));
    EXPECT_EQ(165, fep3::base::DefaultPropertyTypeConversion<int32_t>::fromString("165"));
    EXPECT_EQ(858, fep3::base::DefaultPropertyTypeConversion<int64_t>::fromString("858"));
    EXPECT_EQ(static_cast<uint32_t>(165),
              fep3::base::DefaultPropertyTypeConversion<uint32_t>::fromString("165"));
    EXPECT_EQ(static_cast<uint64_t>(858),
              fep3::base::DefaultPropertyTypeConversion<uint64_t>::fromString("858"));
    EXPECT_DOUBLE_EQ(1.34, fep3::base::DefaultPropertyTypeConversion<double>::fromString("1.34"));
    EXPECT_EQ("testValue",
              fep3::base::DefaultPropertyTypeConversion<std::string>::toString("testValue"));

    EXPECT_EQ(std::vector<bool>({true, false, true}),
              fep3::base::DefaultPropertyTypeConversion<std::vector<bool>>::fromString(
                  "true;false;true"));
    EXPECT_EQ(std::vector<int32_t>({165, 6554, -323, -987412}),
              fep3::base::DefaultPropertyTypeConversion<std::vector<int32_t>>::fromString(
                  "165;6554;-323;-987412"));
    EXPECT_EQ(std::vector<int64_t>({165, 6554, -323, -987412}),
              fep3::base::DefaultPropertyTypeConversion<std::vector<int64_t>>::fromString(
                  "165;6554;-323;-987412"));
    EXPECT_EQ(std::vector<uint32_t>({165, 6554, 323, 987412}),
              fep3::base::DefaultPropertyTypeConversion<std::vector<uint32_t>>::fromString(
                  "165;6554;323;987412"));
    EXPECT_EQ(std::vector<uint64_t>({165, 6554, 323, 987412}),
              fep3::base::DefaultPropertyTypeConversion<std::vector<uint64_t>>::fromString(
                  "165;6554;323;987412"));

    auto string_vector = fep3::base::DefaultPropertyTypeConversion<std::vector<double>>::fromString(
        "1.340000;-2.850000");
    decltype(string_vector) expected_vector = {1.34, -2.85};
    for (size_t i = 0; i < string_vector.size(); i++) {
        EXPECT_DOUBLE_EQ(string_vector.at(i), expected_vector.at(i));
    }

    EXPECT_EQ(std::vector<std::string>({"testValue", "testValue2"}),
              fep3::base::DefaultPropertyTypeConversion<std::vector<std::string>>::fromString(
                  "testValue;testValue2"));
}

/**
 * @brief  Tests PropertyType getTypeName method
 */
TEST(PropertiesHelper, getTypeName)
{
    EXPECT_EQ("bool", fep3::base::PropertyType<bool>::getTypeName());
    EXPECT_EQ("int32", fep3::base::PropertyType<int32_t>::getTypeName());
    EXPECT_EQ("int64", fep3::base::PropertyType<int64_t>::getTypeName());
    EXPECT_EQ("uint32", fep3::base::PropertyType<uint32_t>::getTypeName());
    EXPECT_EQ("uint64", fep3::base::PropertyType<uint64_t>::getTypeName());
    EXPECT_EQ("double", fep3::base::PropertyType<double>::getTypeName());
    EXPECT_EQ("string", fep3::base::PropertyType<std::string>::getTypeName());

    EXPECT_EQ("array-bool", fep3::base::PropertyType<std::vector<bool>>::getTypeName());
    EXPECT_EQ("array-int32", fep3::base::PropertyType<std::vector<int32_t>>::getTypeName());
    EXPECT_EQ("array-int64", fep3::base::PropertyType<std::vector<int64_t>>::getTypeName());
    EXPECT_EQ("array-uint32", fep3::base::PropertyType<std::vector<uint32_t>>::getTypeName());
    EXPECT_EQ("array-uint64", fep3::base::PropertyType<std::vector<uint64_t>>::getTypeName());
    EXPECT_EQ("array-double", fep3::base::PropertyType<std::vector<double>>::getTypeName());
    EXPECT_EQ("array-string", fep3::base::PropertyType<std::vector<std::string>>::getTypeName());
}
