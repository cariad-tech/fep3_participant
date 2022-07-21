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
#include <utility>
#include <vector>
#include <map>

#include <fep3/fep3_result_decl.h>
#include <fep3/components/base/components_intf.h>
#include <fep3/components/base/component_version_info.h>
#include <a_util/result/error_def.h>


namespace fep3
{
namespace arya
{
    /**
        * @brief default component registry implementation
        *
        * @note Objects of this class must be held in an std::shared_ptr, because this class is derived from std::enable_shared_from_this
        */
    class FEP3_PARTICIPANT_EXPORT ComponentRegistry
        : public arya::IComponents,
          public std::enable_shared_from_this<ComponentRegistry>
    {
    public:
        /**
            * @brief Construct a new Component Registry object
            *
            */
        ComponentRegistry();
        /**
            * @brief Destroy the Component Registry object
            *
            */
        ~ComponentRegistry();
        /**
          * @brief would construct a new Component Registry object, but is deleted
          *
          */
        ComponentRegistry(const ComponentRegistry&) = delete;
        /**
          * @brief would assign the Component Registry object, but is deleted
          * @return this ComponentRegistry
          */
        ComponentRegistry& operator=(const ComponentRegistry&) = delete;

        /**
         * @brief Construct a new Component Registry by moving from another
         *
         */
        ComponentRegistry(ComponentRegistry&&) = default;
        /**
          * @brief move the Component Registry object
          * @return this ComponentRegistry
          */
        ComponentRegistry& operator=( ComponentRegistry&&) = default;

        /**
         * @brief This will register a instance of a component to the registry.
         * @pre @p component_type must implement access to the interface id (see @ref FEP3_COMPONENT_IID)
         * @remark The ownership of the IComponent pointer goes to the registry.
         *         Only one component of the interface id is possible.
         *
         * @tparam component_type The component class which must be provide the component interface id.
         * @param[in] component Shared pointer to the component to be registered
         * @retval fep3::ERR_INVALID_TYPE given component does not support the interface
         * @return fep3::Result
         */
        template<class component_type>
        fep3::Result registerComponent(const std::shared_ptr<arya::IComponent>& component, const ComponentVersionInfo& version_info)
        {
            const std::string fep_iid = getComponentIID<component_type>();
            return registerComponent(fep_iid, component, version_info);
        }
        /**
         * @brief unregister the component with the interface id of T
         * @pre @p component_type must implement access to the interface id (see @ref FEP3_COMPONENT_IID)
         *
         * @tparam component_type The component interface to unregister.
         * @return fep3::Result
         */
        template<class component_type>
        fep3::Result unregisterComponent()
        {
            const std::string fep_iid = getComponentIID<component_type>();
            return unregisterComponent(fep_iid);
        }

        /**
            * @brief call the IComponent::create method of the registered components
            *
            * @return fep3::Result
            */
        fep3::Result create();
        /**
            * @brief call the IComponent::destroy method of the registered components
            *
            * @return fep3::Result
            */
        fep3::Result destroy();

        /**
            * @brief call the IComponent::initializing method of the registered components
            *
            * @return fep3::Result
            */
        fep3::Result initialize();
        /**
            * @brief call the IComponent::deinitializing method of the registered components
            *
            * @return fep3::Result
            */
        fep3::Result deinitialize();

        /**
         * @brief call the IComponent::tense method of the registered components
         *
         * @return fep3::Result
         */
        fep3::Result tense();

        /**
         * @brief call the IComponent::relax method of the registered components
         *
         * @return fep3::Result
         */
        fep3::Result relax();

        /**
            * @brief call the IComponent::start method of the registered components
            *
            * @return fep3::Result
            */
        fep3::Result start();
        /**
            * @brief call the IComponent::stop method of the registered components
            *
            * @return fep3::Result
            */
        fep3::Result stop();

        /**
        * @brief call the IComponent::pause method of the registered components
        *
        * @return fep3::Result
        */
        fep3::Result pause();
        /**
        * @brief call the IComponent::contains method of the registered components
        *
        * @return @c true if the component is contained, @c false otherwise
        */
        template<class component_type>
        bool contains() const
        {
            const std::string fep_iid = getComponentIID<component_type>();
            if (findComponent(fep_iid) == nullptr)
            {
                return false;
            }
            return true;
        }

        /**
            * @brief empties the list of components and call there DTOR
            *
            */
        void clear();
        /**
          * @brief Registers an instance of a component to the registry.
          * The given component must implement an access to an interface identifier \p fep_iid (see @ref FEP3_COMPONENT_IID)
          * @remark The component registry takes shared ownership of the \p component if and only if registration is successful.
          *         Only one component of the interface \p fep_iid can be registered.
          *
          * see access function  @ref registerComponent<T>
          *
          * @param[in] fep_iid the component interface identifier of the given \p component
          * @param[in] component Shared pointer to the component to be registered
          * @return fep3::Result
          */
        fep3::Result registerComponent(const std::string& fep_iid,
                                       const std::shared_ptr<arya::IComponent>& component , 
                                       const ComponentVersionInfo& version_info);
        /**
          * @brief unregister the component with the interface id of (see @ref FEP3_COMPONENT_IID)
          *
          * see access function  @ref unregisterComponent<T>
          *
          * @param[in] fep_iid the component interface identifier to unregister.
          * @return fep3::Result
          */
        fep3::Result unregisterComponent(const std::string& fep_iid);
    
        std::pair<fep3::Result, ComponentVersionInfo> getComponentVersion(const std::string& fep_iid) const;
    private:
        arya::IComponent* findComponent(const std::string& fep_iid) const override;

        /**
        * @brief searches for a component in the component registry by raw pointer
        *
        * @param[in] component component to be looked for
        * @return shared_ptr to the component or empty shared_ptr (if not found)
        */
        std::shared_ptr<arya::IComponent> findComponentByPtr(arya::IComponent* component) const;

        /// the components container
        std::vector<std::pair<std::string, std::shared_ptr<arya::IComponent>>> _components;
        std::map<std::string, ComponentVersionInfo> _comp_version_info;
    };
}
using arya::ComponentRegistry;
}

