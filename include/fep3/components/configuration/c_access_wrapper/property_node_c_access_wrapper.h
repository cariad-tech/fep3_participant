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
// @note All methods are defined inline to provide the functionality as header only.

#pragma once

#include <fep3/components/configuration/c_intf/property_node_c_intf.h>
#include <fep3/components/configuration/propertynode_intf.h>
#include <fep3/plugin/c/c_access/c_access_helper.h>
#include <fep3/plugin/c/c_wrapper/c_wrapper_helper.h>
#include <fep3/plugin/c/destruction_manager.h>
#include <fep3/c_access_wrapper/fep3_result_c_access_wrapper.h>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace access
{
namespace arya
{
/// @cond no_documentation
namespace detail
{
    template<typename access_type, typename mutability_type>
    class PropertyNode
    {};
    // class template specialization for fep3_arya_const_SIPropertyNode
    template<typename access_type>
    class PropertyNode<access_type, arya::Helper::Immutable>
        : public fep3::arya::IPropertyNode
        , protected access::arya::Helper
    {
    public:
        inline PropertyNode() = default;
        inline PropertyNode(PropertyNode&&) = delete;
        inline PropertyNode(const PropertyNode&) = delete;
        inline PropertyNode& operator=(PropertyNode&&) = delete;
        inline PropertyNode& operator=(const PropertyNode&) = delete;
        inline virtual ~PropertyNode() = default;
    private:
        // methods implementing non-const methods of fep3::arya::IPropertyNode
        inline fep3::Result setValue(const std::string&, const std::string&) override
        {
            throw Exception(fep3_plugin_c_interface_error_const_incorrectness);
        }
        inline void reset() override
        {
            throw Exception(fep3_plugin_c_interface_error_const_incorrectness);
        }
    public:
        // methods implementing const methods of fep3::arya::IPropertyNode
        inline std::string getName() const override;
        inline std::string getValue() const override;
        inline std::string getTypeName() const override;
        inline bool isEqual(const fep3::arya::IPropertyNode& other) const override;
        inline std::vector<std::shared_ptr<IPropertyNode>> getChildren() const override;
        inline size_t getNumberOfChildren() const override;
        inline std::shared_ptr<IPropertyNode> getChild(const std::string& name) const override;
        inline bool isChild(const std::string& name) const override;
    protected:
        /// Type of access structure
        access_type _access;
    };
    // class template specialization for fep3_arya_SIPropertyNode
    template<typename access_type>
    class PropertyNode<access_type, arya::Helper::Mutable>
        : public PropertyNode<access_type, arya::Helper::Immutable> // decorate by const methods for immutable object
    {
    public:
        inline PropertyNode() = default;
        inline PropertyNode(PropertyNode&&) = delete;
        inline PropertyNode(const PropertyNode&) = delete;
        inline PropertyNode& operator=(PropertyNode&&) = delete;
        inline PropertyNode& operator=(const PropertyNode&) = delete;
        inline ~PropertyNode() = default;
        // methods implementing non-const methods of fep3::arya::IPropertyNode
        inline fep3::Result setValue(const std::string& value, const std::string& type_name) override;
        inline void reset() override;
    };
} // namespace detail
/// @endcond no_documentation

/**
 * Class wrapping access to the C interface for @ref fep3::arya::IPropertyNode.
 * Use this class to access a remote object of a type derived from @ref fep3::arya::IPropertyNode that resides in another binary (e. g. a shared library).
 *
 * @tparam access_type The type of the C access structure for @ref fep3::arya::IPropertyNode
 */
template<typename access_type>
class PropertyNode
    : public detail::PropertyNode<access_type, typename std::conditional
        <std::is_same<access_type, fep3_arya_SIPropertyNode>::value
        , arya::Helper::Mutable
        , arya::Helper::Immutable
        >::type>
    , private c::arya::DestructionManager
{
public:
    /**
     * @brief CTOR
     * @param[in] access Access to the remote object
     * @param[in] destructors List of destructors to be called upon destruction of this
     */
    inline PropertyNode
        (const access_type& access
        , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
        );
    /// Default CTOR
    inline PropertyNode() = default;
    /// Deleted move CTOR
    inline PropertyNode(PropertyNode&&) = delete;
    /// Deleted copy CTOR
    inline PropertyNode(const PropertyNode&) = delete;
    /** Deleted move assignment
     * @return this PropertyNode
     */
    inline PropertyNode& operator=(PropertyNode&&) = delete;
    /** Deleted copy assignment
     * @return this PropertyNode
     */
    inline PropertyNode& operator=(const PropertyNode&) = delete;
    /// Default DTOR
    inline ~PropertyNode() = default;
};

} // namespace arya
} // namespace access

