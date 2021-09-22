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

#include <string>
#include <vector>
#include <fep3/fep3_participant_types.h>
#include <fep3/participant/component_source_type.h>

namespace fep3
{
namespace arya
{
    class ComponentsConfiguration
    {
        public:
            struct PluginInfo
            {
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

        private:
            // Note: The order of entries in the container of items defines the order of component creation,
            // so we use a container of pairs rather than a map.
            const std::vector<ComponentConfiguration> _items;
    };
}
}