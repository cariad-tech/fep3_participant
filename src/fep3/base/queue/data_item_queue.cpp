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


#include <fep3/base/queue/data_item_queue.h>
#include <boost/circular_buffer.hpp>
#include <boost/foreach.hpp>

namespace fep3
{
namespace base
{
namespace arya
{
namespace detail
{

class DataItemQueue::Implementation
{
public:
    Implementation(size_t capacity):
        _items(capacity <= 0 ? 1 : capacity)
    {

    }

    boost::circular_buffer<DataItem> _items;
    mutable std::recursive_mutex _recursive_mutex;
};

DataItemQueue::DataItemQueue(size_t capacity)
    : base::arya::detail::DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>()
    , _impl(std::make_unique<Implementation>(capacity))
{
}

DataItemQueue::~DataItemQueue()
{

}

///@copydoc fep3::base::arya::detail::DataItemQueueBase::pushSample
void DataItemQueue::pushSample(const data_read_ptr<SAMPLE_TYPE>& sample, fep3::arya::Timestamp time_of_receiving)
{
    std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);
    _impl->_items.push_back({ sample, time_of_receiving });
}

///@copydoc fep3::base::arya::detail::DataItemQueueBase::pushType
void DataItemQueue::pushType(const data_read_ptr<STREAM_TYPE>& type, fep3::arya::Timestamp time_of_receiving)
{
    std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);
    _impl->_items.push_back({ type, time_of_receiving });
}

///@copydoc fep3::base::arya::detail::DataItemQueueBase::nextTime
fep3::arya::Optional<fep3::arya::Timestamp> DataItemQueue::nextTime()
{
    std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);
    for (auto it = _impl->_items.begin(); it != _impl->_items.end(); ++it)
    {
        DataItem& ref = *it;
        if (DataItem::Type::sample == ref.getItemType())
        {
            return ref.getSample()->getTime();
        }
    }
    return {};
}

///@copydoc fep3::base::arya::detail::DataItemQueueBase::pop
bool DataItemQueue::pop()
{
    std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);

    if(_impl->_items.size() > 0)
    {
        _impl->_items.pop_back();
        return true;
    }
    return false;
}

///@copydoc fep3::base::arya::detail::DataItemQueueBase::popFront
bool DataItemQueue::popFront(typename DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>::IDataItemReceiver& receiver)
{
    std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);

    if (_impl->_items.size() > 0)
    {

        DataItem& data_item = _impl->_items.front();
        if (DataItem::Type::sample == data_item.getItemType())
        {
            receiver.onReceive(data_item.getSample());
            data_item.resetSample();
        }
        else if (DataItem::Type::type == data_item.getItemType())
        {
            receiver.onReceive(data_item.getStreamType());
            data_item.resetStreamType();
        }

        _impl->_items.pop_front();

        return true;
    }

    return false;
}

///@copydoc fep3::base::arya::detail::DataItemQueueBase::popFront()
std::tuple<data_read_ptr<DataItemQueue::SAMPLE_TYPE>, data_read_ptr<DataItemQueue::STREAM_TYPE>> DataItemQueue::popFront()
{
    data_read_ptr<SAMPLE_TYPE> sample = nullptr;
    data_read_ptr<STREAM_TYPE> stream_type = nullptr;

    {
        std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);

        if (_impl->_items.size() > 0)
        {
            DataItem& data_item = _impl->_items.front();
            if (DataItem::Type::sample == data_item.getItemType())
            {
                sample = std::move(data_item.getSample());
                data_item.resetSample();
            }
            else if (DataItem::Type::type == data_item.getItemType())
            {
                stream_type = std::move(data_item.getStreamType());
                data_item.resetStreamType();
            }

            _impl->_items.pop_front();
        }
    }

    return std::make_tuple(std::move(sample), std::move(stream_type));
}

///@copydoc fep3::base::arya::detail::DataItemQueueBase::popBack
bool DataItemQueue::popBack(typename DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>::IDataItemReceiver& receiver)
{
    std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);

    if (_impl->_items.size() > 0)
    {
        DataItem& data_item = _impl->_items.back();
        if (DataItem::Type::sample == data_item.getItemType())
        {
            receiver.onReceive(data_item.getSample());
            data_item.resetSample();
        }
        else if (DataItem::Type::type == data_item.getItemType())
        {
            receiver.onReceive(data_item.getStreamType());
            data_item.resetStreamType();
        }

        _impl->_items.pop_back();
        return true;
    }

    return false;
}

///@copydoc fep3::base::arya::detail::DataItemQueueBase::popBack()
std::tuple<data_read_ptr<DataItemQueue::SAMPLE_TYPE>, data_read_ptr<DataItemQueue::STREAM_TYPE>> DataItemQueue::popBack()
{
    data_read_ptr<SAMPLE_TYPE> sample = nullptr;
    data_read_ptr<STREAM_TYPE> stream_type = nullptr;

    {
        std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);

        if (_impl->_items.size() > 0)
        {
            DataItem& data_item = _impl->_items.back();
            if (DataItem::Type::sample == data_item.getItemType())
            {
                sample = std::move(data_item.getSample());
                data_item.resetSample();
            }
            else if (DataItem::Type::type == data_item.getItemType())
            {
                stream_type = std::move(data_item.getStreamType());
                data_item.resetStreamType();
            }
            _impl->_items.pop_back();
        }
    }

    return std::make_tuple(std::move(sample), std::move(stream_type));
}