namespace wrapper
{
namespace arya
{

/**
 * Wrapper class for interface \ref fep3::arya::IPropertyNode
 *
 * @tparam handle_type The type of the handle to a remote object of @ref fep3::arya::IPropertyNode
 */
template<typename handle_type>
class PropertyNode : private arya::Helper<typename std::conditional
    <std::is_same<handle_type, fep3_arya_const_HIPropertyNode>::value
    , const fep3::arya::IPropertyNode
    , fep3::arya::IPropertyNode
    >::type>
{
private:
    using WrapperHelper = arya::Helper<typename std::conditional
        <std::is_same<handle_type, fep3_arya_const_HIPropertyNode>::value
        , const fep3::arya::IPropertyNode
        , fep3::arya::IPropertyNode
        >::type>;

public:
    /**
     * Functor creating an access structure for @ref ::fep3::arya::IPropertyNode
     */
    struct AccessCreator
    {
        /**
         * Creates an access structure to the property node as pointed to by \p pointer_to_property_node
         *
         * @param[in] pointer_to_property_node Pointer to the property node to create an access structure for
         * @return Access structure to the property node
         */
        fep3_arya_SIPropertyNode operator()(fep3::arya::IPropertyNode* pointer_to_property_node) const noexcept
        {
            return fep3_arya_SIPropertyNode
                {reinterpret_cast<fep3_arya_HIPropertyNode>(pointer_to_property_node)
                , wrapper::arya::PropertyNode<fep3_arya_HIPropertyNode>::getName
                , wrapper::arya::PropertyNode<fep3_arya_HIPropertyNode>::getValue
                , wrapper::arya::PropertyNode<fep3_arya_HIPropertyNode>::getTypeName
                , wrapper::arya::PropertyNode<fep3_arya_HIPropertyNode>::setValue
                , wrapper::arya::PropertyNode<fep3_arya_HIPropertyNode>::isEqual
                , wrapper::arya::PropertyNode<fep3_arya_HIPropertyNode>::reset
                , wrapper::arya::PropertyNode<fep3_arya_HIPropertyNode>::getChildren
                , wrapper::arya::PropertyNode<fep3_arya_HIPropertyNode>::getNumberOfChildren
                , wrapper::arya::PropertyNode<fep3_arya_HIPropertyNode>::getChild
                , wrapper::arya::PropertyNode<fep3_arya_HIPropertyNode>::isChild
                };
        }

        /**
         * Creates an access structure to the const property node as pointed to by \p pointer_to_const_property_node
         *
         * @param[in] pointer_to_const_property_node Pointer to the const property node to create an access structure for
         * @return Access structure to the const property node
         */
        fep3_arya_const_SIPropertyNode operator()(const fep3::arya::IPropertyNode* const pointer_to_const_property_node) const noexcept
        {
            return fep3_arya_const_SIPropertyNode
                {reinterpret_cast<fep3_arya_const_HIPropertyNode>(pointer_to_const_property_node)
                , wrapper::arya::PropertyNode<fep3_arya_const_HIPropertyNode>::getName
                , wrapper::arya::PropertyNode<fep3_arya_const_HIPropertyNode>::getValue
                , wrapper::arya::PropertyNode<fep3_arya_const_HIPropertyNode>::getTypeName
                , wrapper::arya::PropertyNode<fep3_arya_const_HIPropertyNode>::isEqual
                , wrapper::arya::PropertyNode<fep3_arya_const_HIPropertyNode>::getChildren
                , wrapper::arya::PropertyNode<fep3_arya_const_HIPropertyNode>::getNumberOfChildren
                , wrapper::arya::PropertyNode<fep3_arya_const_HIPropertyNode>::getChild
                , wrapper::arya::PropertyNode<fep3_arya_const_HIPropertyNode>::isChild
                };
        }
    };

public:
    /// @cond no_documentation
    static inline fep3_plugin_c_InterfaceError getName
        (handle_type handle
        , void(*callback)(void*, const char*)
        , void* destination
        ) noexcept
    {
        return WrapperHelper::callWithResultCallback
            (handle
            , &fep3::arya::IPropertyNode::getName
            , callback
            , destination
            , [](const std::string& name)
                {
                    return name.c_str();
                }
            );
    }

    static inline fep3_plugin_c_InterfaceError getValue
        (handle_type handle
        , void(*callback)(void*, const char*)
        , void* destination
        ) noexcept
    {
        return WrapperHelper::callWithResultCallback
            (handle
            , &fep3::arya::IPropertyNode::getValue
            , callback
            , destination
            , [](const std::string& value)
                {
                    return value.c_str();
                }
            );
    }

    static inline fep3_plugin_c_InterfaceError getTypeName
        (handle_type handle
        , void(*callback)(void*, const char*)
        , void* destination
        ) noexcept
    {
        return WrapperHelper::callWithResultCallback
            (handle
            , &fep3::arya::IPropertyNode::getTypeName
            , callback
            , destination
            , [](const std::string& type_name)
                {
                    return type_name.c_str();
                }
            );
    }

    template<typename = typename std::enable_if<std::is_same<handle_type, fep3_arya_HIPropertyNode>::value, void>>
    static inline fep3_plugin_c_InterfaceError setValue
        (handle_type handle
        , fep3_result_callback_type result_callback
        , void* result_destination
        , const char* value
        , const char* type_name
        ) noexcept
    {
        return WrapperHelper::callWithResultCallback
            (handle
            , &fep3::arya::IPropertyNode::setValue
            , result_callback
            , result_destination
            , getResult
            , value
            , type_name
            );
    }

    static inline fep3_plugin_c_InterfaceError isEqual
        (handle_type handle
        , bool* result
        , fep3_arya_const_SIPropertyNode property_node_access
        ) noexcept
    {
        return WrapperHelper::callWithResultParameter
            (handle
            , &fep3::arya::IPropertyNode::isEqual
            , [](bool result)
                {
                    return result;
                }
            , result
            , access::arya::PropertyNode<fep3_arya_const_SIPropertyNode>(property_node_access, {})
            );
    }

    template<typename = typename std::enable_if<std::is_same<handle_type, fep3_arya_HIPropertyNode>::value, void>>
    static inline fep3_plugin_c_InterfaceError reset(handle_type handle) noexcept
    {
        return WrapperHelper::call
            (handle
            , &fep3::arya::IPropertyNode::reset
            );
    }

    static inline fep3_plugin_c_InterfaceError getChildren
        (handle_type handle
        , void(*callback)(void*, fep3_plugin_c_arya_SDestructionManager, fep3_arya_SIPropertyNode)
        , void* destination
        ) noexcept
    {
        try
        {
            if(const auto& pointer_to_object = reinterpret_cast<const fep3::arya::IPropertyNode*>(handle))
            {
                const auto& children = pointer_to_object->getChildren();
                for(const auto& child : children)
                {
                    auto child_reference_manager = new c::arya::DestructionManager;
                    // reference to the local child must be released when the remote child is destroyed, so we add a (new) shared reference to the reference manager
                    child_reference_manager->addDestructor
                        (std::make_unique<c::arya::OtherDestructor<typename std::remove_reference<decltype(child)>::type>>
                        (new std::shared_ptr<typename std::decay<decltype(child)>::type::element_type>(child)));
                    auto child_reference_manager_access = fep3_plugin_c_arya_SDestructionManager
                        {reinterpret_cast<fep3_plugin_c_arya_HDestructionManager>(static_cast<c::arya::DestructionManager*>(child_reference_manager))
                        , wrapper::arya::Destructor::destroy
                        };
                    callback
                        (destination
                        , child_reference_manager_access
                        , AccessCreator()(child.get())
                        );
                }
                return fep3_plugin_c_interface_error_none;
            }
            else
            {
                return fep3_plugin_c_interface_error_invalid_handle;
            }
        }
        catch(...)
        {
            return fep3_plugin_c_interface_error_exception_caught;
        }
    }

    static inline fep3_plugin_c_InterfaceError getNumberOfChildren
        (handle_type handle
        , size_t* result
        ) noexcept
    {
        return WrapperHelper::callWithResultParameter
            (handle
            , &fep3::arya::IPropertyNode::getNumberOfChildren
            , [](auto result)
                {
                    return result;
                }
            , result
            );
    }

    static inline fep3_plugin_c_InterfaceError getChild
        (handle_type handle
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access
        , fep3_arya_SIPropertyNode* property_node_access
        , const char* name
        ) noexcept
    {
        return WrapperHelper::getSharedPtr
            (handle
            , &fep3::arya::IPropertyNode::getChild
            , destruction_manager_access
            , property_node_access
            , AccessCreator()
            , name
            );
    }

    static inline fep3_plugin_c_InterfaceError isChild
        (handle_type handle
        , bool* result
        , const char* name
        ) noexcept
    {
        return WrapperHelper::callWithResultParameter
            (handle
            , &fep3::arya::IPropertyNode::isChild
            , [](bool result)
                {
                    return result;
                }
            , result
            , name
            );
    }
};

} // namespace arya
} // namespace wrapper

