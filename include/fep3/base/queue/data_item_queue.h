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

#include <atomic>
#include <mutex>
#include <vector>
#include "data_item_queue_base.h"
#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep3/base/queue/data_item_queue_base.h"

namespace fep3
{
namespace base
{
namespace arya
{
/**
 * @brief namespace API components where the API compatibility is NOT guaranteed.
 *
 */
namespace detail
{
/**
 * @brief Data Item queue (at the moment a locked queue)
 * This implementation will provide a queue to read samples and types from a given index and
 * read/pop samples and types from the front and back of the queue.
 * The capacity of the DataItemQueue is fixed. If samples are pushed into the queue while the queue's capacity is reached
 * old items are dropped.
 *
 * |Nr. sample|Queue|
 * |1.        |front|
 * |2.        |2.   |
 * |..        |..   |
 * |n.        |back |
 *
 * Item nr. 1 is the first item received and the front of the queue
 * while item nr. n is the latest item received and the rear of the queue.
 *
 */

class FEP3_PARTICIPANT_EXPORT DataItemQueue : public arya::detail::DataItemQueueBase<const fep3::arya::IDataSample, const fep3::arya::IStreamType>
{
private:
    using SAMPLE_TYPE = const fep3::arya::IDataSample;
    using STREAM_TYPE = const fep3::arya::IStreamType;
    using typename arya::detail::DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>::DataItem;
    using typename arya::detail::DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>::QueueType;

public:
    /**
    * @brief CTOR
    *
    * @param[in] capacity capacity by item count of the queue (there are sample + stream type covered)
    */
    DataItemQueue(size_t capacity);

    /**
    * @brief DTOR
    *
    */
    virtual ~DataItemQueue();

    ///@copydoc fep3::base::arya::detail::DataItemQueueBase::pushSample
    void pushSample(const data_read_ptr<SAMPLE_TYPE>& sample, fep3::arya::Timestamp time_of_receiving) override;

    ///@copydoc fep3::base::arya::detail::DataItemQueueBase::pushType
    void pushType(const data_read_ptr<STREAM_TYPE>& type, fep3::arya::Timestamp time_of_receiving) override;

    ///@copydoc fep3::base::arya::detail::DataItemQueueBase::nextTime
    fep3::arya::Optional<fep3::arya::Timestamp> nextTime() override;

    ///@copydoc fep3::base::arya::detail::DataItemQueueBase::pop
    bool pop() override;

    ///@copydoc fep3::base::arya::detail::DataItemQueueBase::popFront
    bool popFront(typename DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>::IDataItemReceiver& receiver) override;

    ///@copydoc fep3::base::arya::detail::DataItemQueueBase::popFront()
    std::tuple<data_read_ptr<SAMPLE_TYPE>, data_read_ptr<STREAM_TYPE>> popFront() override;

    ///@copydoc fep3::base::arya::detail::DataItemQueueBase::popBack
    bool popBack(typename DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>::IDataItemReceiver& receiver) override;

    ///@copydoc fep3::base::arya::detail::DataItemQueueBase::popBack()
    std::tuple<data_read_ptr<SAMPLE_TYPE>, data_read_ptr<STREAM_TYPE>> popBack() override;

    ///@copydoc fep3::base::arya::detail::DataItemQueueBase::read
    std::tuple<data_read_ptr<SAMPLE_TYPE>, data_read_ptr<STREAM_TYPE>> read(size_t index) const override;

    ///@copydoc fep3::base::arya::detail::DataItemQueueBase::readFront
    std::tuple<data_read_ptr<SAMPLE_TYPE>, data_read_ptr<STREAM_TYPE>> readFront() override;

    ///@copydoc fep3::base::arya::detail::DataItemQueueBase::readBack
    std::tuple<data_read_ptr<SAMPLE_TYPE>, data_read_ptr<STREAM_TYPE>> readBack() const override;

    ///@copydoc fep3::base::arya::detail::DataItemQueueBase::capacity
    size_t capacity() const override;

    ///@copydoc fep3::base::arya::detail::DataItemQueueBase::size
    size_t size() const override;

    ///@copydoc fep3::base::arya::detail::DataItemQueueBase::clear
    void clear() override;

    ///@copydoc fep3::base::arya::detail::DataItemQueueBase::getQueueType
    QueueType getQueueType() const override;

    /**
     * @brief resizes the queue to an empty queue with capacity of queue_size
     * if queue_size <= 0, the queue is resized to capacity 1
     *
     * @param queue_size resized capacity of the queue
     * @return size_t the new capacity
     */
    size_t setCapacity(size_t queue_size);

    /**
     * @brief reverse iteration over all items stored in the queue
     *
     * @param callback callback function will be called for each item in the queue
     */
    void reverseIteration(const std::function<bool(const data_read_ptr<SAMPLE_TYPE> & sample, const data_read_ptr<STREAM_TYPE> & stream_type, fep3::arya::Timestamp)> & callback) const;

    /**
     * @brief iteration over all items stored in the queue
     *
     * @param callback callback function will be called for each item in the queue
     */
    void iteration(const std::function<bool(const data_read_ptr<SAMPLE_TYPE> & sample, const data_read_ptr<STREAM_TYPE> & stream_type, fep3::arya::Timestamp)> & callback) const;

private:
    class Implementation;
    std::unique_ptr<Implementation> _impl;
};

}
}
}
}
