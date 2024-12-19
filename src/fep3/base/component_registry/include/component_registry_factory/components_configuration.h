/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "../../include/component_factory/component_source_type.h"

#include <fep3/fep3_filesystem.h>

#include <vector>

namespace fep3 {
namespace arya {
class ComponentsConfiguration {
public:
    struct PluginInfo {
        ComponentSourceType _source_type;
        // if the source type is any plugin type, the file path is set
        std::string _plugin_file_path;
    };
    // The first part of the pair is the Component IID.
    using ComponentConfiguration = std::pair<std::string, PluginInfo>;

public:
    ComponentsConfiguration(const std::string& file_path);

    std::vector<ComponentConfiguration> getItems() const;

private:
    std::vector<ComponentConfiguration> load(const std::string& file_path);
    fs::path getAbsoluteComponentLibPath(const fs::path& component_lib_rel_path,
                                         const std::string& fep_component_file_path) const;

private:
    // Note: The order of entries in the container of items defines the order of component creation,
    // so we use a container of pairs rather than a map.
    const std::vector<ComponentConfiguration> _items;
};
} // namespace arya
} // namespace fep3