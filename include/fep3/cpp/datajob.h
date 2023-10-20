/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/base/properties/propertynode.h>
#include <fep3/cpp/datajob_base.h>

namespace fep3 {
namespace cpp {

using fep3::base::PropertyVariable;

/**
 * A DataJob will automatically register on initialization time the data to the fep::IDataRegistry
 * It will also set up the default timing behaviour of its process method
 */
class DataJob : public fep3::core::Job, public fep3::base::Configuration, public DataJobBase {
public:
    /// type of an execution function to execute if job is triggered
    using ExecuteCallback = fep3::core::Job::ExecuteCallback;

    /*
     *
     * @brief trait to check if the last element of variadic parameters is a valid callable
     * with return type as fep3::Result
     *
     **/
    template <typename... A>
    struct args_last_callable
        : std::bool_constant<(std::is_invocable_r_v<fep3::Result, A, fep3::arya::Timestamp>, ...)>
    // unary right fold on comma operator (Exp , ...), it will evaluate the last element
    {
    };

    /**
     * @brief CTOR
     * This constructor will be called if the parameters have any callable
     *
     * @param[in] name Name of the job
     * @param[in] args Other arguments
     */
    template <typename... Args, std::enable_if_t<args_last_callable<Args...>::value, bool> = true>
    DataJob(const std::string& name, Args&&... args)
        : Job(name, std::forward<Args>(args)...), Configuration("job_" + name)
    {
    }

    /**
     * @brief CTOR
     * This constructor will be called if the parameters do not have any callable
     *
     * @param[in] name Name of the job
     * @param[in] args Other arguments
     */
    template <typename... Args, std::enable_if_t<!args_last_callable<Args...>::value, bool> = true>
    DataJob(const std::string& name, Args&&... args)
        : Job(name,
              std::forward<Args>(args)...,
              [&](Timestamp time_of_ex) { return process(time_of_ex); }),
          Configuration("job_" + name)
    {
    }

    /**
     * @brief Deleted Copy CTOR
     */
    DataJob(const DataJob&) = delete;

    /**
     * @brief Deleted Move CTOR
     */
    DataJob(DataJob&&) = delete;

    /**
     * @brief Deleted Copy assignment operator
     *
     * @return DataJob&
     */
    DataJob& operator=(const DataJob&) = delete;

    /**
     * @brief Deleted Move assignment operator
     *
     * @return DataJob&
     */
    DataJob& operator=(DataJob&&) = delete;

    /**
     * @brief Default DTOR
     */
    virtual ~DataJob() = default;

    /**
     * @brief adds the created reader and writer to the data registry and clock
     *
     * @param[in] components the components to add to
     * @return fep3::Result
     */
    fep3::Result addDataToComponents(const fep3::IComponents& components);

    /**
     * @brief remove reader and writer references to the data registry and remove the corresponding
     * readers and writers from the data registry.
     *
     * @param[in] components the components to remove from
     * @return fep3::Result
     */
    fep3::Result removeDataFromComponents(const fep3::IComponents& components);

    /**
     * @brief create a data reader and return a pointer to it.
     * This function takes into account the type of the data job and creates the data reader
     * accordingly. For a time triggered data job the created data reader will forward samples whose
     * timestamp is less than the current simulation time only. For a data triggered data job the
     * created data reader will forward samples whose timestamp is less than or equal to the current
     * simulation time.
     *
     * @param[in] name the data reader's name
     * @param[in] type the data reader's type
     * @return DataReader* the created data reader
     */
    DataReader* addDataIn(const std::string& name, const fep3::IStreamType& type);

    /**
     * @brief create a data reader having a specific data sample queue size and return a pointer to
     * it. This function takes into account the type of the data job and creates the data reader
     * accordingly. For a time triggered data job the created data reader will forward samples whose
     * timestamp is less than the current simulation time only. For a data triggered data job the
     * created data reader will forward samples whose timestamp is less than or equal to the current
     * simulation time.
     *
     * @param[in] name the data reader's name
     * @param[in] type the data reader's type
     * @param[in] queue_size the data reader's queue_size
     * @return DataReader* the created data reader
     */
    DataReader* addDataIn(const std::string& name,
                          const fep3::IStreamType& type,
                          size_t queue_size);

protected:
    /**
     * The process method to override.
     * implement your functionality here.
     * you do not need to lock something in here!
     * @param[in] time_of_execution current time of execution from the ClockService
     *                          this is the beginning time of the execution in simulation time
     * @return fep3::Result If you return an error here the scheduler might stop execution
     * immediately!
     */
    virtual fep3::Result process(fep3::Timestamp time_of_execution);

public: // Job
    /**
     * The reset method to override.
     * implement your reset functionality here.
     * This method is called each time before the @p process method is called
     * @return fep3::Result If you return an error here the scheduler might stop execution
     * immediately!
     */
    fep3::Result reset() override;

private:
    ///@copydoc fep3::core::Job::executeDataIn
    fep3::Result executeDataIn(fep3::Timestamp time_of_execution) override;
    ///@copydoc fep3::core::Job::executeDataOut
    fep3::Result executeDataOut(fep3::Timestamp time_of_execution) override;
};

/// @cond nodoc
namespace arya {
using DataJob [[deprecated(
    "Since 3.1, fep3::cpp::arya::DataJob is deprecated. Please use fep3::cpp::DataJob")]] =
    fep3::cpp::DataJob;

template <typename... Args>
auto addToComponents(Args&&... args)
    -> decltype(fep3::cpp::addToComponents(std::forward<Args>(args)...))
{
    return fep3::cpp::addToComponents(std::forward<Args>(args)...);
}

template <typename... Args>
auto removeFromComponents(Args&&... args)
    -> decltype(fep3::cpp::removeFromComponents(std::forward<Args>(args)...))
{
    return fep3::cpp::removeFromComponents(std::forward<Args>(args)...);
}
} // namespace arya
/// @endcond nodoc

} // namespace cpp
} // namespace fep3
