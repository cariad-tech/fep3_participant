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

namespace fep3 {
namespace native {

struct WrappedDataItemReceiver : public base::arya::detail::DataItemQueueBase<>::IDataItemReceiver {
    fep3::arya::IDataRegistry::IDataReceiver& _receiver;
    WrappedDataItemReceiver(fep3::arya::IDataRegistry::IDataReceiver& receiver)
        : _receiver(receiver)
    {
    }
    void onReceive(const data_read_ptr<const fep3::arya::IDataSample>& sample) override
    {
        _receiver(sample);
    }
    void onReceive(const data_read_ptr<const fep3::arya::IStreamType>& stream_type) override
    {
        _receiver(stream_type);
    }
};

/**
 * @brief A data reader queue implementation
 */
class DataReaderQueue : public fep3::arya::IDataRegistry::IDataReceiver,
                        public fep3::arya::IDataRegistry::IDataReader {
public:
    /**
     * @brief Construct a new Data Reader Queue object
     *
     * @param[in] capa the initial capacity (if 0 it will be dynamic size!)
     */
    explicit DataReaderQueue(size_t capa) : _queue(capa)
    {
    }

    /**
     * @brief DTOR
     */
    ~DataReaderQueue() = default;

    /**
     * @brief retrieves the current size of the queue.
     *
     * @return size_t the size in item count.
     */
    size_t size() const override final
    {
        return _queue.size();
    }

    /**
     * @brief retrieves the capacity if the queue
     *
     * @return size_t the capacity
     */
    size_t capacity() const override final
    {
        return _queue.capacity();
    }

    /**
     * @brief Receives a stream type item
     *
     * @param[in] type The received stream type
     */
    void operator()(const data_read_ptr<const fep3::arya::IStreamType>& type) override final
    {
        _queue.pushType(type, std::chrono::milliseconds(0));
    }

    /**
     * @brief Receives a data sample item
     *
     * @param[in] sample The received data sample
     */
    void operator()(const data_read_ptr<const fep3::arya::IDataSample>& sample) override final
    {
        _queue.pushSample(sample, sample->getTime());
    }

    /**
     * @brief Get the Front Time
     *
     * @return fep3::Optional<Timestamp>
     * @retval valid time queue is not empty
     * @retval invalid time queue is empty
     */
    fep3::arya::Optional<fep3::arya::Timestamp> getFrontTime() const override final
    {
        return _queue.nextTime();
    }

    /**
     * @brief pops the next item (type or sample)
     *
     * @param[in] receiver the receiver to "send" the items to
     * @return fep3::Result
     * @retval ERR_NOERROR received successfully
     * @retval ERR_EMPTY queue is empty
     */
    ::fep3::Result pop(fep3::arya::IDataRegistry::IDataReceiver& receiver) override final
    {
        WrappedDataItemReceiver wrap(receiver);
        return _queue.popFront(wrap) ? fep3::Result() : fep3::ERR_EMPTY;
    }

    /**
     * @brief empty the queue
     */
    void clear()
    {
        _queue.clear();
    }

private:
    mutable base::arya::detail::DataItemQueue _queue;
};
} // namespace native
} // namespace fep3
