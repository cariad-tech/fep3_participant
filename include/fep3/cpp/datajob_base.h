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

#include <fep3/components/logging/easy_logger.h>
#include <fep3/core/data/data_reader.h>
#include <fep3/core/data/data_writer.h>
#include <fep3/core/job.h>

namespace fep3 {
namespace cpp {

using core::DataReader;
using core::DataWriter;

/**
 * A DataJob will automatically register on initialization time the data to the fep::IDataRegistry
 * It will also set up the default timing behaviour of its process method
 */

class DataJobBase : public fep3::base::EasyLogging {
public:
    /**
     * Creates a fep3::core::DataReader with a default queue capacity of 2
     * If you receive more than two samples before reading, oldest samples will be dropped and
     * therefore are lost.
     * @param[in] name name of signal
     * @param[in] type type of signal
     * @param[in] job_configuration configuration of the data job
     * @return created DataReader
     */
    DataReader* addDataIn(const std::string& name,
                          const fep3::IStreamType& type,
                          std::unique_ptr<JobConfiguration> job_configuration);

    /**
     * Creates a fep3::core::DataReader with a specific queue capacity
     * If you receive more samples than specified by @c queue_size before reading, these samples
     * will be lost.
     * @param[in] name name of signal
     * @param[in] type type of signal
     * @param[in] queue_size capacity of the internal DataReader item queue
     * @param[in] job_configuration configuration of the data job
     * @return created DataReader
     */
    DataReader* addDataIn(const std::string& name,
                          const fep3::IStreamType& type,
                          size_t queue_size,
                          std::unique_ptr<JobConfiguration> job_configuration);

    /**
     * Creates a fep3::core::DataWriter with a default queue capacity of 1
     * If you queue more than one sample before flushing these samples will be lost.
     * @param[in] name name of signal
     * @param[in] type type of signal
     * @return created DataWriter
     */
    DataWriter* addDataOut(const std::string& name, const fep3::IStreamType& type);

    /**
     * Creates a fep3::core::DataWriter with a fixed maximum queue capacity.
     * If you queue more than @c queue_size samples before flushing these samples will be lost.
     * @param[in] name name of signal
     * @param[in] type type of signal
     * @param[in] queue_size maximum capacity of the queue
     * @throw throws if queue_size parameter is <= 0
     * @return created DataWriter
     */
    DataWriter* addDataOut(const std::string& name,
                           const fep3::IStreamType& type,
                           size_t queue_size);

    /**
     * Creates a fep3::core::DataWriter with infinite queue capacity.
     * Pushing a sample into the queue extends the queue.
     * Every sample that is queued will be written on flush.
     * @param[in] name name of signal
     * @param[in] type type of signal
     * @return created DataWriter
     * @remark pushing big numbers of samples into the queue might lead to out-of-memory situations
     */
    DataWriter* addDynamicDataOut(const std::string& name, const fep3::IStreamType& type);

    /**
     * Resize the DataReaderBacklog of a DataReader.
     * @param[in] name name of the DataReader to be resized.
     * @param[in] queue_capacity capacity of the DataReaderBacklog after resizing.
     * @return fep3::Result
     * @return ERR_NOT_FOUND found no DataReader with the given @p name.
     */
    fep3::Result reconfigureDataIn(const std::string& name, size_t queue_capacity);

    /**
     * @brief adds the created reader and writer to the data registry and clock
     *
     * @param[in] components the components to add to
     * @param[in] job_name name of job
     * @return fep3::Result
     */
    fep3::Result addDataToComponents(const fep3::IComponents& components,
                                     const std::string& job_name);

    /**
     * @deprecated
     * @brief remove reader and writer references to the data registry without removing the
     * corresponding readers and writers from the data registry.
     *
     * @return fep3::Result
     * @see fep3::DataJob::removeDataFromComponents(const fep3::IComponents& components)
     */
    [[deprecated("fep3::DataReader::removeFromDataRegistry() is deprecated. Please use "
                 "fep3::DataReader::removeFromDataRegistry(fep3::IDataRegistry& data_registry) "
                 "instead.")]] fep3::Result
    removeDataFromComponents();

