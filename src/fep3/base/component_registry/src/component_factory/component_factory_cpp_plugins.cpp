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

#include "../../include/component_factory/component_factory_cpp_plugins.h"

#include "../../include/plugin/cpp/cpp_host_plugin.h"

#include <fep3/components/logging/easy_logger.h>
#include <fep3/plugin/cpp/cpp_plugin_component_factory_intf.h>
#include <fep3/plugin/cpp/cpp_plugin_intf.h>

namespace fep3 {

struct ComponentFactoryCPPPlugin::Implementation {
    Implementation(const std::string& plugin_file_path)
        : _plugin(plugin_file_path),
          _component_factory(_plugin.create<plugin::cpp::catelyn::IComponentFactory>(
              SYMBOL_fep3_plugin_cpp_catelyn_getFactory))
    {
        if (!_component_factory) {
            throw std::runtime_error("The plugin '" + _plugin.getFilePath() +
                                     "' returned an invalid component factory.");
        }
    }
    ~Implementation() = default;

    plugin::cpp::HostPlugin _plugin;
    const std::unique_ptr<plugin::cpp::catelyn::IComponentFactory> _component_factory;
};

ComponentFactoryCPPPlugin::ComponentFactoryCPPPlugin(const std::string& plugin_file_path)
    : _impl(std::make_unique<Implementation>(plugin_file_path))
{
}

ComponentFactoryCPPPlugin::~ComponentFactoryCPPPlugin()
{
}

std::shared_ptr<fep3::arya::IComponent> ComponentFactoryCPPPlugin::createComponent(
    const std::string& iid, const ILogger* logger) const
{
    auto component = _impl->_component_factory->createComponent(iid);
    if (component) {
        FEP3_LOGGER_LOG_DEBUG(logger,
                              std::string() + "Created FEP Component for interface \"" + iid +
                                  "\" from CPP Plugin @ \"" + _impl->_plugin.getFilePath() +
                                  ", version \"" + _impl->_plugin.getPluginVersion() + "\"" +
                                  ", built with \"" +
                                  _impl->_plugin.getParticipantLibraryVersion().toString() + "\"")
    }
    else {
        FEP3_LOGGER_LOG_ERROR(logger,
                              std::string() + "Failed to create FEP Component for interface \"" +
                                  iid + "\" from CPP Plugin @ \"" + _impl->_plugin.getFilePath() +
                                  ", version \"" + _impl->_plugin.getPluginVersion() + "\"" +
                                  ", built with \"" +
                                  _impl->_plugin.getParticipantLibraryVersion().toString() + "\"")
    }

    return component;
}

ComponentVersionInfo ComponentFactoryCPPPlugin::getPluginInfo() const
{
    return _impl->_plugin.getPluginInfo();
}

} // namespace fep3
