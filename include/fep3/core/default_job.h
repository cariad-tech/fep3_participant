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
#include <fep3/components/data_registry/data_registry_intf.h>
#include <fep3/components/job_registry/job_configuration.h>
#include <fep3/components/job_registry/job_intf.h>
#include <fep3/components/logging/easy_logger.h>
#include <fep3/core/data_io_container_intf.h>

namespace fep3::arya {
class IComponents;
} // namespace fep3::arya

namespace fep3::core {

/**
 * @brief Class for creating a default job
 *
 * Inherit from it to create a default job to run in a @ref fep3::core::DefaultJobElement
 */
class DefaultJob : public fep3::arya::IJob,
                   public fep3::base::EasyLogging,
                   public fep3::base::Configuration {
public:
    /**
     * @brief CTOR
     *
     * @param job_name job's name
     */
    DefaultJob(const std::string& job_name) : _job_name(job_name), Configuration(job_name)
    {
    }

    /**
     * @brief Default DTOR
     */
    virtual ~DefaultJob() = default;

    /**
     * @brief Create the DataReaders and DataWriters
     *
     * @param components
     * @param io_container container for data IOs
     * @param job_config job configuration for the given name you return in @ref getName
     */
    virtual void createDataIOs(const fep3::arya::IComponents& components,
                               fep3::core::IDataIOContainer& io_container,
                               const fep3::catelyn::JobConfiguration& job_config) = 0;

    /**
     * @brief Return name of the job
     *
     * @return name of the job
     */
    virtual std::string getName()
    {
        return _job_name;
    }

    /**
     * @brief Sets the DataIOContainer
     *
     * @param io_container
     */
    void setDataIOContainer(fep3::core::IDataIOContainer& io_container)
    {
        _container = &io_container;
    }

    /**
     * @copydoc fep3::arya::IJob::executeDataIn
     */
    fep3::Result executeDataIn(fep3::Timestamp time_of_execution) override
    {
        return _container->executeDataIn(time_of_execution);
    }

    /**
     * @copydoc fep3::arya::IJob::executeDataOut
     */
    fep3::Result executeDataOut(fep3::Timestamp time_of_execution) override
    {
        return _container->executeDataOut(time_of_execution);
    }

    /**
     * @brief Create default property variables
     *
     * @return fep3::Result
     * @remark Called in @ref fep3::base::IElement::loadElement.
     */
    fep3::Result createDefaultPropertyVariables()
    {
        FEP3_RETURN_IF_FAILED(registerPropertyVariable(_purged_samples_log_capacity,
                                                       FEP3_PURGED_SAMPLES_LOG_CAPACITY_PROPERTY));

        return registerPropertyVariable(_clear_input_signal_queues,
                                        FEP3_CLEAR_INPUT_SIGNALS_QUEUES_PROPERTY);
    }

    /**
     * @brief Evaluates and applies default property variables
     *
     * @return fep3::Result
     * @remark Called in @ref fep3::base::IElement::initialize.
     */
    fep3::Result applyDefaultPropertyVariables()
    {
        if (_purged_samples_log_capacity < 0) {
            RETURN_ERROR_DESCRIPTION(
                fep3::ERR_INVALID_ARG,
                "Invalid value of '%lld' for property '%s'. Value has to be >= '0'.",
                static_cast<int64_t>(_purged_samples_log_capacity),
                FEP3_PURGED_SAMPLES_LOG_CAPACITY_PROPERTY);
        }
        const DataIOContainerConfiguration configuration = {
            static_cast<size_t>(_purged_samples_log_capacity), _clear_input_signal_queues};
        _container->setConfiguration(configuration);

        return {};
    }

    /**
     * @brief Remove default property variables
     *
     * @return fep3::Result
     * @remark Called in @ref fep3::base::IElement::unloadElement.
     */
    fep3::Result removeDefaultPropertyVariables()
    {
        return unregisterPropertyVariable(_purged_samples_log_capacity,
                                          FEP3_PURGED_SAMPLES_LOG_CAPACITY_PROPERTY);
    }

    /**
     * @brief Log information regarding data inputs and outputs
     *
     * @remark Called in @ref fep3::base::IElement::stop.
     */
    void logIOInfo() const
    {
        _container->logIOInfo(getLogger().get());
    }

    /**
     * @brief Do additional actions required in state transition initialize
     *
     * It does nothing in default implementation.
     *
     * @return fep3::Result
     * @remark Called in @ref fep3::base::IElement::initialize.
     *         Override this method in the child class.
     */
    virtual fep3::Result initialize(const fep3::arya::IComponents&)
    {
        return {};
    };

    /**
     * @brief Do additional actions required in state transition start
     *
     * It does nothing in default implementation.
     *
     * @return fep3::Result
     * @remark Called in @ref fep3::base::IElement::run.
     *         Override this method in the child class.
     */
    virtual fep3::Result start()
    {
        return {};
    }

    /**
     * @brief Do additional actions required in state transition stop
     *
     * It does nothing in default implementation.
     *
     * @return fep3::Result
     * @remark Called in @ref fep3::base::IElement::stop.
     *         Override this method in the child class.
     */
    virtual fep3::Result stop()
    {
        return {};
    }

    /**
     * @brief Do additional actions required in state transition deinitialize
     *
     * It does nothing in default implementation.
     *
     * @return fep3::Result
     * @remark Called in @ref fep3::base::IElement::deinitialize.
     *         Override this method in the child class.
     */
    virtual fep3::Result deinitialize()
    {
        return {};
    };

    /**
     * @brief Register property variables during initialization
     *
     * It does nothing in default implementation.
     *
     * @return fep3::Result
     * @remark Inherite from fep3::base::Configuration.
     *         Override it to register property.
     */
    fep3::Result registerPropertyVariables() override
    {
        return {};
    }

    /**
     * @brief Unregister property variables during deinitialization
     *
     * @return fep3::Result
     * @remark Inherite from fep3::base::Configuration.
     *         Override it to unregister properties.
     */
    fep3::Result unregisterPropertyVariables() override
    {
        return {};
    }

private:
    ///@cond nodoc
    std::string _job_name;
    fep3::core::IDataIOContainer* _container = nullptr;
    base::PropertyVariable<int64_t> _purged_samples_log_capacity{
        FEP3_PURGED_SAMPLES_LOG_CAPACITY_DEFAULT_VALUE};
    base::PropertyVariable<bool> _clear_input_signal_queues{false};
    ///@endcond nodoc
};
} // namespace fep3::core
