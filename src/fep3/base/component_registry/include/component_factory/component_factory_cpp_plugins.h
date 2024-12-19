/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "component_factory_base.h"

namespace fep3 {

class ComponentFactoryCPPPlugin : public ComponentFactoryBase {
public:
    struct Implementation;
    ComponentFactoryCPPPlugin();
    ComponentFactoryCPPPlugin(const std::string& file_path);
    virtual ~ComponentFactoryCPPPlugin();
    std::shared_ptr<fep3::arya::IComponent> createComponent(const std::string& iid,
                                                            const ILogger* logger) const override;
    ComponentVersionInfo getPluginInfo() const override;
    ComponentFactoryCPPPlugin(ComponentFactoryCPPPlugin&&);
    ComponentFactoryCPPPlugin& operator=(ComponentFactoryCPPPlugin&&);

private:
    std::unique_ptr<Implementation> _impl;
};

} // namespace fep3