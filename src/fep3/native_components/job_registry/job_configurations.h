/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/base/properties/propertynode.h>
#include <fep3/components/job_registry/job_registry_intf.h>

namespace fep3 {
namespace native {

using JobConfigurations = std::map<std::string, fep3::arya::JobConfiguration>;

using JobConfigurationPtrs =
    std::map<std::string, std::unique_ptr<fep3::catelyn::JobConfiguration>>;

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
Result readJobConfigurationsFromPropertyNode(const base::NativePropertyNode& jobs_node,
                                             JobConfigurations& job_configurations);

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
Result readJobConfigurationsFromPropertyNode(const base::NativePropertyNode& jobs_node,
                                             JobConfigurationPtrs& job_configurations);

} // namespace native
} // namespace fep3
