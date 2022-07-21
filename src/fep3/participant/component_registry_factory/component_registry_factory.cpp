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


#include <map>

#include "component_registry_factory.h"
#include "components_configuration.h"
#include <fep3/base/environment_variable/environment_variable.h>
#include <fep3/participant/component_factories/cpp/component_factory_cpp_plugins.h>
#include <fep3/participant/component_factories/c/component_factory_c_plugins.h>
#include <fep3/base/file/file.h>
#include <fep3/base/binary_info/binary_info.h>
#include <fep3/components/logging/logger_intf.h>
#include <fep3/components/logging/easy_logger.h>

#define FEP3_PARTICIPANT_COMPONENTS_FILE_PATH_ENVIRONMENT_VARIABLE "FEP3_PARTICIPANT_COMPONENTS_FILE_PATH"

namespace fep3
{
    namespace arya
    {
        std::shared_ptr<ComponentRegistry> ComponentRegistryFactory::createRegistry(const ILogger* logger)
        {
            a_util::filesystem::Path component_config_file_path;
            const std::vector<a_util::filesystem::Path> search_hints
                {::a_util::filesystem::getWorkingDirectory()
                , binary_info::getFilePath()
                };
            // Note: if the environment variable is set, the user tells us to load a specific components configuration file, so
            // - we do not search for the default file name (see below)
            // - we do not silently load default components only
            // , but rather throw an exception if such file is not found.
            const auto& environment_variable_file_path = environment_variable::get(FEP3_PARTICIPANT_COMPONENTS_FILE_PATH_ENVIRONMENT_VARIABLE);
            if(environment_variable_file_path)
            {
                FEP3_LOGGER_LOG_INFO
                    (logger
                    , std::string() + "Found environment variable \"" + FEP3_PARTICIPANT_COMPONENTS_FILE_PATH_ENVIRONMENT_VARIABLE + "\" with "
                        + " value \"" + *environment_variable_file_path + "\""
                    )
                component_config_file_path = file::find
                    (environment_variable_file_path.value()
                    , search_hints
                    );
                if(component_config_file_path.isEmpty())
                {
                    throw std::runtime_error(std::string() + "Couldn't find FEP Component Configuration File '" + *environment_variable_file_path + "'");
                }
            }
            else
            {
                FEP3_LOGGER_LOG_INFO
                    (logger
                    , std::string() + "Environment variable \"" + FEP3_PARTICIPANT_COMPONENTS_FILE_PATH_ENVIRONMENT_VARIABLE + "\" not set; "
                        + " searching for default FEP Component Configuration File with name \"fep3_participant.fep_components \""
                    )
                // search for the default file name
                component_config_file_path = file::find
                    ("./fep3_participant.fep_components"
                    , search_hints
                    );
            }
            if(component_config_file_path.isEmpty())
            {
                FEP3_LOGGER_LOG_ERROR
                (logger
                    , std::string() + "No FEP Component Configuration File found; Built-in component loading is deprecated, use a FEP Component Configuration File"
                )
                throw std::runtime_error("No FEP Component Configuration File found; Built-in component loading is deprecated, use a FEP Component Configuration File");
            }
            else
            {
                FEP3_LOGGER_LOG_INFO
                    (logger
                    , std::string() + "FEP Component Configuration File found @ \"" + component_config_file_path.toString() + "\"; loading FEP Components accordingly"
                    )
                return createRegistryByFile(component_config_file_path, logger);
            }
        }

        std::shared_ptr<ComponentRegistry> ComponentRegistryFactory::createRegistryByFile
            (const a_util::filesystem::Path& file_path
            , const ILogger* logger
            )
        {
            auto registry = std::make_shared<ComponentRegistry>();

            // Load the FEP Components Configuration by parsing the file.
            // Note: The FEP Components Configuration file must contain an entry for each
            // FEP Component IID that is used by the FEP Participant.
            ComponentsConfiguration components_configuration(file_path);

            // for each item (FEP Component IID) in the FEP Components Configuration
            // create the FEP Component instance from within the correct origin
            for (const auto& item_to_create : components_configuration.getItems())
            {
                const auto& component_iid = item_to_create.first;
                const auto& component_source_type = item_to_create.second._source_type;
                const auto& plugin_file_path = item_to_create.second._plugin_file_path;

                std::unique_ptr<ComponentFactoryBase> component_factory;
                switch(component_source_type)
                {
                    case ComponentSourceType::built_in:
                    {
                        throw std::runtime_error( "Built in components are deprecated, use a cpp-plugin component");
                        break;
                    }
                    case ComponentSourceType::cpp_plugin: component_factory = std::make_unique<ComponentFactoryCPPPlugin>(plugin_file_path); break;
                    case ComponentSourceType::c_plugin: component_factory = std::make_unique<ComponentFactoryCPlugin>(plugin_file_path); break;
                    default: throw std::runtime_error
                        (std::string() + "Unsupported FEP Component source type "
                        + "\"" + getString(component_source_type) + "\"."
                        + " Check your fep_components file : " + file_path
                        );
                }
                if(auto component = component_factory->createComponent(component_iid, logger))
                {
                    auto plugin_info = component_factory->getPluginInfo();
                    registry->registerComponent
                        (component_iid
                        , std::move(component)
                        , plugin_info
                        );
                }
                else
                {
                     throw std::runtime_error
                        ("The FEP Component factory for type "
                        + getString(component_source_type)
                        + " cannot create a FEP Component with IID \"" + component_iid + "\"."
                        );
                }
            }
            return registry;
        }
    }
}
