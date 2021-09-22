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

#include <map>

#include <fep3/fep3_errors.h>
#include <fep3/base/properties/propertynode.h>
#include <fep3/components/job_registry/job_registry_intf.h>
#include <fep3/components/job_registry/job_configuration.h>
#include <functional>

namespace fep3
{
namespace native
{

using JobConfigurations = std::map<std::string, JobConfiguration>;

/**
* @brief Read job configurations from a property node containing job nodes.
*
* @param[in] jobs_node Property node containing job configuration entries
* @param[out] job_configurations job configurations output
* @return fep3::Result
* @retval ERR_NOERROR Everything went fine.
* @retval ERR_NOT_FOUND A required property node is missing. E.g. a job or configuration node.
* @retval ERR_INVALID_ARG A property node contains invalid configuration values.
*/
Result readJobConfigurationsFromPropertyNode(
        const base::NativePropertyNode& jobs_node,
        JobConfigurations& job_configurations);

} // namespace native
} // namespace fep3


