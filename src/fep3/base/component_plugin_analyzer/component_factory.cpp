/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "component_factory.h"

#include "component_path_hash_function.h"

namespace fep3::base {
ComponentFactory::ComponentFactory(const std::unordered_set<ComponentPath>& component_files)
{
    for (const auto& component_file: component_files) {
        _cpp_plugin_factories.emplace_back(component_file._absolute_path);
    }
}

ComponentData ComponentFactory::createComponent(const ComponentToAnalyze& comp)
{
    fep3::ComponentFactoryCPPPlugin& factory = getFactory(comp._component_path._absolute_path);
    auto ptr = factory.createComponent(comp._component_iid, nullptr);
    if (ptr) {
        const auto version_info = factory.getPluginInfo();
        fep3::ComponentVersionInfo version_info_rel_path(
            version_info.getVersion(),
            comp._component_path._relative_path,
            version_info.getParticipantLibraryVersion());

        return {ptr, version_info_rel_path, comp._component_iid};
    }

    // better throw?
    assert(false && "Happy path not reached");
    return {nullptr, {}, comp._component_iid};
}

fep3::ComponentFactoryCPPPlugin& ComponentFactory::getFactory(const std::string& plugin_path)
{
    auto it = std::find_if(_cpp_plugin_factories.begin(),
                           _cpp_plugin_factories.end(),
                           [&plugin_path](fep3::ComponentFactoryCPPPlugin& plugin) {
                               return plugin.getPluginInfo().getFilePath() == plugin_path;
                           });
    // test only happy path at the moment
    assert(it != _cpp_plugin_factories.end());
    return *it;
}
} // namespace fep3::base
