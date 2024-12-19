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

#include <fep3/base/sample/raw_memory_intf.h>
#include <fep3/fep3_timestamp.h>

#include <memory>

namespace fep3 {
namespace arya {
/**
 * @brief Class for resource management of a pooled pointer reference.
 *
 * @tparam T The object pointer type to manage
 */
template <typename T>
using data_read_ptr = std::shared_ptr<T>;

/**
 * @brief Interface for one data sample.
 * A data sample is a abstraction for raw memory with a corresponding
 * timestamp of creation (@ref fep3::arya::IDataSample::getTime) and
 * a counter (@ref fep3::arya::IDataSample::getCounter)
 */
class IDataSample {
protected:
    /// DTOR
    ~IDataSample() = default;

public:
    /**
     * @brief Get the time stamp of the object.
     * Usually the time of creation of the data as a reference to the main simulation time
     * (fep3::arya::IClockService::getTime).
     *
     * @return The timestamp of the data sample in simulation time
     */
    virtual arya::Timestamp getTime() const = 0;

    /**
     * @brief Get the Size of the data in bytes
     *
     * @return size_t The size of the data in bytes
     */
    virtual size_t getSize() const = 0;

    /**
     * @brief Get the counter of the sample set by the sender
     *
     * @return uint32_t the counter
     */
    virtual uint32_t getCounter() const = 0;

    /**
     * @brief Calls the IRawMemory::set function to copy the memory content of the sample to the
     * given @p writeable_memory
     *
     * @param[out] writeable_memory The memory to copy the memory to (the callback to copy is the
     * @ref fep3::IRawMemory::set call)
     * @return size_t Returns the size in byte which are copied (usually it's the return value of
     * @ref fep3::IRawMemory::set)
     */
    virtual size_t read(arya::IRawMemory& writeable_memory) const = 0;

    /**
     * @brief Set the timestamp of the sample.
     * If set to 0 or lower the time will be set to the current simulation time (@ref
     * fep3::arya::IClockService::getTime) during transmission (see @ref
     * fep3::arya::IDataRegistry::IDataWriter::write).
     *
     * @param[in] time The simulation time
     */
    virtual void setTime(const arya::Timestamp& time) = 0;

    /**
     * @brief Set the counter
     *
     * @param[in] counter The counter set by the writer
     */
    virtual void setCounter(uint32_t counter) = 0;

    /**
     * @brief Changes and copies the given memory to the sample's internal memory.
     *
     * @param[in] readable_memory The memory to copy (uses @ref fep3::IRawMemory::cdata and @ref
     * fep3::IRawMemory::size to obtain)
     * @return size_t Returns the size in bytes that were copied
     */
    virtual size_t write(const arya::IRawMemory& readable_memory) = 0;
};

} // namespace arya
using arya::data_read_ptr;
using arya::IDataSample;
} // namespace fep3
