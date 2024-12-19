/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/base/component_registry/component_version_info.h>
#include <fep3/components/base/component_intf.h>

#include <algorithm>
#include <set>
#include <string>
#include <vector>
namespace fep3::base {

/**
 * .@brief Stores the iids and dependencies of a supercomponent
 */
struct ComponentData {
    /**
     * @brief Constructor.
     *
     * @param[in] component Component pointer.
     * @param[in] version The component version info.
     * @param[in] iid The component iid.
     */
    ComponentData(std::shared_ptr<fep3::arya::IComponent> component,
                  fep3::ComponentVersionInfo version,
                  std::string iid)
        : _super_component(std::move(component)),
          _version(std::move(version)),
          _iids{std::move(iid)}
    {
    }

    /**
     * @brief Checks if the component has an iid.
     *
     * @param[in] fep_component_iid The component iid to check.
     * @return True if the component has this iid.
     */
    bool hasIId(const std::string& fep_component_iid) const
    {
        auto it =
            std::find_if(_iids.begin(), _iids.end(), [&fep_component_iid](const auto& comp_iid) {
                return fep_component_iid == comp_iid;
            });
        return it != _iids.end();
    }

    /**
     * @brief Returns the iid that the object was
     * constructed with.
     *
     * @return The component iid.
     */
    std::string getIID() const
    {
        return _iids.at(0);
    }
    /** Pointer to the component */
    std::shared_ptr<fep3::arya::IComponent> _super_component;
    /** The version info of the component */
    fep3::ComponentVersionInfo _version;
    /** All the iids that the component is registered with */
    std::vector<std::string> _iids;
    /** The dependencies of the component */
    std::set<std::string> _required_components;
};

/**
 * .@brief Stores the information about a property of a component.
 */
struct PropertyInfo {
    ///@cond nodoc
    std::string _available_in_state;
    std::string _property_name;
    std::string _type;
    std::string _default_value;
    std::string _path;
};

enum class SignalFlow
{
    input,
    output
};
///@endcond nodoc

/**
 * .@brief Stores the information about the properties
 * of a stream type.
 */
struct StreamTypeProperty {
    ///@cond nodoc
    std::string _name;
    std::string _value;
    std::string _type;
    ///@endcond nodoc
};

/**
 * .@brief Stores the information about a data reader or
 * writer that is created by a component.
 */
struct SignalInfo {
    ///@cond nodoc
    std::string _name;
    std::string _stream_meta_type;
    std::vector<StreamTypeProperty> _properties;
    SignalFlow _direction;
    ///@endcond nodoc
};

/**
 * .@brief Stores all the information represented in component metamodel
 * about a component.
 */
struct ComponentMetaModelInfo {
    ///@cond nodoc
    ComponentMetaModelInfo(const ComponentData& comp_data,
                           std::vector<PropertyInfo> property_info,
                           std::vector<SignalInfo> signal_infos)
        : _path(comp_data._version.getFilePath()),
          _required_components(comp_data._required_components),
          _property_info(std::move(property_info)),
          _signal_infos(std::move(signal_infos)),
          _iids(comp_data._iids)
    {
    }
    ///@endcond nodoc

    /**
     * @brief Get all the information about the signals that are sent
     * or received by a component.
     *
     * @param flow Selects the input or output signals.
     * @return The signals read or written by the component
     */
    std::vector<SignalInfo> getSignalsWithDirection(SignalFlow flow) const
    {
        std::vector<SignalInfo> ret;
        for (const auto& signal_info: _signal_infos) {
            if (signal_info._direction == flow) {
                ret.push_back(signal_info);
            }
        }

        return ret;
    }

    /**
     * @brief Checks if the component has an iid.
     *
     * @param[in] iid The component iid to check.
     * @return True if the component has this iid.
     */
    bool hasIID(const std::string& iid)
    {
        auto it = std::find(_iids.begin(), _iids.end(), iid);
        return it != _iids.end();
    }

    ///@cond nodoc
    std::string _path;
    std::set<std::string> _required_components;
    std::vector<PropertyInfo> _property_info;
    std::vector<SignalInfo> _signal_infos;
    std::vector<std::string> _iids;
    ///@endcond nodoc
};

/**
 * .@brief The file paths of a component.
 * .
 */
struct ComponentPath {
    ComponentPath(std::string absolute_path, std::string relative_path)
        : _absolute_path(std::move(absolute_path)), _relative_path(std::move(relative_path))
    {
    }
    /** The path as given to the component loading mechanism */
    std::string _absolute_path = "";
    /** The path to the component relative to the conan package */
    std::string _relative_path = "";

    ///@cond nodoc
    bool operator==(const ComponentPath& path) const
    {
        return _absolute_path == path._absolute_path && _relative_path == path._relative_path;
    }
    ///@endcond nodoc
};

/**
 * .@brief Component that the analyzer should generate
 *  meta information for.
 * .
 */
struct ComponentToAnalyze {
    ComponentToAnalyze(std::string component_iid,
                       std::string absolute_path,
                       std::string relative_path)
        : _component_iid(std::move(component_iid)),
          _component_path(std::move(absolute_path), std::move(relative_path))
    {
    }
    ///@cond nodoc
    std::string _component_iid;
    ComponentPath _component_path;
    ///@endcond nodoc
};
} // namespace fep3::base
