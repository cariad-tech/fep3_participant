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

/**
 * Enables 'deprecated code' warnings for linux and windows platforms.
 * Include after code block which the warning has been disabled for.
 * Example usage:
 * \#include enable_deprecation_warning.h
 * @remark Has to be used in combination with disable_deprecation_warning.h
 * @remark For information regarding when to use this file and when to disable 'deprecated code'
 *         warning in the FEP context, please have a look at the FEP programming guidelines.
 */
#ifdef __GNUC__
    #pragma GCC diagnostic pop
#elif _MSC_VER
    #pragma warning(pop)
#endif