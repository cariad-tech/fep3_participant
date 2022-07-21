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

#pragma once

#include <fep3/fep3_participant_types.h>
#include <fep3/participant/component_factories/component_factory_base.h>
#include <fep3/participant/component_source_type.h>
#include <vector>
#include <string>
#include <memory>

namespace fep3
{
    namespace arya
    {
        class ComponentFactoryCPPPlugin : public ComponentFactoryBase
        {
        public:
            ComponentFactoryCPPPlugin(const std::string& file_path);
            virtual ~ComponentFactoryCPPPlugin();
            std::unique_ptr<fep3::arya::IComponent> createComponent(const std::string& iid, const ILogger* logger) const override;
            ComponentVersionInfo getPluginInfo() const override;
        private:
            struct Implementation;
            std::unique_ptr<Implementation> _impl;
        };
    }
}