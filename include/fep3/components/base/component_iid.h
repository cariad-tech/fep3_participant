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

#ifndef _FEP3_COMPONENTS_H_
#define _FEP3_COMPONENTS_H_

/**
 * @brief Macro defines a static internal function.
 * @param[in] iid the interface identifier as string which is use for the getComponentIID functionality of the @ref fep3::arya::IComponents registry
 *
 */
#define FEP_COMPONENT_IID(iid) \
static constexpr const char* const FEP3_COMP_IID = iid; \
static const char* getComponentIID() \
{                        \
    return iid;          \
}

/**
* @brief Macro defines a static internal function.
* @param[in] iid the interface identifier as string which is use for the getComponentIID functionality of the @ref fep3::arya::IComponents registry
*
*/
#define FEP3_COMPONENT_IID(iid) FEP_COMPONENT_IID(iid)

#endif // _FEP3_COMPONENTS_H_