///@copydoc fep3::base::arya::detail::DataItemQueueBase::read
std::tuple<data_read_ptr<DataItemQueue::SAMPLE_TYPE>, data_read_ptr<DataItemQueue::STREAM_TYPE>> DataItemQueue::read(size_t index) const
{
    data_read_ptr<SAMPLE_TYPE> sample = nullptr;
    data_read_ptr<STREAM_TYPE> stream_type = nullptr;

    std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);

    if (0 < size() && index < size())
    {
        const DataItem& data_item = _impl->_items[index];
        if (DataItem::Type::sample == data_item.getItemType())
        {
            sample = std::move(data_item.getSample());
        }
        else if (DataItem::Type::type == data_item.getItemType())
        {
            stream_type = std::move(data_item.getStreamType());
        }
    }

    return std::make_tuple(std::move(sample), std::move(stream_type));
}

///@copydoc fep3::base::arya::detail::DataItemQueueBase::readFront
std::tuple<data_read_ptr<DataItemQueue::SAMPLE_TYPE>, data_read_ptr<DataItemQueue::STREAM_TYPE>> DataItemQueue::readFront()
{
    data_read_ptr<SAMPLE_TYPE> sample = nullptr;
    data_read_ptr<STREAM_TYPE> stream_type = nullptr;

    {
        std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);

        if (_impl->_items.size() > 0)
        {
            const DataItem& data_item = _impl->_items.front();
            if (DataItem::Type::sample == data_item.getItemType())
            {
                sample = std::move(data_item.getSample());
            }
            else if (DataItem::Type::type == data_item.getItemType())
            {
                stream_type = std::move(data_item.getStreamType());
            }
        }
    }

    return std::make_tuple(std::move(sample), std::move(stream_type));
}

///@copydoc fep3::base::arya::detail::DataItemQueueBase::readBack
std::tuple<data_read_ptr<DataItemQueue::SAMPLE_TYPE>, data_read_ptr<DataItemQueue::STREAM_TYPE>> DataItemQueue::readBack() const
{
    data_read_ptr<SAMPLE_TYPE> sample = nullptr;
    data_read_ptr<STREAM_TYPE> stream_type = nullptr;

    {
        std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);

        if (_impl->_items.size() > 0)
        {
            const DataItem& data_item = _impl->_items.back();
            if (DataItem::Type::sample == data_item.getItemType())
            {
                sample = std::move(data_item.getSample());
            }
            else if (DataItem::Type::type == data_item.getItemType())
            {
                stream_type = std::move(data_item.getStreamType());
            }
        }
    }

    return std::make_tuple(std::move(sample), std::move(stream_type));
}

///@copydoc fep3::base::arya::detail::DataItemQueueBase::capacity
size_t DataItemQueue::capacity() const
{
    std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);

    return _impl->_items.capacity();
}

///@copydoc fep3::base::arya::detail::DataItemQueueBase::size
size_t DataItemQueue::size() const
{
    std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);

    return _impl->_items.size();
}

///@copydoc fep3::base::arya::detail::DataItemQueueBase::clear
void DataItemQueue::clear()
{
    std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);

    _impl->_items.clear();
}

///@copydoc fep3::base::arya::detail::DataItemQueueBase::getQueueType
DataItemQueue::QueueType DataItemQueue::getQueueType() const
{
    return QueueType::fixed;
}

/**
    * @brief resizes the queue to an empty queue with capacity of queue_size
    * if queue_size <= 0, the queue is resized to capacity 1
    *
    * @param queue_size resized capacity of the queue
    * @return size_t the new capacity
    */
size_t DataItemQueue::setCapacity(size_t queue_size)
{
    std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);
    _impl->_items.set_capacity(queue_size);

    clear();

    return queue_size;
}

void DataItemQueue::reverseIteration(const std::function<bool(const data_read_ptr<SAMPLE_TYPE> & sample, const data_read_ptr<STREAM_TYPE> & stream_type, fep3::arya::Timestamp)> & callback) const
{
    std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);
    BOOST_REVERSE_FOREACH(const DataItem& item, _impl->_items)
    {
        if (!callback(item.getSample(), item.getStreamType(), item.getTime())) return;
    };
}

void DataItemQueue::iteration(const std::function<bool(const data_read_ptr<SAMPLE_TYPE> & sample, const data_read_ptr<STREAM_TYPE> & stream_type, fep3::arya::Timestamp)> & callback) const
{
    std::lock_guard<std::recursive_mutex> lock_guard(_impl->_recursive_mutex);
    BOOST_FOREACH(const DataItem& item, _impl->_items)
    {
        if (!callback(item.getSample(), item.getStreamType(), item.getTime())) return;
    };
}

}
}
}
}

