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

#include <fep3/base/queue/data_item_queue.h>
#include <fep3/components/data_registry/data_registry_intf.h>
#include <fep3/components/logging/easy_logger.h>

#include <mutex>
#include <sstream>

namespace {
// Format timestamp vector in a human readable way for logging purposes.
std::stringstream timestampVecToString(const std::vector<fep3::Timestamp>& timestamps)
{
    std::stringstream ss;
    for (auto it = timestamps.begin(); it != timestamps.end(); it++) {
        if (it != timestamps.begin()) {
            ss << ", ";
        }
        ss << it->count();
    }
    return ss;
}
} // namespace

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
     * @param purged_sample_log_capacity capacity of the buffer storing information regarding purged
     * samples.
     */
    DataReaderBacklog(size_t sample_queue_capacity,
                      const fep3::arya::IStreamType& init_type,
                      size_t purged_sample_log_capacity)
        : _sample_queue(sample_queue_capacity),
          _init_type(std::make_shared<fep3::base::arya::StreamType>(init_type)),
          _purged_sample_log_capacity(purged_sample_log_capacity)
    {
        if (samplePurgeLogEnabled()) {
            _purged_sample_timestamps.reserve(_purged_sample_log_capacity);
        }
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
     * @brief Receives a data sample item.
     * Optionally stores information of samples purged automatically @see
     * _purged_sample_log_capacity. Samples are purged automatically if a sample is pushed to the
     * queue while the queue is full.
     *
     * @param[in] sample The received data sample
     */
    void operator()(const data_read_ptr<const fep3::arya::IDataSample>& sample) override
    {
        if (samplePurgeLogEnabled() && sampleQueueFull() && !purgeLogQueueFull()) {
            auto oldest_sample_timestamp = _sample_queue.nextTime();
            if (oldest_sample_timestamp.has_value()) {
                _purged_sample_timestamps.push_back(oldest_sample_timestamp.value());
            }
        }

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
     * and purges all other samples which are older than the given timestamp from queue.
     * Optionally stores information of samples purged @see _purged_sample_log_capacity.
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
            if (samplePurgeLogEnabled() && !purgeLogQueueFull() && oldest &&
                (oldest->getTime() < timestamp)) {
                _purged_sample_timestamps.push_back(sample->getTime());
            }
        }
        return sample;
    }

    /**
     * @brief Clears the buffer and optionally stores information regarding purged samples @see
     * _purged_sample_log_capacity.
     *
     */
    void clear()
    {
        if (samplePurgeLogEnabled()) {
            _sample_queue.iteration(
                [&samples = _purged_sample_timestamps, capacity = _purged_sample_log_capacity](
                    const data_read_ptr<const fep3::arya::IDataSample>&,
                    const data_read_ptr<const fep3::arya::IStreamType>&,
                    fep3::arya::Timestamp timestamp) {
                    if (samples.size() >= capacity) {
                        return false;
                    }

                    samples.push_back(timestamp);
                    return true;
                });
        }

        _sample_queue.clear();
    }

    /**
     * @brief Logs the amount of purged samples and the corresponding timestamps.
     * The capacity of information stored regarding purged samples can be configured
     * @see _purged_sample_log_capacity. If
     *
     * @param[in] signal_name The name of the signal this data reader backlog belongs to
     * @param[in] logger The logger used to log the information regarding samples purged
     */
    void logPurgedSamples(const std::string& signal_name, const fep3::arya::ILogger* logger) const
    {
        if (!samplePurgeLogEnabled()) {
            return;
        }

        if (_purged_sample_timestamps.empty()) {
            FEP3_LOGGER_LOG_DEBUG(
                logger,
                a_util::strings::format("No samples have been purged from signal '%s'.",
                                        signal_name.c_str()));
        }
        else {
            FEP3_LOGGER_LOG_WARNING(
                logger,
                a_util::strings::format(
                    "'%d' samples having the following timestamps have been purged from signal "
                    "'%s' and have never "
                    "been accessible from within the FEP Participant: '%s'",
                    _purged_sample_timestamps.size(),
                    signal_name.c_str(),
                    timestampVecToString(_purged_sample_timestamps).str().c_str()));
            if (purgeLogQueueFull()) {
                FEP3_LOGGER_LOG_WARNING(
                    logger,
                    a_util::strings::format(
                        "The buffer containing information regarding purged samples has reached "
                        "its limit. "
                        "The number of purged samples for signal '%s' might be higher than '%d'. "
                        "Increase the buffer capacity to allow storing more "
                        "information regarding purged samples.",
                        signal_name.c_str(),
                        _purged_sample_timestamps.size()));
            }
        }
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
    /**
     * @brief returns whether logging of purged samples is enabled.
     *
     * @return bool is purged samples logging enabled
     */
    bool samplePurgeLogEnabled() const
    {
        return _purged_sample_log_capacity != 0;
    }

    /**
     * @brief returns whether the data item queue is full.
     *
     * @return bool is the data item queue full
     */
    bool sampleQueueFull() const
    {
        return _sample_queue.size() >= _sample_queue.capacity();
    }

    /**
     * @brief returns whether the purged sample logging queue is full.
     *
     * @return bool is the purged sample logging queue full
     */
    bool purgeLogQueueFull() const
    {
        return _purged_sample_timestamps.size() >= _purged_sample_log_capacity;
    }

private:
    ///@cond no_documentation
    base::arya::detail::DataItemQueue _sample_queue;
    data_read_ptr<const fep3::arya::IStreamType> _init_type;
    mutable std::mutex _mutex;
    /** Capacity of the buffer storing information regarding purged samples @see
     * _purged_sample_timestamps.
     * @see clear, @see logPurgedSamples and @see operator() will store information in case
     * of samples being purged. Once the limit is reached, no further information regarding purged
     * samples will be stored. Value of 0 disables storage of information regarding
     * purged samples.
     */
    size_t _purged_sample_log_capacity;
    /// Buffer storing the timestamps of purged samples. Capacity is configurable @see
    /// _purged_sample_log_capacity.
    std::vector<fep3::Timestamp> _purged_sample_timestamps;
    ///@endcond no_documentation
};
} // namespace arya
using arya::DataReaderBacklog;
} // namespace core
} // namespace fep3
