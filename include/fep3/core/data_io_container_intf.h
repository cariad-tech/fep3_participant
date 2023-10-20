/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2023 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/fep3_result_decl.h>
#include <fep3/fep3_timestamp.h>

#include <string>

namespace fep3::arya {
class IComponents;
class IStreamType;
class IDataRegistry;
class IClockService;
} // namespace fep3::arya

namespace fep3::catelyn {
class JobConfiguration;
} // namespace fep3::catelyn

namespace fep3::core::arya {
class DataReader;
class DataWriter;
} // namespace fep3::core::arya

namespace fep3::core {
/// @cond nodoc
using DataReader = arya::DataReader;
using DataWriter = arya::DataWriter;
/// @endcond nodoc
} // namespace fep3::core

namespace fep3::core {
/**
 * @brief Data IOs container interface
 */
class IDataIOContainer {
public:
    /**
     * @brief Default DTOR
     */
    virtual ~IDataIOContainer() = default;

    /**
     * Create a data reader having a specific data sample queue size and return a pointer to
     * it. This function takes into account the type of the data job and creates the data reader
     * accordingly. For a time triggered data job the created data reader will forward samples whose
     * timestamp is less than the current simulation time only. For a data triggered data job the
     * created data reader will forward samples whose timestamp is less than or equal to the current
     * simulation time.
     *
     * @param[in] name data reader's name
     * @param[in] type data reader's type
     * @param[in] queue_size data reader's queue_size
     * @param[in] job_configuration job configuration
     * @return pointer of created DataReader, can be nullptr
     */
    virtual fep3::core::DataReader* addDataIn(
        const std::string& name,
        const fep3::arya::IStreamType& type,
        size_t queue_size,
        const fep3::catelyn::JobConfiguration& job_configuration) = 0;

    /**
     * Creates a fep3::core::DataWriter with a fixed maximum queue capacity.
     * If you queue more than @c queue_size samples before flushing these samples will be lost.
     *
     * @param[in] name name of signal
     * @param[in] type type of signal
     * @param[in] queue_size maximum capacity of the queue or DATA_WRITER_QUEUE_SIZE_DYNAMIC
     * @return pointer of created DataWriter, can be nullptr
     */
    virtual fep3::core::DataWriter* addDataOut(const std::string& name,
                                               const fep3::arya::IStreamType& type,
                                               size_t queue_size) = 0;

    /**
     * @brief Read input samples. Provides samples to all DataReaders added with
     * @ref addDataIn
     *
     * @param[in] time_of_execution current simulation time
     * @return fep3::Result
     */
    virtual fep3::Result executeDataIn(fep3::Timestamp time_of_execution) = 0;

    /**
     * @brief Write output samples. Flushes all samples from all DataWriters added
     * with @ref addDataOut
     *
     * @param[in] time_of_execution current simulation time
     * @return fep3::Result
     */
    virtual fep3::Result executeDataOut(fep3::Timestamp time_of_execution) = 0;

    /**
     * @brief Add all given data writers and readers to a given registry and clock service
     *
     * @param[in] data_registry data registry to add readers and writers into
     * @param[in] clock_service clock service to add writers into
     * @return fep3::Result
     */
    virtual fep3::Result addToDataRegistry(fep3::arya::IDataRegistry& data_registry,
                                           fep3::arya::IClockService& clock_service) = 0;
    /**
     * @brief Remove all given data writers and readers from a given data registry.
     *
     * @param[in] data_registry data registry to remove data writers from.
     */
    virtual void removeFromDataRegistry(fep3::arya::IDataRegistry& data_registry) = 0;
};

} // namespace fep3::core