    /**
     * @brief remove reader and writer references to the data registry and remove the corresponding
     * readers and writers from the data registry.
     *
     * @param[in] components the components to remove from
     * @param[in] job_name name of job
     * @return fep3::Result
     */
    fep3::Result removeDataFromComponents(const fep3::IComponents& components,
                                          const std::string& job_name);

protected:
    ~DataJobBase() = default;

    ///@copydoc fep3::core::Job::executeDataIn
    fep3::Result dataIn(fep3::Timestamp /*time_of_execution*/);
    ///@copydoc fep3::core::Job::executeDataOut
    fep3::Result dataOut(fep3::Timestamp /*time_of_execution*/);

private:
    /// the readers
    std::list<core::DataReader> _readers;
    /// the writers
    std::list<core::DataWriter> _writers;
};

/**
 * @brief Datajobs will be removed from the job registry of the @p component.
 * If one job cannot be removed, the function will still try to remove the following ones within @p
 * job_names.
 *
 * @param[in] jobs List of jobs to be removed
 * @param[in] components The component container in which the job registry will be looked up
 * @return fep3::Result.
 *           If it's an actual error, it is the error code for the job that failed last.
 *           If more than one remove fails, the error description contains a list of error
 * descriptions separated by ';'.
 * @retval ERR_NOERROR Everything went fine
 * @retval ERR_NOT_FOUND A job with the given name is not registered
 * @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
 */
template <typename DataJob,
          typename = std::enable_if_t<std::is_base_of<DataJobBase, DataJob>::value>>
inline fep3::Result removeFromComponents(const std::vector<std::shared_ptr<DataJob>>& jobs,
                                         const fep3::IComponents& components)
{
    std::vector<std::string> job_names;
    for (const auto& current_job_to_add: jobs) {
        job_names.push_back(current_job_to_add->getJobInfo().getName());
        current_job_to_add->removeDataFromComponents(components);
    }
    return core::removeFromComponents(job_names, components);
}

/**
 * @copydoc fep3::cpp::removeFromComponents
 */
template <typename DataJob,
          typename = std::enable_if_t<std::is_base_of<DataJobBase, DataJob>::value>>
inline fep3::Result removeFromComponents(
    const std::initializer_list<std::shared_ptr<DataJob>>& jobs,
    const fep3::IComponents& components)
{
    // Enable initializer_list as parameter
    return removeFromComponents(std::vector<std::shared_ptr<DataJob>>{jobs}, components);
}

/**
 * @brief Datajobs will be added to the job registry of the @p component.
 *        Additionally, there data reader and writer will be registered at the DataRegistery
 * If one job cannot be added, the function returns not adding the following ones and rolling back!
 *
 * @param[in] jobs List of jobs to be added
 * @param[in] components The component container in which the job registry will be looked up
 * @return fep3::Result
 * @retval ERR_NOERROR Everything went fine
 * @retval ERR_RESOURCE_IN_USE A job with the given name is already registered
 * @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
 */
template <typename DataJob,
          typename = std::enable_if_t<std::is_base_of<DataJobBase, DataJob>::value>>
inline fep3::Result addToComponents(const std::vector<std::shared_ptr<DataJob>>& jobs,
                                    const fep3::IComponents& components)
{
    for (const auto& current_job_to_add: jobs) {
        const auto res_adding_data = current_job_to_add->addDataToComponents(components);
        if (res_adding_data) {
            auto res_adding_job =
                core::addToComponents(current_job_to_add->getJobInfo().getName(),
                                      current_job_to_add,
                                      current_job_to_add->getJobInfo().getConfig(),
                                      components);
            if (!res_adding_job) {
                removeFromComponents(jobs, components);
                return res_adding_job;
            }
        }
        else {
            removeFromComponents(jobs, components);
            return res_adding_data;
        }
    }
    return {};
}

/**
 * @copydoc fep3::cpp::addToComponents
 */
template <typename DataJob,
          typename = std::enable_if_t<std::is_base_of<DataJobBase, DataJob>::value>>
inline fep3::Result addToComponents(const std::initializer_list<std::shared_ptr<DataJob>>& jobs,
                                    const fep3::IComponents& components)
{
    // Enable initializer_list as parameter
    return addToComponents(std::vector<std::shared_ptr<DataJob>>{jobs}, components);
}

} // namespace cpp
} // namespace fep3
