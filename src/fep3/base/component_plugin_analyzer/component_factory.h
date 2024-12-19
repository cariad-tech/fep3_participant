/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <fep3/base/component_plugin_analyzer/component_description_base.h>

#include <component_factory/component_factory_cpp_plugins.h>
#include <unordered_set>
#include <vector>

namespace fep3::base {
class ComponentFactory {
public:
    ComponentFactory(const std::unordered_set<ComponentPath>& component_files);
    ComponentData createComponent(const ComponentToAnalyze& comp);

private:
    fep3::ComponentFactoryCPPPlugin& getFactory(const std::string& plugin_path);
    std::vector<fep3::ComponentFactoryCPPPlugin> _cpp_plugin_factories;
};
} // namespace fep3::base
