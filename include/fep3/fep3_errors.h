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

#ifndef _FEP3_ERRORS_H
#define _FEP3_ERRORS_H

#include <fep3/fep3_result_decl.h>

// #include <a_util/result/result_type.h>
#include <a_util/result/error_def.h>

namespace fep3 {
// deprecated since 3.2, but kept for backwards compatibility
/// @copydoc a_util::result::is_ok
using a_util::result::isOk;

// deprecated since 3.2, but kept for backwards compatibility
/// @copydoc a_util::result::is_failed
using a_util::result::isFailed;

// TODO has to be removed once ODAUTIL-168 is fixed
#ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
#endif

/// @cond nodoc
inline constexpr auto ERR_NOERROR = a_util::result::SUCCESS;
_MAKE_RESULT(-2, ERR_UNKNOWN);
_MAKE_RESULT(-3, ERR_UNEXPECTED);
_MAKE_RESULT(-4, ERR_POINTER);
_MAKE_RESULT(-5, ERR_INVALID_ARG);
_MAKE_RESULT(-7, ERR_INVALID_ADDRESS);
_MAKE_RESULT(-11, ERR_INVALID_FILE);
_MAKE_RESULT(-12, ERR_MEMORY);
_MAKE_RESULT(-13, ERR_TIMEOUT);
_MAKE_RESULT(-15, ERR_RESOURCE_IN_USE);
_MAKE_RESULT(-16, ERR_NOT_IMPL);
_MAKE_RESULT(-17, ERR_NO_INTERFACE);
_MAKE_RESULT(-19, ERR_NOT_SUPPORTED);
_MAKE_RESULT(-20, ERR_NOT_FOUND);
_MAKE_RESULT(-21, ERR_CANCELLED);
_MAKE_RESULT(-22, ERR_RETRY);
_MAKE_RESULT(-23, ERR_FILE_NOT_FOUND);
_MAKE_RESULT(-24, ERR_PATH_NOT_FOUND);
_MAKE_RESULT(-25, ERR_ACCESS_DENIED);
_MAKE_RESULT(-31, ERR_BAD_DEVICE);
_MAKE_RESULT(-32, ERR_DEVICE_IO);
_MAKE_RESULT(-33, ERR_DEVICE_NOT_READY);
_MAKE_RESULT(-35, ERR_NOT_CONNECTED);
_MAKE_RESULT(-36, ERR_UNKNOWN_FORMAT);
_MAKE_RESULT(-37, ERR_NOT_INITIALISED);
_MAKE_RESULT(-38, ERR_FAILED);
_MAKE_RESULT(-40, ERR_INVALID_STATE);
_MAKE_RESULT(-41, ERR_EXCEPTION_RAISED);
_MAKE_RESULT(-42, ERR_INVALID_TYPE);
_MAKE_RESULT(-43, ERR_EMPTY);
_MAKE_RESULT(-49, ERR_OUT_OF_RANGE);
_MAKE_RESULT(-50, ERR_KNOWN_PROBLEM);
/// @endcond

#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif
} // namespace fep3

/**
 * @brief Compares two result objects
 * @param[in] lhs the left hand side result
 * @param[in] rhs the right hand side result
 * @return @c lhs if lhs is an error result, @c rhs otherwise
 */
inline fep3::Result operator|(const fep3::Result& lhs, const fep3::Result& rhs)
{
    return !lhs ? lhs : rhs;
}

/**
 * @brief Compares two result objects and assigns the resulting object
 * @param[in,out] lhs the left hand side result
 * @param[in] rhs the right hand side result
 * @return lhs if lhs is an error result, @c rhs otherwise
 */
inline fep3::Result& operator|=(fep3::Result& lhs, const fep3::Result& rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

/// Provide legacy RETURN_IF_FAILED implementation
#ifndef FEP3_RETURN_IF_FAILED
    #define FEP3_RETURN_IF_FAILED(s)                                                               \
        {                                                                                          \
            fep3::Result _errcode(s);                                                              \
            if (!_errcode) {                                                                       \
                return (_errcode);                                                                 \
            }                                                                                      \
        }
#endif

/// using a printf like parameter list for detailed error description
#ifndef CREATE_ERROR_DESCRIPTION
    #define CREATE_ERROR_DESCRIPTION(_errcode, ...)                                                \
        a_util::result::Result(_errcode,                                                           \
                               a_util::strings::format(__VA_ARGS__).c_str(),                       \
                               __LINE__,                                                           \
                               __FILE__,                                                           \
                               A_UTIL_CURRENT_FUNCTION)
#endif

#endif // _FEP3_ERRORS_H
