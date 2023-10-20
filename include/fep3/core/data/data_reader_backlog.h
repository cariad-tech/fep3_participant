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

#include <fep3/base/queue/data_item_queue.h>
#include <fep3/components/data_registry/data_registry_intf.h>

#include <mutex>

namespace fep3 {
namespace core {
namespace arya {

/**
 * @brief Reader Backlog Queue to keep the last items (capacity) until they are read.
 * Pop calls will empty the backlog. Read calls wont.
 * Backlog will overwrite oldest item if capacity is full and a new item is received.
 */
class DataReaderBacklog : public fep3::arya::IDataRegistry::IDataReceiver {
public:
    /**
     * @brief CTOR for a Data Reader Backlog object
     *
     * @param sample_queue_capacity sample queue capacity
     * @param init_type Stream Type at init time
     */
    DataReaderBacklog(size_t sample_queue_capacity, const fep3::arya::IStreamType& init_type)
        : _sample_queue(sample_queue_capacity),
          _init_type(std::make_shared<fep3::base::arya::StreamType>(init_type))
    {
    }

    /// @cond nodoc
    DataReaderBacklog(const DataReaderBacklog&) = delete;
    DataReaderBacklog(DataReaderBacklog&&) = delete;
    DataReaderBacklog& operator=(const DataReaderBacklog&) = delete;
    DataReaderBacklog& operator=(DataReaderBacklog&&) = delete;
    /// @endcond

    /**
     * @brief DTOR
     */
    ~DataReaderBacklog()
    {
        _sample_queue.clear();
    }

    /**
     * @brief Receives a stream type item
     *
     * @param[in] type The received stream type
     */
    void operator()(const data_read_ptr<const fep3::arya::IStreamType>& type) override
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);
        _init_type = type;
    }

    /**
     * @brief Receives a data sample item
     *
     * @param[in] sample The received data sample
     */
    void operator()(const data_read_ptr<const fep3::arya::IDataSample>& sample) override
    {
        _sample_queue.pushSample(sample, sample->getTime());
    }

    /**
     * @brief Get the Front Time (the timestamp of the oldest item available)
     *
     * @return fep3::Optional<Timestamp>
     * @retval valid time queue is not empty
     * @retval invalid time queue is empty
     */
    fep3::arya::Optional<fep3::arya::Timestamp> getFrontTime()
    {
        return _sample_queue.nextTime();
    }

    /**
     * @brief retrieves the current size of the data sample backlog
     *
     * @return size_t the size as item count
     */
    size_t getSampleQueueSize() const
    {
        return _sample_queue.size();
    }

    /**
     * @brief retrieves the capacity of the data sample backlog
     *
     * @return size_t the capacity
     */
    size_t getSampleQueueCapacity() const
    {
        return _sample_queue.capacity();
    }

    /**
     * @brief pops the latest sample available
     *
     * @return data_read_ptr<const IDataSample>
     * @retval valid pointer the sample popped
     * @retval invalid pointer the queue was empty
     */
    data_read_ptr<const fep3::arya::IDataSample> popSampleLatest()
    {
        return std::get<0>(_sample_queue.popBack());
    }

    /**
     * @brief pops the oldest sample available
     *
     * @return data_read_ptr<const IDataSample>
     * @retval valid pointer the sample popped
     * @retval invalid pointer the queue was empty
     */
    data_read_ptr<const fep3::arya::IDataSample> popSampleOldest()
    {
        return std::get<0>(_sample_queue.popFront());
    }

    /**
     * @brief reads the latest sample available
     *
     * @return data_read_ptr<const IDataSample>
     * @retval valid pointer the sample read
     * @retval invalid pointer the queue was empty
     */
    data_read_ptr<const fep3::arya::IDataSample> readSampleLatest() const
    {
        return std::get<0>(_sample_queue.readBack());
    }

    /**
     * @brief reads the oldest sample available
     *
     * @return data_read_ptr<const IDataSample>
     * @retval valid pointer the sample read
     * @retval invalid pointer the queue was empty
     */
    data_read_ptr<const fep3::arya::IDataSample> readSampleOldest()
    {
        return std::get<0>(_sample_queue.readFront());
    }

    /**
     * @brief reads the latest sample which is below an upper bound timestamp
     *
     * @param upper_bound time looking for
     * @return data_read_ptr<const IDataSample>
     * @retval valid pointer the sample read
     * @retval invalid pointer the queue was empty or did not contain a sample with timestamp below
     * upper_bound
     */
    data_read_ptr<const fep3::arya::IDataSample> readSampleBefore(
        fep3::arya::Timestamp upper_bound) const
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);

        if (0 == _sample_queue.size()) {
            return {};
        }

        data_read_ptr<const fep3::arya::IDataSample> result;
        _sample_queue.reverseIteration(
            [&result, upper_bound](const data_read_ptr<const fep3::arya::IDataSample>& sample,
                                   const data_read_ptr<const fep3::arya::IStreamType>&,
                                   fep3::arya::Timestamp timestamp) {
                if (timestamp < upper_bound) {
                    result = sample;
                    return false;
                }
                return true;
            });

        return result;
    }

    /**
     * @brief pops latest sample older than timestamp
     * and purges all other samples which are older than the given timestamp from queue
     *
     * @param[in] timestamp threshold for samples to remove
     * @return data_read_ptr<const IDataSample>
     * @retval valid pointer the sample popped
     * @retval invalid pointer the queue was empty or does not contain a sample older than the given
     * timestamp
     */
    data_read_ptr<const fep3::arya::IDataSample> purgeAndPopSampleBefore(
        const fep3::arya::Timestamp timestamp)
    {
        data_read_ptr<const fep3::arya::IDataSample> sample, oldest = readSampleOldest();
        // check for newer samples (which are still older than the timestamp)
        while (oldest && (oldest->getTime() < timestamp)) {
            sample = popSampleOldest();
            oldest = readSampleOldest();
        }
        return sample;
    }

    /**
     * @brief reads the current type
     *
     * @return data_read_ptr<const IStreamType>
     * @retval valid pointer the streamtype read
     */
    data_read_ptr<const fep3::arya::IStreamType> readType() const
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);

        return _init_type;
    }

    /**
     * @brief resizes the data reader backlog.
     * This will reset the data sample queue and resize it to the given queue_size.
     * If sample queue_size <= 0, the sample queue is resized to capacity 1
     *
     * @param sample_queue_size resized capacity of the data sample queue
     * @return size_t the new capacity
     */
    size_t setCapacity(size_t sample_queue_size)
    {
        if (sample_queue_size <= 0) {
            sample_queue_size = 1;
        }

        return _sample_queue.setCapacity(sample_queue_size);
    }

    /**
     * @brief reads the type until upper_bound is reached
     *
     * upper_bound time looking for
     * @return data_read_ptr<const IStreamType>
     * @retval valid pointer the streamtype read
     */
    data_read_ptr<const fep3::arya::IStreamType> readTypeBefore(Timestamp /*upper_bound*/) const
    {
        // TODO: create a ITEM Queue to receive the right type for the right sample
        std::lock_guard<std::mutex> lock_guard(_mutex);
        return _init_type;
    }

private:
    ///@cond no_documentation
    base::arya::detail::DataItemQueue _sample_queue;
    data_read_ptr<const fep3::arya::IStreamType> _init_type;
    mutable std::mutex _mutex;
    ///@endcond no_documentation
};
} // namespace arya
using arya::DataReaderBacklog;
} // namespace core
} // namespace fep3
