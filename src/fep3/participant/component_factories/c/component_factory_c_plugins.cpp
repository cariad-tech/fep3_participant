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
#include <fep3/plugin/c/c_host_plugin.h>
#include "component_factory_c_plugins.h"
#include "component_creator_c_plugin.h"
#include <fep3/components/base/c_access_wrapper/component_getter_function_getter.h>

// include access classes of all components that are exchangeable via Component C Plugin System here
#include <fep3/components/clock/c_access_wrapper/clock_service_c_access_wrapper.h>
#include <fep3/components/job_registry/c_access_wrapper/job_registry_with_rpc_c_access_wrapper.h>
#include <fep3/components/participant_info/c_access_wrapper/participant_info_c_access_wrapper.h>
#include <fep3/components/scheduler/c_access_wrapper/scheduler_service_c_access_wrapper.h>
#include <fep3/components/simulation_bus/c_access_wrapper/simulation_bus_c_access_wrapper.h>

namespace fep3
{
namespace arya
{

class ComponentFactoryCPlugin::Implementation
{
public:
    Implementation(const std::string& plugin_file_path)
        : _plugin(std::make_shared<plugin::c::HostPlugin>(plugin_file_path))
    {}
    std::shared_ptr<plugin::c::HostPlugin> _plugin;
    TypedComponentCreatorCPlugin
        // list of access object types of all components that are exchangeable via Component C Plugin System
        // note: the version namespace (arya, bronn, etc.) must be incorporated to support the creation of Components for different namespaces
        <fep3::plugin::c::access::arya::ClockService
        , fep3::plugin::c::access::arya::JobRegistryWithRPC
        , fep3::plugin::c::access::arya::ParticipantInfo
        , fep3::plugin::c::access::arya::SchedulerService
        , fep3::plugin::c::access::arya::SimulationBus
        //, fep3::plugin::c::access::bronn::SimulationBus
        //, fep3::plugin::c::access::bronn::SchedulerService
        > _typed_component_creator;
};

ComponentFactoryCPlugin::ComponentFactoryCPlugin(const std::string& plugin_file_path)
    : _impl(std::make_unique<Implementation>(plugin_file_path))
{}

ComponentFactoryCPlugin::~ComponentFactoryCPlugin() = default;

std::unique_ptr<fep3::arya::IComponent> ComponentFactoryCPlugin::createComponent(const std::string& iid, const ILogger* logger) const
{
    auto component = _impl->_typed_component_creator
        (_impl->_plugin
        // list of access object types of all components that can be accessed from within a C plugin
        // note: the version namespace (arya, bronn, etc.) must be incorporated to support access to components for different namespaces
        , std::make_shared<fep3::plugin::c::arya::ComponentGetterFunctionGetter
            <fep3::plugin::c::access::arya::ClockService
            , fep3::plugin::c::access::arya::JobRegistry
            , fep3::plugin::c::access::arya::ParticipantInfo
            , fep3::plugin::c::access::arya::SchedulerService
            , fep3::plugin::c::access::arya::SimulationBus
            >>()
        , iid
        );
    if(component)
    {
        FEP3_LOGGER_LOG_INFO
            (logger
            , std::string() + "Created FEP Component for interface \"" + iid
                +  "\" from C Plugin @ \"" + _impl->_plugin->getFilePath()
                + ", version \"" + _impl->_plugin->getPluginVersion() +  "\""
                + ", built with \"" + _impl->_plugin->getParticipantLibraryVersion().toString() + "\""
            )
    }
    else
    {
        FEP3_LOGGER_LOG_ERROR
            (logger
            , std::string() + "Failed to create FEP Component for interface \"" + iid
                +  "\" from C Plugin @ \"" + _impl->_plugin->getFilePath()
                + ", version \"" + _impl->_plugin->getPluginVersion() +  "\""
                + ", built with \"" + _impl->_plugin->getParticipantLibraryVersion().toString() + "\""
            )
    }

    return component;
}

ComponentVersionInfo ComponentFactoryCPlugin::getPluginInfo() const
{
    return ComponentVersionInfo(_impl->_plugin->getPluginVersion(), _impl->_plugin->getFilePath(), _impl->_plugin->getParticipantLibraryVersion().toString());
}

}
}
