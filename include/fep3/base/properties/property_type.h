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

// Guideline - FEP System Library API Exception
#ifndef _FEP3_COMP_PROPERTY_TYPE_H_
#define _FEP3_COMP_PROPERTY_TYPE_H_

#include <fep3/base/template_helpers/template_helpers.h>

#include <array>
#include <string>
#include <vector>

namespace fep3 {
namespace base {
namespace arya {
/**
 * @brief
 *
 * @tparam T The Type to define the type conversion for
 */
template <typename T, class U = void>
struct DefaultPropertyTypeConversion {
    /**
     * @brief static function to deserialize the value from string (utf-8) to the typed
     * representation @p T.
     *
     * @param[in] from the value as string
     * @return T the value as type @p T.
     */
    inline static T fromString(const std::string& from)
    {
        return from;
    }

    /**
     * @brief static function to serialize the value of type T to a string (utf-8).
     *
     * @param[in] value the value as type @p T.
     * @return std::string the value as string
     */
    inline static std::string toString(const T& value)
    {
        return value;
    }
};

/**
 * @brief The property type template can be used to define the type description name within @ref
 * fep3::arya::IProperties
 *
 * @tparam T The type to retrieve the type name from. Default typed are defined in @ref
 * allowed_property_types
 */

struct NodePropertyType {
};

/// alias indicating the allowed property types
using allowed_property_types =
    std::tuple<bool, int32_t, int64_t, uint32_t, uint64_t, double, std::string, NodePropertyType>;

/// array containing the allowed property type names
static constexpr std::array<const char*, std::tuple_size<allowed_property_types>::value>
    property_type_names = {
        "bool", "int32", "int64", "uint32", "uint64", "double", "string", "node"};

/**
 * @brief Trait for checking if a type is an allowed property type.
 *
 * @tparam U the type to chek for
 */
template <typename U>
struct IsPropertyType {
    /*

    could be directly the following function but there is a bug in earyl VS compiler that does
    create compile error when this constexpr function is used in SFINATE

    template<typename U>
    constexpr auto IsPropertyType() {
    return TupleContains<U, allowed_property_types>::value;
    }
    */

    ///@cond nodoc
    static const bool value = TupleContains<U, allowed_property_types>::value;
    ///@endcond nodoc
};

/**
 * @brief The property type template can be used to define the type description name within @ref
 * fep3::arya::IProperties
 *
 * @tparam T The type to retrieve the type name from. See the default types @ref
 * allowed_property_types.
 *
 * @tparam U Type used for SFINAE.
 */
template <typename T, typename U = void>
struct PropertyType {
    /**
     * @brief gets the type name for the type @p T
     *
     *  @return std::string the type name for the type @p T
     */
    static constexpr const char* getTypeName()
    {
        return "";
    }
};

///@cond nodoc
/**
 * @brief This is a partial specialization that gets instantiated for the types defined in @ref
 * allowed_property_types.
 *
 * @tparam T The type to retrieve the type name from.
 */
template <typename T>
struct PropertyType<T, typename std::enable_if_t<IsPropertyType<T>::value>> {
    /* could be constexpr static char*, but in case of vector we return string, so we have the same
     * return type*/
    /**
     * @brief gets the type name for the type @p T.
     *
     * @return std::string the type name for the type @p T
     */
    static std::string getTypeName()
    {
        return property_type_names[TupleIndex<T, allowed_property_types>::value];
    }
};

/**
 * @brief This is a partial specialization that gets instantiated for vectors of types defined in
 * @ref allowed_property_types.
 *
 * @tparam T Is the type contained in the vector type to retrieve the type name from.
 */
template <typename T>
struct PropertyType<std::vector<T>, typename std::enable_if_t<IsPropertyType<T>::value>> {
    // no constexpr std string until c++20, could use string_view in c++17 to make it constexpr
    // static
    /**
     * @brief gets the type name for the type @p T
     *
     * @return std::string the type name for the type @p T
     */
    static std::string getTypeName()
    {
        return std::string("array-") +
               property_type_names[TupleIndex<T, allowed_property_types>::value];
    }
};
///@endcond nodoc

/**
 * @brief Implementation class to represent a typed value of T can be used for @ref
 * fep3::arya::IProperties
 *
 * @tparam T the type
 * @tparam PROP_TYPE the type name type
 * @tparam PROP_TYPECONVERSION the type conversion type to serialize and deserialize to/from a
 * string representation
 */
template <typename T,
          typename PROP_TYPE = PropertyType<T>,
          typename PROP_TYPECONVERSION = DefaultPropertyTypeConversion<T>>
class PropertyValue {
public:
    /**
     * @brief Construct a new Property Value object of type T with default
     */
    PropertyValue() : _value(T())
    {
    }

    /**
     * @brief Construct a new Property Value object of type T
     *
     * @param[in] value
     */
    PropertyValue(T value) : _value(std::move(value))
    {
    }

    /**
     * @brief Get the Type name
     *
     * @return std::string the type name in this case described by the type PROP_TYPE
     */
    std::string getTypeName()
    {
        return PROP_TYPE::getTypeName();
    }
    /**
     * @brief converts the value T to a string
     *
     * @return std::string the converted value as string in this case described by the type
     * PROP_TYPECONVERSION
     */
    std::string toString() const
    {
        return PROP_TYPECONVERSION::toString(_value);
    }

    /**
     * @brief operator const T&
     *
     * @return const T&
     */
    operator const T&() const
    {
        return getReference();
    }

    /**
     * @brief operator==
     *
     * @param[in] other @p T to compare for equality
     * @return @c true if equal, @c false otherwise
     */
    bool operator==(const T& other)
    {
        return getReference() == other;
    }

    /**
     * @brief operator!=
     *
     * @param[in] other @p T to compare for unequality
     *
     * @return @c true if not equal, @c false otherwise
     */
    bool operator!=(const T& other)
    {
        return getReference() != other;
    }

protected:
    /**
     * @brief setter for the value
     *
     * @param[in] value Value to set
     */
    void setValue(T value)
    {
        _value = value;
    }

private:
    /**
     * @brief Get const reference to property value
     *
     * @return const T&
     */
    const T& getReference() const
    {
        return reinterpret_cast<const T&>(_value);
    }

    /**
     * @brief the value
     */
    T _value;
};

} // namespace arya
using arya::DefaultPropertyTypeConversion;
using arya::NodePropertyType;
using arya::PropertyType;
using arya::PropertyValue;
} // namespace base
} // namespace fep3

#endif //_FEP3_COMP_PROPERTIES_INTF_H_
