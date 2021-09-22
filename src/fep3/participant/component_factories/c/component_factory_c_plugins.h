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


#include <memory>
#include <string>
#include <vector>

#include <fep3/participant/component_factories/component_factory_base.h>
#include <fep3/participant/component_source_type.h>

namespace fep3
{
namespace arya
{

/**
 * @brief Factory class creating components from within a C plugin
 */
class ComponentFactoryCPlugin : public ComponentFactoryBase
{
public:
    /**
     * CTOR
     * @param[in] plugin_file_path File path to the C plugin that provides facilities to create the
     *                             FEP Component instance to be created by this factory
     */
    ComponentFactoryCPlugin(const std::string& plugin_file_path);
    /// DTOR
    virtual ~ComponentFactoryCPlugin() override;
    /**
     * Creates a component identified by \p iid from within a C plugin
     * @param[in] iid The interface identifier of the component to be created
     * @return Unique pointer to the created component (if any)
     */
    std::unique_ptr<fep3::IComponent> createComponent(const std::string& iid, const ILogger* logger) const override;

private:
    class Implementation;
    std::unique_ptr<Implementation> _impl;
};

}
}
