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

#pragma once

/**
 * Converts the passed parameter to a string
 * param the parameter to convert
 * Example:
 * \#define MY_TEXT foo
 * std::string my_string = FEP3_EXPAND_TO_STRING(MY_TEXT);
 */
#define FEP3_STRING(param) #param

/** Expands the passed macro to a string
 * macro the macro
 * Example:
 * \#define MY_TEXT foo
 * std::string my_string = FEP3_EXPAND_TO_STRING(MY_TEXT);
 *
 * @note If the passed \p macro is not defined this macro will
 * expand to the string literal of the passed macro
 */
#define FEP3_EXPAND_TO_STRING(macro) FEP3_STRING(macro)