namespace access
{
namespace arya
{

/// @cond no_documentation
template<typename access_type>
PropertyNode<access_type>::PropertyNode
    (const access_type& access
    , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
    )
{
    this->_access = access;
    addDestructors(std::move(destructors));
}

template<typename access_type>
std::string detail::PropertyNode<access_type, arya::Helper::Immutable>::getName() const
{
    return callWithResultCallback<std::string>
        (_access._handle
        , _access.getName
        , [](auto result)
            {
                return result;
            }
        );
}

template<typename access_type>
std::string detail::PropertyNode<access_type, arya::Helper::Immutable>::getValue() const
{
    return callWithResultCallback<std::string>
        (_access._handle
        , _access.getValue
        , [](auto result)
            {
                return result;
            }
        );
}

template<typename access_type>
std::string detail::PropertyNode<access_type, arya::Helper::Immutable>::getTypeName() const
{
    return callWithResultCallback<std::string>
        (_access._handle
        , _access.getTypeName
        , [](auto result)
            {
                return result;
            }
        );
}

template<typename access_type>
fep3::Result detail::PropertyNode<access_type, arya::Helper::Mutable>::setValue(const std::string& value, const std::string& type_name)
{
    return this->template callWithResultCallback<fep3::Result>
        (this->_access._handle
        , this->_access.setValue
        , &getResult
        , value.c_str()
        , type_name.c_str()
        );
}

template<typename access_type>
bool detail::PropertyNode<access_type, arya::Helper::Immutable>::isEqual(const fep3::arya::IPropertyNode& other) const
{
    return callWithResultParameter
        (_access._handle
        , _access.isEqual
        , wrapper::arya::PropertyNode<fep3_arya_const_HIPropertyNode>::AccessCreator()(&other)
        );
}

template<typename access_type>
void detail::PropertyNode<access_type, Helper::Mutable>::reset()
{
    return this->call
        (this->_access._handle
        , this->_access.reset
        );
}

template<typename access_type>
std::vector<std::shared_ptr<IPropertyNode>> detail::PropertyNode<access_type, arya::Helper::Immutable>::getChildren() const
{
    std::vector<std::shared_ptr<IPropertyNode>> result;

    fep3_plugin_c_InterfaceError error = (*_access.getChildren)
        (_access._handle
        , []
            (void* destination
            , fep3_plugin_c_arya_SDestructionManager reference_manager_access
            , fep3_arya_SIPropertyNode property_node_access
            )
            {
                std::deque<std::unique_ptr<c::arya::IDestructor>> property_node_destructors;
                // shared ownership: release reference to remote object when local object is destroyed
                property_node_destructors.push_back(std::make_unique<access::arya::Destructor<fep3_plugin_c_arya_SDestructionManager>>
                    (reference_manager_access));

                (*static_cast<decltype(result)*>(destination)).push_back(std::make_shared<access::arya::PropertyNode<fep3_arya_SIPropertyNode>>
                    (property_node_access
                    , std::move(property_node_destructors)
                    ));
            }
        , static_cast<void*>(&result)
        );
    if(fep3_plugin_c_interface_error_none != error)
    {
        throw Exception(error);
    }
    return result;
}

template<typename access_type>
size_t detail::PropertyNode<access_type, arya::Helper::Immutable>::getNumberOfChildren() const
{
    return callWithResultParameter
        (_access._handle
        , _access.getNumberOfChildren
        );
}

template<typename access_type>
std::shared_ptr<fep3::arya::IPropertyNode> detail::PropertyNode<access_type, arya::Helper::Immutable>::getChild
    (const std::string& name) const
{
    return getSharedPtr<access::arya::PropertyNode<fep3_arya_SIPropertyNode>, fep3_arya_SIPropertyNode>
        (_access._handle
        , _access.getChild
        , name.c_str()
        );
}

template<typename access_type>
bool detail::PropertyNode<access_type, arya::Helper::Immutable>::isChild
    (const std::string& name) const
{
    return callWithResultParameter
        (_access._handle
        , _access.isChild
        , name.c_str()
        );
}
/// @endcond no_documentation

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
