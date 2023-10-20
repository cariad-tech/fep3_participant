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

#include <fep3/base/component_registry/component_version_info.h>
#include <fep3/components/base/component_intf.h>
#include <fep3/components/logging/logger_intf.h>

namespace fep3 {

class ComponentFactoryBase {
public:
    ComponentFactoryBase() = default;
    virtual ~ComponentFactoryBase() = default;
    ComponentFactoryBase(ComponentFactoryBase&&) = default;
    ComponentFactoryBase& operator=(ComponentFactoryBase&&) = default;

protected:
    ComponentFactoryBase(const ComponentFactoryBase&) = delete;
    ComponentFactoryBase& operator=(const ComponentFactoryBase&) = delete;

public:
    virtual std::shared_ptr<fep3::arya::IComponent> createComponent(const std::string& /*iid*/,
                                                                    const ILogger* /*logger*/) const
    {
        return {};
    }

    virtual ComponentVersionInfo getPluginInfo() const = 0;
};

} // namespace fep3
