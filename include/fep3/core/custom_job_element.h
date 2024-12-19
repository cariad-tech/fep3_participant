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

#include <fep3/base/properties/propertynode.h>
#include <fep3/components/job_registry/job_registry_intf.h>
#include <fep3/components/logging/easy_logger.h>
#include <fep3/core/default_job.h>

namespace fep3::core {

/**
 * The base job element for customizing.
 * Derive from it for custom behaviors and to pass it
 * to the constructor of fep3::core::DefaultJobElement
 */
class CustomJobElement : public fep3::base::arya::Configuration,
                         public fep3::base::arya::EasyLogging {
public:
    /// Type for shared pointer of @ref fep3::core::DefaultJob
    using JobPtr = std::shared_ptr<fep3::core::DefaultJob>;
    /// Type for shared pointer of @ref fep3::catelyn::JobConfiguration
    using JobConfigPtr = std::unique_ptr<fep3::catelyn::JobConfiguration>;

    /**
     * @brief CTOR
     *
     * @param config_root_name name for root node in configuration
     */
    CustomJobElement(const std::string& config_root_name) : Configuration(config_root_name){};

    /**
     * @brief Default DTOR
     */
    virtual ~CustomJobElement() = default;

    /**
     * @copydoc fep3::base::IElement::getTypename
     * @remark Must be implemented by user
     */
    virtual std::string getTypename() const = 0;

    /**
     * @copydoc fep3::base::IElement::getVersion
     * @remark Must be implemented by user
     */
    virtual std::string getVersion() const = 0;

    /**
     * @brief Create a job and its job configuration.
     * This function shall not throw or forward exceptions but return an approriate fep3::Result
     * instead.
     *
     * @return {fep3::Result, JobPtr, JobConfigPtr}
     * @remark Must be implemented by user
     */
    virtual std::tuple<fep3::Result, JobPtr, JobConfigPtr> createJob(
        const fep3::arya::IComponents&) = 0;

    /**
     * @brief Destroy a job
     *
     * @return fep3::Result
     * @remark Must be implemented by user
     */
    virtual fep3::Result destroyJob() = 0;

    /**
     * @brief Do additional actions required in state transition load
     *
     * It does nothing in default implementation.
     *
     * @return fep3::Result
     * @remark Called in @ref fep3::base::IElement::loadElement
     *         Override this method in the child class.
     */
    virtual fep3::Result load(const fep3::arya::IComponents&)
    {
        return {};
    }

    /**
     * @brief Do additional actions required in state transition initialize
     *
     * It does nothing in default implementation.
     *
     * @return fep3::Result
     * @remark Called in @ref fep3::base::IElement::initialize
     *         Override this method in the child class
     */
    virtual fep3::Result initialize(const fep3::arya::IComponents&)
    {
        return {};
    }

    /**
     * @brief Do additional actions required in state transition start
     *
     * It does nothing in default implementation.
     *
     * @return fep3::Result
     * @remark Called in @ref fep3::base::IElement::run
     *         Override this method in the child class.
     */
    virtual fep3::Result run()
    {
        return {};
    }

    /**
     * @brief Do additional actions required in state transition stop
     *
     * It does nothing in default implementation.
     *
     * @remark Called in @ref fep3::base::IElement::stop.
     *         Override this method in the child class.
     */
    virtual void stop()
    {
    }

    /**
     * @brief Do additional actions required in state transition deinitialize
     *
     * It does nothing in default implementation.
     *
     * @remark Called in @ref fep3::base::IElement::deinitialize.
     *         Override this method in the child class.
     */
    virtual void deinitialize(const fep3::arya::IComponents&)
    {
    }

    /**
     * @brief Do additional actions required in state transition unload
     *
     * It does nothing in default implementation.
     *
     * @remark Called in @ref fep3::base::IElement::unloadElement.
     *         Override this method in the child class.
     */
    virtual void unload(const fep3::arya::IComponents&)
    {
    }
};

} // namespace fep3::core
