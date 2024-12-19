/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../../include/component_registry_factory/components_configuration.h"

#include <fep3/fep3_filesystem.h>

#include <a_util/strings.h>
#include <a_util/xml.h>

namespace fep3 {
namespace arya {

ComponentsConfiguration::ComponentsConfiguration(const std::string& file_path)
    : _items(load(file_path))
{
}

std::vector<ComponentsConfiguration::ComponentConfiguration> ComponentsConfiguration::load(
    const std::string& file_path)
{
    std::vector<ComponentConfiguration> items;
    a_util::xml::DOM loaded_file;
    if (!loaded_file.load(file_path)) {
        auto error_message = a_util::strings::format("can not loaded %s - Error : %s",
                                                     file_path.c_str(),
                                                     loaded_file.getLastError().c_str());
        throw std::runtime_error(error_message);
    }

    a_util::xml::DOMElement schema_version;
    if (loaded_file.getRoot().findNode("schema_version", schema_version)) {
        auto schema_version_string = schema_version.getData();
        if (a_util::strings::trim(schema_version_string) != "1.0.0") {
            auto error_message = a_util::strings::format(
                "can not loaded %s - Error : wrong schema version found : expect %s - found %s",
                file_path.c_str(),
                "1.0.0",
                schema_version_string.c_str());
            throw std::runtime_error(error_message);
        }
    }
    else {
        auto error_message = a_util::strings::format(
            "can not loaded %s - Error : %s", file_path.c_str(), "no schema version tag found");
        throw std::runtime_error(error_message);
    }

    a_util::xml::DOMElementList comps;
    if (loaded_file.getRoot().findNodes("component", comps)) {
        { // validate it

            for (const auto& comp_node: comps) {
                auto source_node = comp_node.getChild("source");
                auto comp_iid = comp_node.getChild("iid");

                if (source_node.isNull()) {
                    auto error_message =
                        a_util::strings::format("can not loaded %s - Error : %s",
                                                file_path.c_str(),
                                                "no iid node for component tag found");
                    throw std::runtime_error(error_message);
                }
                if (comp_iid.isNull()) {
                    auto error_message =
                        a_util::strings::format("can not loaded %s - Error : %s",
                                                file_path.c_str(),
                                                "no source node for component tag found");
                    throw std::runtime_error(error_message);
                }
            }
        }
        { // fill items
            for (const auto& comp_node: comps) {
                auto source_node = comp_node.getChild("source");
                auto comp_iid = comp_node.getChild("iid").getData();
                auto source_type =
                    fep3::arya::getComponentSourceType(source_node.getAttribute("type"));
                auto source_file_string = source_node.getData();
                a_util::strings::trim(source_file_string);
                fs::path component_lib_path;
                if (!source_file_string.empty()) {
                    component_lib_path = source_file_string;
                }
                if (!component_lib_path.empty() && component_lib_path.is_relative()) {
                    component_lib_path = getAbsoluteComponentLibPath(component_lib_path, file_path);
                }
                items.push_back({comp_iid, {source_type, component_lib_path.string()}});
            }
        }
    }
    else {
        // everything is fine, but there are no components in the file defined
    }
    return items;
}

std::vector<ComponentsConfiguration::ComponentConfiguration> ComponentsConfiguration::getItems()
    const
{
    return _items;
}

fs::path ComponentsConfiguration::getAbsoluteComponentLibPath(
    const fs::path& component_lib_rel_path, const std::string& fep_component_file_path) const
{
    // we make it relative to the File! (not the workingdirectory!!)
    // first get the directory of the components dll
    fs::path component_plugin_directory = component_lib_rel_path.parent_path();
    // calculate the canonical path to the fep components file, usually it has a dot in the path
    fs::path fep_components_file_path = fs::canonical(fep_component_file_path);
    // now get the directory of the fep components file
    fep_components_file_path = fep_components_file_path.parent_path();
    // calculate the absolute directory of the components dll
    component_plugin_directory =
        fs::canonical(fs::absolute(fep_components_file_path / component_plugin_directory));

    return component_plugin_directory / component_lib_rel_path.filename();
}
} // namespace arya
} // namespace fep3
