/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../../include/component_factory/component_factory_cpp_plugins.h"
#include "../../include/component_registry_factory/components_configuration.h"
#include "../../include/file/file.h"

#include <fep3/base/component_registry/component_registry_factory.h>
#include <fep3/components/logging/easy_logger.h>

#include <a_util/process/process.h>
#include <a_util/system/address_info.h>

namespace fep3 {
namespace arya {
std::shared_ptr<ComponentRegistry> ComponentRegistryFactory::createRegistry(
    const ILogger* logger, const std::string& env_variable, const std::string& component_file_name)
{
    /// Static method to query information about the memory address of this binary
    const auto address_info = a_util::system::AddressInfo(ComponentRegistryFactory::createRegistry);
    fs::path component_config_file_path;
    const std::vector<fs::path> search_hints{
        fs::current_path(), fs::path(address_info.getFilePath().getParent().toString())};
    // Note: if the environment variable is set, the user tells us to load a specific components
    // configuration file, so
    // - we do not search for the default file name (see below)
    // - we do not silently load default components only
    // , but rather throw an exception if such file is not found.
    const auto& environment_variable_file_path = a_util::process::getEnvVar(env_variable, "");

    if (!environment_variable_file_path.empty()) {
        FEP3_LOGGER_LOG_DEBUG(logger,
                              std::string() + "Found environment variable \"" + env_variable +
                                  "\" with " + " value \"" + environment_variable_file_path + "\"")
        component_config_file_path = file::find(environment_variable_file_path, search_hints);
        if (component_config_file_path.empty()) {
            throw std::runtime_error(std::string() +
                                     "Couldn't find FEP Component Configuration File '" +
                                     environment_variable_file_path + "'");
        }
    }
    else {
        FEP3_LOGGER_LOG_DEBUG(
            logger,
            std::string() + "Environment variable \"" + env_variable + "\" not set; " +
                " searching for default FEP Component Configuration File with name \"" +
                component_file_name + "\"")
        // search for the default file name
        component_config_file_path = file::find("./" + component_file_name, search_hints);
    }
    if (component_config_file_path.empty()) {
        FEP3_LOGGER_LOG_ERROR(logger,
                              std::string() +
                                  "No FEP Component Configuration File found; Built-in component "
                                  "loading is deprecated, use a FEP Component Configuration File")
        throw std::runtime_error("No FEP Component Configuration File found; Built-in component "
                                 "loading is deprecated, use a FEP Component Configuration File");
    }
    else {
        FEP3_LOGGER_LOG_DEBUG(logger,
                              std::string() + "FEP Component Configuration File found @ \"" +
                                  component_config_file_path.string() +
                                  "\"; loading FEP Components accordingly")
        return createRegistryByFile(component_config_file_path.string(), logger);
    }
}

std::shared_ptr<ComponentRegistry> ComponentRegistryFactory::createRegistryByFile(
    const std::string& file_path, const ILogger* logger)
{
    auto registry = std::make_shared<ComponentRegistry>();

    // Load the FEP Components Configuration by parsing the file.
    // Note: The FEP Components Configuration file must contain an entry for each
    // FEP Component IID that is used by the FEP Participant.
    ComponentsConfiguration components_configuration(file_path);
    const auto& component_configuration_items = components_configuration.getItems();

    // for each plugin path in the FEP Components Configuration
    // create the appropriate component factory
    std::map<std::string, std::unique_ptr<ComponentFactoryBase>> component_factories;
    for (const auto& component_configuration_item: component_configuration_items) {
        const auto& source_type = component_configuration_item.second._source_type;
        const auto& plugin_file_path = component_configuration_item.second._plugin_file_path;
        if (component_factories.find(plugin_file_path) == component_factories.cend()) {
            std::unique_ptr<ComponentFactoryBase> component_factory;
            switch (source_type) {
            case ComponentSourceType::built_in: {
                throw std::runtime_error(
                    "Built in components are deprecated, use a cpp-plugin component");
            }
            case ComponentSourceType::cpp_plugin:
                component_factory = std::make_unique<ComponentFactoryCPPPlugin>(plugin_file_path);
                break;
            case ComponentSourceType::c_plugin: {
                throw std::runtime_error("C-plugin components are deprecated");
            }
            default:
                throw std::runtime_error(std::string() + "Unsupported FEP Component source type " +
                                         "\"" + getString(source_type) + "\"." +
                                         " Check your fep_components file : " + file_path);
            }
            component_factories.emplace(component_configuration_item.second._plugin_file_path,
                                        std::move(component_factory));
        }
    }

    // for each item (FEP Component IID) in the FEP Components Configuration
    // create the FEP Component instance from within the correct plugin
    for (const auto& component_configuration_item: component_configuration_items) {
        const auto& plugin_file_path = component_configuration_item.second._plugin_file_path;
        const auto& component_factory_iter = component_factories.find(plugin_file_path);
        if (component_factory_iter != component_factories.cend()) {
            const auto& component_factory = component_factory_iter->second;
            if (component_factory) {
                const auto& component_iid = component_configuration_item.first;
                if (auto component = component_factory->createComponent(component_iid, logger)) {
                    auto plugin_info = component_factory->getPluginInfo();
                    registry->registerComponent(component_iid, std::move(component), plugin_info);
                }
                else {
                    throw std::runtime_error(
                        "The FEP Component factory for plugin file path \"" + plugin_file_path +
                        "\" cannot create a FEP Component with IID \"" + component_iid + "\".");
                }
            }
            else {
                throw std::runtime_error(
                    "Invalid FEP Component factory found for plugin file path \"" +
                    plugin_file_path + "\".");
            }
        }
        else {
            throw std::runtime_error("No FEP Component factory found for plugin file path \"" +
                                     plugin_file_path + "\".");
        }
    }
    return registry;
}
} // namespace arya
} // namespace fep3
