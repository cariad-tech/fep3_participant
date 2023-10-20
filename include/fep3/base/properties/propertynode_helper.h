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

#pragma once

#include <fep3/base/properties/property_type_conversion.h>
#include <fep3/components/configuration/configuration_service_intf.h>
#include <fep3/fep3_errors.h>
#include <fep3/fep3_optional.h>

#include <a_util/strings/strings_format.h>

#include <regex>

namespace fep3 {
namespace base {
namespace arya {

/**
 * @brief Validate the given @p property_name.
 * A regex is used to validate the name.
 *
 * @param[in] property_name Name of the property
 */
inline void validatePropertyName(const std::string& property_name)
{
    using namespace a_util::strings;
    const auto regex = "^[a-zA-Z0-9_]+$";
    const std::regex regex_object(regex);
    if (!std::regex_match(property_name, regex_object)) {
        throw std::invalid_argument(
            format("The property name '%s' is not valid. It has to comply with the regex '%s'.",
                   property_name.c_str(),
                   regex));
    }
}

/**
 * @brief Set the value of the @p property_node to @p value in a typed way.
 * The @p value will be converted to string and stored in this @p property_node.
 * By default only these types are supported: @ref fep3::base::arya::PropertyType<T>.
 *
 * @tparam T Type of the @p property_node
 * @param[in] property_node Node to set the value for
 * @param[in] value The value to set
 * @return fep3::Result
 * @retval ERR_INVALID_TYPE if @p property_node is of different type than @p T and no conversion is
 * implemented
 */
template <typename T>
fep3::Result setPropertyValue(fep3::arya::IPropertyNode& property_node, T value)
{
    return property_node.setValue(base::arya::DefaultPropertyTypeConversion<T>::toString(value),
                                  base::arya::PropertyType<T>::getTypeName());
}

/**
 * @brief Set the @p value for a property node with this @p property_path in a typed way.
 * The provided configuration service will be searched for the property with @p property_path.
 * See @ref setPropertyValue(fep3::arya::IPropertyNode&, T) for details about the value to be set.
 *
 * @tparam T Type of the property with path @p property_path.
 * @param[in] config_service The configuration service of the fep element
 * @param[in] property_path The property path of the property as registered with the @p
 * config_service (e.g. Clock/CycleTime).
 * @param[in] value The value to set
 * @return fep3::Result
 * @retval ERR_NOT_FOUND if no property with this @p property_path was found
 * @retval ERR_INVALID_TYPE if property @p property_path is of different type than @p T and no
 * conversion is implemented
 */
template <typename T>
fep3::Result setPropertyValue(fep3::arya::IConfigurationService& config_service,
                              const std::string& property_path,
                              T value)
{
    auto node = config_service.getNode(property_path);
    if (!node) {
        RETURN_ERROR_DESCRIPTION(
            ERR_NOT_FOUND, "A property with path '%s' was not found", property_path.c_str());
    }
    return setPropertyValue<T>(*node, value);
}

/**
 * @brief Get the value of the @p property_node in a typed way.
 * If the property value cannot be represented by @p T a default value is returned.
 * See implementation of @ref fep3::base::arya::DefaultPropertyTypeConversion<T>::fromString() for
 * used @p T. By default only these types are supported: @ref fep3::base::arya::PropertyType<T>.
 *
 * @tparam T Type of the @p property_node
 * @param[in] property_node Property to get value from
 * @return Value of type @p T
 */
template <typename T>
T getPropertyValue(const fep3::arya::IPropertyNode& property_node)
{
    return base::arya::DefaultPropertyTypeConversion<T>::fromString(property_node.getValue());
}

/**
 * @brief Get the value of the property node with @p property_path in a typed way.
 * The provided configuration service will be searched for the property node.
 * See @ref fep3::base::arya::getPropertyValue(const fep3::arya::IPropertyNode&) for detail.
 *
 * @tparam T Type of the property with @p property_path.
 * @param[in] config_service The configuration service of the fep element
 * @param[in] property_path Path of the property (e.g. Clock/CycleTime).
 * @return If the node cannot be found, an empty @ref fep3::arya::Optional<T> is returned
 */
template <typename T>
fep3::arya::Optional<T> getPropertyValue(const fep3::arya::IConfigurationService& config_service,
                                         const std::string& property_path)
{
    auto result_value = fep3::arya::Optional<T>();
    auto node = config_service.getNode(property_path);
    if (node) {
        result_value = getPropertyValue<T>(*node);
    }

    return result_value;
}

} // namespace arya

using arya::getPropertyValue;
using arya::setPropertyValue;
using arya::validatePropertyName;

} // namespace base
} // namespace fep3
