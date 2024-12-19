/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

// Guideline - FEP System Library API Exception
#ifndef _FEP3_TEMPLATE_HELPERS_H_
    #define _FEP3_TEMPLATE_HELPERS_H_

    #include <tuple>

///@cond nodoc
namespace fep3 {
namespace base {

/**
 * @brief The index of a type contained in a tuple. This is the base template that should not be
 * instantiated when Tuple is not a std::tuple. Will return a compile error if tuple does not
 * contain the type.
 *
 * @tparam T the type to look in the tuple for
 * @tparam Tuple the type of the Tuple
 */
template <class T, class Tuple>
struct TupleIndex;

/**
 * @brief Instantianted in the first iteration.
 *
 * @tparam T the type to look in the tuple for
 * @tparam Types the types contained in the Tuple
 */
template <class T, class... Types>
struct TupleIndex<T, std::tuple<T, Types...>> {
    /// member containing the result.
    static const std::size_t value = 0;
};

/**
 * @brief Instantianted in the recursive iteration.
 *
 * @tparam T the type to look in the tuple for
 * @tparam U the first type contained in the tuple.
 * @tparam Types the rest types contained in the Tuple
 */
template <class T, class U, class... Types>
struct TupleIndex<T, std::tuple<U, Types...>> {
    /// member containing the result.
    static const std::size_t value = 1 + TupleIndex<T, std::tuple<Types...>>::value;
};

/**
 * @brief Trait checking if tuple contains a type. This is the base template that should not be
 * instantiated when @p Tuple is not a std::tuple
 *
 * @tparam T the type to look in the tuple for
 * @tparam Tuple the type of the Tuple
 */
template <typename T, typename Tuple>
struct TupleContains;

/**
 * @brief Partial specialization for tuples.
 *
 * @tparam U the type to look in the tuple for
 * @tparam T the types contained in the the Tuple
 */
template <typename U, typename... T>
struct TupleContains<U, std::tuple<T...>> {
    /// member containing the result.
    static const bool value =
        !std::is_same<std::integer_sequence<bool, false, std::is_same<U, T>::value...>,
                      std::integer_sequence<bool, std::is_same<U, T>::value..., false>>::value;

    // in c++17 we can do
    // (std::is_same_v<U, T> || ...); or  std::disjunction_v<std::is_same<U, T>...>;
};
} // namespace base
} // namespace fep3
#endif // _FEP3_TEMPLATE_HELPERS_H_
///@endcond nodoc
