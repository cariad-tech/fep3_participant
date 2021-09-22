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
#include <memory>
#include <list>
#include <map>

#include <fep3/fep3_participant_export.h>
#include <fep3/participant/element_intf.h>
#include <fep3/components/logging/easy_logger.h>


namespace fep3
{
namespace core
{
namespace arya
{

/**
 * This is the base class for every FEP3 Element.
 * To implement your own FEP3 Element, derive from this class.
 */
class ElementBase : public fep3::arya::IElement,
    public fep3::base::arya::EasyLogging

{
public:
    /**
     * Deleted default CTOR.
     */
    ElementBase() = delete;
protected:
    /**
     * CTOR
     *
     * @param[in] type_name Name of the type the element presents
     * @param[in] version_info Version information of the element
     */
    ElementBase(std::string type_name, std::string version_info) : _type_name(std::move(type_name)),
                                                                   _version_info(std::move(version_info)),
                                                                   _components(nullptr)
    {
    }
public:
    /**
     * Default DTOR.
     *
     */
    virtual ~ElementBase() = default;

protected:
    /**
     * Return the typename of the element.
     * The typename represents the type the element implementing.
     *
     * @remark This is not the instance name of the element!
     *         the instance name is usually the same as the participant this element is loaded in.
     *
     * @return std::string the typename
     */
    std::string getTypename() override
    {
        return _type_name;
    }

    /**
     * Return the version of the element.
     * The version of the element implementation.
     *
     * @remark This is the instance version of the element type
     *         and will be used only for information at the moment!
     *         There is no further functionality or checks on that!
     *
     * @return std::string the version as string (this is vendor dedendent, and only for information!)
     */
    std::string getVersion() override
    {
        return _version_info;
    }
    /**
     * Internal callback to load the element.
     *
     * @param[in] components reference to the components. this pointer is valid until unload was called.
     *
     * @return Result error code
     * @retval NO_ERROR if succeded
     */
    Result loadElement(const fep3::arya::IComponents& components) override
    {
        auto init_logger_res = initLogger(components, "element");
        if (isFailed(init_logger_res))
        {
            _components = nullptr;
            return init_logger_res;
        }

        _components = &components;

        auto load_res = load();
        if (isFailed(load_res))
        {
            _components = nullptr;
        }
        return load_res;
    }
    /**
     * Internal callback to unload the element.
     */
    void unloadElement() override
    {
        unload();
        deinitLogger();
        _components = nullptr;
    }

    /**
     * Retrieves the component pointer.
     * This pointer is only valid after loadElement() was successfully called and before unloadElement() was called.
     *
     * @return the components pointer
     * @retval nullptr if the component pointer is not valid
     */
    const fep3::arya::IComponents* getComponents() const
    {
        return _components;
    }

public:
    /**
     * Callback to load the element.
     *
     * @return Result error code
     * @retval NO_ERROR if succeded
     */
    virtual Result load()
    {
        return {};
    };
    /**
     * Callback to cleanup the element before unloading.
     *
     */
    virtual void unload()
    {

    };
    /**
     * Does nothing.
     *
     * @return NO_ERROR
     * @remark To perform initialization of your element implementation
     *  override this method in your ElementBase's child class.
     *  Reinitialization means that your element has already been initialized in the past
     *  and (thus e. g. memory has been allocated) now initializes again (thus e. g. there is
     *  no need to allocate memory again).
    */
    Result initialize() override
    {
        return {};
    }
    /**
     * Does nothing.
     *
     * @remark To perform deinitialization of your element implementation
     *  override this method in your ElementBase's child class.
     *  Reinitialization means that your element has already been initialized in the past
     *  and (thus e. g. memory has been allocated) now initializes again (thus e. g. there is
     *  no need to allocate memory again).
    */
    void deinitialize() override
    {
    }
    /**
     * Does nothing.
     *
     * @return NO_ERROR
     * @remark To put your element into run state
     *  override this method in your ElementBase's child class.
     */
    Result run() override
    {
        return {};
    }
    /**
     * Does nothing.
     *
     * @remark To perform reinitialization of your element implementation
     *  override this method in your ElementBase's child class.
     *  Reinitialization means that your element has already been initialized in the past
     *  and (thus e. g. memory has been allocated) now initializes again (thus e. g. there is
     *  no need to allocate memory again).
    */
    void stop() override
    {
    }

private:
    ///type name of the element
    std::string _type_name;
    ///versioninformation of the element
    std::string _version_info;
    ///collection of components interfaces
    const fep3::arya::IComponents* _components;
};

} // namespace arya
using arya::ElementBase;
} // namespace core
} // namespace fep3
