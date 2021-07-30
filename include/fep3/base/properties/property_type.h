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

//Guideline - FEP System Library API Exception
#ifndef _FEP3_COMP_PROPERTY_TYPE_H_
#define _FEP3_COMP_PROPERTY_TYPE_H_

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
template<typename T>
struct DefaultPropertyTypeConversion
{
    /**
     * @brief static function to deserialize the value from string (utf-8) to the typed representation T
     *
     * @param[in] from the value as string
     * @return T the value as type
     */
    inline static T fromString(const std::string& from)
    {
        return from;
    }
    /**
     * @brief static function to serialize the value of type T to a string (utf-8)
     *
     * @param[in] value the value as type
     * @return std::string the value as string
     */
    inline static std::string toString(const T& value)
    {
        return value;
    }
};

/**
 * @brief The property type template can be used to define the type description name within @ref fep3::arya::IProperties
 *
 * @tparam T The type to retrieve the type name from. See the default types:
 *           \li @ref PropertyType<bool>
 *           \li @ref PropertyType<int32_t>
 *           \li @ref PropertyType<int64_t>
 *           \li @ref PropertyType<double>
 *           \li @ref PropertyType<std::string>
 *           \li @ref PropertyType<std::vector<bool>>
 *           \li @ref PropertyType<std::vector<int32_t>>
 *           \li @ref PropertyType<std::vector<int64_t>>
 *           \li @ref PropertyType<std::vector<double>>
 *           \li @ref PropertyType<std::vector<std::string>>
 *
 */
template<typename T>
struct PropertyType
{
    /**
     * @brief gets the type name for the type T
     *
     * @return std::string the type name for the type \p T
     */
    inline static std::string getTypeName()
    {
        return "";
    }
};

/**
 * @brief spezialized PropertyType for type bool. Can be used for @ref fep3::arya::IProperties.
 *
 */
template<>
struct PropertyType<bool>
{
    /**
     * @brief gets the type name for the type bool
     *
     * @return std::string the type name for the type \p bool
     */
    inline static std::string getTypeName()
    {
        return "bool";
    }
};

/**
 * @brief spezialized PropertyType for type int32_t. Can be used for @ref fep3::arya::IProperties.
 *
 */
template<>
struct PropertyType<int32_t>
{
    /**
     * @brief gets the type name for the type int32_t
     *
     * @return std::string the type name for the type \p int32_t
     */
    static std::string getTypeName()
    {
        return "int32";
    }
};

/**
 * @brief spezialized PropertyType for type int64_t. Can be used for @ref fep3::arya::IProperties.
 *
 */
template<>
struct PropertyType<int64_t>
{
    /**
     * @brief gets the type name for the type int64_t
     *
     * @return std::string the type name for the type \p int64_t
     */
    static std::string getTypeName()
    {
        return "int64";
    }
};

/**
 * @brief spezialized PropertyType for type double. Can be used for @ref fep3::arya::IProperties.
 *
 */
template<>
struct PropertyType<double>
{
    /**
     * @brief gets the type name for the type double
     *
     * @return std::string the type name for the type \p double
     */
    static std::string getTypeName()
    {
        return "double";
    }
};

/**
 * @brief spezialized PropertyType for type std::string. Can be used for @ref fep3::arya::IProperties.
 *
 */
template<>
struct PropertyType<std::string>
{
    /**
     * @brief gets the type name for the type std::string
     *
     * @return std::string the type name for the type \p std::string
     */
    inline static std::string getTypeName()
    {
        return "string";
    }
};

/**
 * @brief spezialized PropertyType for type std::vector<bool>. Can be used for @ref fep3::arya::IProperties.
 *
 */
template<>
struct PropertyType<std::vector<bool>>
{
    /**
     * @brief gets the type name for the type std::vector<bool>
     *
     * @return std::string the type name for the type \p std::vector<bool>
     */
    static std::string getTypeName()
    {
        return "array-bool";
    }
};

/**
 * @brief spezialized PropertyType for type std::vector<int32_t>. Can be used for @ref fep3::arya::IProperties.
 *
 */
template<>
struct PropertyType<std::vector<int32_t>>
{
    /**
     * @brief gets the type name for the type std::vector<int32_t>
     *
     * @return std::string the type name for the type \p std::vector<int32_t>
     */
    static std::string getTypeName()
    {
        return "array-int32";
    }
};

/**
 * @brief spezialized PropertyType for type std::vector<int64_t>. Can be used for @ref fep3::arya::IProperties.
 *
 */
template<>
struct PropertyType<std::vector<int64_t>>
{
    /**
     * @brief gets the type name for the type std::vector<int64_t>
     *
     * @return std::string the type name for the type \p std::vector<int64_t>
     */
    static std::string getTypeName()
    {
        return "array-int64";
    }
};

/**
 * @brief spezialized PropertyType for type std::vector<double>. Can be used for @ref fep3::arya::IProperties.
 *
 */
template<>
struct PropertyType<std::vector<double>>
{
    /**
     * @brief gets the type name for the type std::vector<double>
     *
     * @return std::string the type name for the type \p std::vector<double>
     */
    static std::string getTypeName()
    {
        return "array-double";
    }
};

/**
 * @brief spezialized PropertyType for type std::vector<std::string>. Can be used for @ref fep3::arya::IProperties.
 *
 */
template<>
struct PropertyType<std::vector<std::string>>
{
    /**
     * @brief gets the type name for the type std::vector<std::string>
     *
     * @return std::string the type name for the type \p std::vector<std::string>
     */
    static std::string getTypeName()
    {
        return "array-string";
    }
};

struct NodePropertyType
{};

/**
 * @brief spezialized PropertyType for type NodePropertyType. Can be used for @ref fep3::arya::IProperties.
 * The type is is used for a property node that has no value but only children nodes.
 *
 */
template<>
struct PropertyType<NodePropertyType>
{
    /**
     * @brief gets the type name for the type NodePropertyType
     *
     * @return std::string the type name for the type \p NodePropertyType
     */
    static std::string getTypeName()
    {
        return "node";
    }
};

/**
 * @brief Implementation class to represent a typed value of T can be used for @ref fep3::arya::IProperties
 *
 * @tparam T the type
 * @tparam PROP_TYPE the type name type
 * @tparam PROP_TYPECONVERSION the type conversion type to serialize and deserialize to/from a string representation
 */
template<typename T, typename PROP_TYPE = PropertyType< T >, typename PROP_TYPECONVERSION = DefaultPropertyTypeConversion< T >>
class PropertyValue
{
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
     * @return std::string the converted value as string in this case described by the type PROP_TYPECONVERSION
     */
    std::string toString() const
    {
        return PROP_TYPECONVERSION::toString(_value);
    }
    /**

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