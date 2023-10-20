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

#include <fep3/base/component_registry/component_registry.h>
#include <fep3/components/logging/logging_service_intf.h>

namespace fep3 {
namespace arya {
/**
 * @brief class wit static factory function for creating a component registry.
 */

class ComponentRegistryFactory {
public:
    /**
     * @brief Creates a component registry
     * The components are created according to the rules in a components configuration file if such
     * file is found as follows:
     * - If the environment variable @p env_variable is set and it
     *    contains an absolute file path and such file is found at that path.
     * - If the environment variable @p env_variable is set and it
     *    contains a relative file path and such file is found in the current working directory
     *    or in the directory where the fep3_participant shared library file resides in
     * - If the environment variable @p env_variable is not set and
     *    the file @p component_file_name is found in the current working directory
     *    or in the directory where the fep3_participant shared library file resides in
     * Otherwise, i. e. if no components configuration file is found, the a default set of native
     * components is created.
     *
     * @param[in] logger The logger to log information about component registry creation to
     * @param[in] env_variable The name of the environment variable.
     * @param[in] component_file_name The name of the components file
     * @throw Throws an exception of type std::runtime_error if
     *  * the environment variable @p env_variable is set but no file can be found at the that
     * location
     *  * parsing the components configuration file fails, e. g. if its content does not conform to
     * the expected schema
     *  * opening one of the shared libraries as referred to by the components configuration file
     * @return Shared pointer to the created component registry
     */
    static std::shared_ptr<ComponentRegistry> createRegistry(
        const ILogger* logger,
        const std::string& env_variable,
        const std::string& component_file_name = "fep3_participant.fep_components");

private:
    ///@cond nodoc
    static std::shared_ptr<ComponentRegistry> createRegistryByFile(const std::string& file_path,
                                                                   const ILogger* logger);
    ///@endcond nodoc
};

} // namespace arya
using arya::ComponentRegistryFactory;
} // namespace fep3