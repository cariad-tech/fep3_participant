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

#include <fep3/base/properties/property_type.h>

#include <a_util/strings/strings_convert.h>
#include <a_util/strings/strings_format.h>
#include <a_util/strings/strings_functions.h>

#include <algorithm>
#include <iterator>
#include <sstream>

namespace fep3 {
namespace base {
namespace arya {

/**
 * @brief Alias for defining a function taking a const std::string& and returning @p T.
 *
 * @tparam T the return type of the conversion function.
 */
template <typename T>
using conversion_function_type = T (*)(const std::string&);

/**
 * @brief Dummy function that takes string as input and returns it.
 *        defined since there is (logically) no a_util::strings::toString
 *
 * @param[in] s string returned from the function.
 *
 * @return std::string the string passed in the input.
 */
inline std::string stringPropertyToString(const std::string& s)
{
    return s;
}

/**
 * @brief Tuple defining the conversion functions from types to string.
 */
static constexpr auto func_covertors =
    std::make_tuple(static_cast<conversion_function_type<bool>>(a_util::strings::toBool),
                    static_cast<conversion_function_type<int32_t>>(a_util::strings::toInt32),
                    static_cast<conversion_function_type<int64_t>>(a_util::strings::toInt64),
                    static_cast<conversion_function_type<uint32_t>>(a_util::strings::toUInt32),
                    static_cast<conversion_function_type<uint64_t>>(a_util::strings::toUInt64),
                    static_cast<conversion_function_type<double>>(a_util::strings::toDouble),
                    stringPropertyToString);
///@cond nodoc
/**
 * @brief Trait checking if there is conversion function from string to type @p T in  @link
 * func_covertors @endlink.
 *
 * @tparam T Is the type contained in the vector type to retrieve the type name from.
 */
template <typename T>
struct has_from_string_conversion_function
    : TupleContains<conversion_function_type<T>, std::remove_cv_t<decltype(func_covertors)>> {
};

/**
 * @brief Trait checking if there is an a_util::strings::toString function that takes type @p A as
 * argument. This gets instantiated if the type @p A is not found.
 * @tparam A The type that  a_util::strings::toString should take as input argument.
 */
template <typename A, typename = void>
struct hasToStringConversion : std::false_type {
};

/**
 * @brief Trait checking if there is an a_util::strings::toString function that takes type @p A as
 * argument. This gets instantiated if the type @p A is found.
 * @tparam A The type that  a_util::strings::toString should take as input argument.
 */
template <typename A>
struct hasToStringConversion<A,
                             decltype((void)static_cast<std::string (*)(A)>(
                                 a_util::strings::toString))> : std::true_type {
};

/**
 * @brief Conversion structured that gets enabled only if the type @p T is available as input
 * function in one of the @link func_covertors @endlink.
 *
 * @tparam T The Type to define the type conversion for.
 */
template <class T>
struct DefaultPropertyTypeConversion<
    T,
    std::enable_if_t<has_from_string_conversion_function<T>::value>> {
    /**
     * alias for defining a conversion function from string to type @p T.
     */
    using intern_conversion_function_type = T (*)(const std::string&);

    /**
     * @brief static function to deserialize the value from string (utf-8) to the typed
     * representation @p T.
     *
     * @param[in] from the value as string.
     * @return T the value as type @p T.
     */
    inline static T fromString(const std::string& from)
    {
        static constexpr auto intern_conversion_function =
            std::get<intern_conversion_function_type>(func_covertors);
        return (*intern_conversion_function)(from);
    }

    /**
     * @brief static function to serialize the value of type @p T to a string (utf-8).
     *
     * @param[in] from the value as type @p T
     * @return std::string the value as string
     */
    inline static std::string toString(const T& from)
    {
        return toStringImpl(from);
    }

private:
    /**
     * @brief static function to serialize the value of type @p T to a string (utf-8).
     *        This will get instantiated if there is an a_util::strings::toString function that
     * takes type @p U as argument and type @p U is the same as @p T. SFINAE here allows to define
     * toStringImpl overloads for types not supported by a_util::strings::toString.
     *
     * @tparam U The Type to define the type conversion for.
     *
     * @param[in] value the value as type @p U.
     * @return std::string the value as string
     */
    template <typename U>
    inline static std::enable_if_t<hasToStringConversion<U>::value && std::is_same<T, U>::value,
                                   std::string>
    toStringImpl(const U& value)
    {
        return a_util::strings::toString(value);
    }

    /**
     * @brief static function to serialize the value of type @p T to a string (utf-8).
     *        This will get instantiated if @p T and @p U are of std::string type.
     *
     * @tparam U The Type to define the type conversion for.
     *
     * @param[in] value the value as type
     * @return std::string the value as string
     */
    template <typename U>
    inline static std::enable_if_t<std::is_same<U, std::string>::value && std::is_same<T, U>::value,
                                   std::string>
    toStringImpl(const U& value)
    {
        return value;
    }
};
///@endcond nodoc

/**
 * @brief Trait checking if DefaultPropertyTypeConversion has a function toString that takes as
 * argument type @p A. This gets instantiated if the type @p A is not found.
 * @tparam A The type that DefaultPropertyTypeConversion:toString should take as input argument.
 */
template <typename A, typename = void>
struct hasDefaultProperyToStringConversion : std::false_type {
};
///@cond nodoc
/**
 * @brief Trait checking if DefaultPropertyTypeConversion has a function toString that takes as
 * argument type @p A. This gets instantiated if the type @p A is not found.
 * @tparam A The type that DefaultPropertyTypeConversion:toString should take as input argument.
 */
template <typename A>
struct hasDefaultProperyToStringConversion<A,
                                           decltype((void)static_cast<std::string (*)(const A&)>(
                                               DefaultPropertyTypeConversion<A>::toString))>
    : std::true_type {
};
///@endcond nodoc

/**
 * @brief Conversion structured that gets enabled for vector of type @p T only if the type @p T of
 * the vector is available as input function in one of the @link func_covertors @endlink.
 *
 * @tparam T The Type of the vector.
 */
template <class T>
struct DefaultPropertyTypeConversion<
    std::vector<T>,
    std::enable_if_t<has_from_string_conversion_function<T>::value>> {
    /**
     * @brief static function to deserialize the value from string (utf-8) to the typed
     * representation std::vector<T>
     *
     * @param[in] from the value as string
     * @return std::vector<T> the value as std::vector<T>
     */
    inline static std::vector<T> fromString(const std::string& from)
    {
        std::vector<std::string> split_vector_value = a_util::strings::split(from, ";");
        std::vector<T> return_value(split_vector_value.size());

        std::transform(split_vector_value.begin(),
                       split_vector_value.end(),
                       return_value.begin(),
                       DefaultPropertyTypeConversion<T>::fromString);
        return return_value;
    }

    /**
     * @brief static function to serialize the value of type std::vector<T> to a string (utf-8).
     *
     * @param[in] from the value as std::vector<T>
     * @return std::string the value as string
     */
    inline static std::string toString(const std::vector<T>& from)
    {
        return toStringImpl(from);
    }

private:
    ///@cond nodoc
    /**
     * @brief static function to serialize the value of type std::vector<U> to a string.
     *        The function will be only instantiated if the type @p U can be an  input argument to
     *        DefaultPropertyTypeConversion<U>::toString, so that overloads of toStringImpl can be
     *        implememted for other non supported types.
     * @param[in] from the value as std::vector<T>
     * @return std::string the value as string
     */
    template <typename U>
    inline static std::enable_if_t<hasDefaultProperyToStringConversion<U>::value &&
                                       std::is_same<T, U>::value,
                                   std::string>
    toStringImpl(const std::vector<U>& from)
    {
        if (from.empty()) {
            return "";
        }
        else {
            std::stringstream imploded;
            std::transform(
                from.begin(),
                from.end(),
                std::ostream_iterator<std::string>(imploded, ";"),
                [](const U& u) { return DefaultPropertyTypeConversion<U>::toString(u); });

            std::string result_string = imploded.str();
            result_string.pop_back();

            return result_string;
        }
    }
};
///@endcond nodoc

} // namespace arya
using arya::DefaultPropertyTypeConversion;
} // namespace base
} // namespace fep3
