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


#include <fep3/components/logging/easy_logger.h>
#include "component_factory_cpp_plugins.h"
#include "component_creator_cpp_plugin.h"
#include <fep3/plugin/cpp/cpp_host_plugin.h>

namespace fep3
{
namespace arya
{
    struct ComponentFactoryCPPPlugin::Implementation
    {
        Implementation(const std::string& plugin_file_path)
            : _plugin(plugin_file_path)
        {}
        ~Implementation() = default;

        plugin::cpp::HostPlugin _plugin;
    };

    ComponentFactoryCPPPlugin::ComponentFactoryCPPPlugin(const std::string& plugin_file_path) :
        _impl(std::make_unique<Implementation>(plugin_file_path))
    {}
    ComponentFactoryCPPPlugin::~ComponentFactoryCPPPlugin()
    {}
    std::unique_ptr<fep3::arya::IComponent> ComponentFactoryCPPPlugin::createComponent(const std::string& iid, const ILogger* logger) const
    {
        auto component = ComponentCreatorCPPPlugin()(_impl->_plugin, iid);
        if(component)
        {
            FEP3_LOGGER_LOG_INFO
                (logger
                , std::string() + "Created FEP Component for interface \"" + iid
                    +  "\" from CPP Plugin @ \"" + _impl->_plugin.getFilePath()
                    + ", version \"" + _impl->_plugin.getPluginVersion() +  "\""
                    + ", built with \"" + _impl->_plugin.getParticipantLibraryVersion().toString() + "\""
                )
        }
        else
        {
            FEP3_LOGGER_LOG_ERROR
                (logger
                , std::string() + "Failed to create FEP Component for interface \"" + iid
                    +  "\" from CPP Plugin @ \"" + _impl->_plugin.getFilePath()
                    + ", version \"" + _impl->_plugin.getPluginVersion() +  "\""
                    + ", built with \"" + _impl->_plugin.getParticipantLibraryVersion().toString() + "\""
                )
        }

        return component;
    }

    ComponentVersionInfo ComponentFactoryCPPPlugin::getPluginInfo() const
    {
        return _impl->_plugin.getPluginInfo();
    }
}
}
