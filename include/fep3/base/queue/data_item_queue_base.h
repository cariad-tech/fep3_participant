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

#include "fep3/fep3_optional.h"
#include "fep3/base/sample/data_sample_intf.h"
#include "fep3/base/stream_type/stream_type_intf.h"

namespace fep3
{
namespace base
{
namespace arya
{
namespace detail
{
/**
 * @brief Data Item queue base
 * Base class for data item queue implementations
 *
 * @tparam SAMPLE_TYPE IDataRegistry::IDataSample class for samples
 * @tparam STREAM_TYPE IStreamType class for types
 */
template<class SAMPLE_TYPE = const fep3::arya::IDataSample, class STREAM_TYPE = const fep3::arya::IStreamType>
class DataItemQueueBase
{
protected:
    /**
     * @brief internal queue type
     *
     */
    enum QueueType
    {
        /// the queue capacity is fix
        fixed,
        /// the queue capacity will dynamically grow
        dynamic
    };

    /**
     * @brief internal data item
     *
     */
    class DataItem
    {
    public:
        /**
         * content type of the data item
         */
        enum Type
        {
            ///is a sample
            sample,
            ///is a stream type
            type
        };

    public:
        /**
         * CTOR
         */
        DataItem()
            : _item_type(),
            _time()
        {
        }

        /**
         * CTOR for a sample data item
         *
         * @param[in] sample the sample to be stored in the data item
         * @param[in] time the timestamp of the data item
         */
        DataItem(const data_read_ptr<SAMPLE_TYPE>& sample, fep3::arya::Timestamp time)
            : _item_type(Type::sample),
            _time(time),
            _sample(sample)
        {
        }

        /**
        * CTOR for a stream type data item
        *
        * @param[in] stream_type the stream_type to be stored in the data item
        * @param[in] time the timestamp of the data item
        */
        DataItem(const data_read_ptr<STREAM_TYPE>& stream_type, fep3::arya::Timestamp time)
            : _item_type(Type::type),
            _time(time),
            _stream_type(stream_type)
        {
        }

    public:
        /**
        * @brief Setter for a new data sample
        * Resets the stream type member of the data item and handles the remaining members accordingly
        *
        * @param[in] sample the sample to be stored in the data item
        * @param[in] time the timestamp of the data item
        */
        void set(const data_read_ptr<SAMPLE_TYPE>& sample, fep3::arya::Timestamp time)
        {
            _sample = sample;
            _time = time;
            _stream_type.reset();
            _item_type = Type::sample;
        }

        /**
         * @brief Setter for a new stream_type
         * Resets the sample member of the data item and handles the remaining members accordingly
         *
         * @param[in] stream_type the stream_type to be stored in the data item
         * @param[in] time the timestamp of the data item
         */
        void set(const data_read_ptr<STREAM_TYPE>& stream_type, fep3::arya::Timestamp time)
        {
            _stream_type = stream_type;
            _time = time;
            _sample.reset();
            _item_type = Type::type;
        }

        /**
         * gets the type information of the current item.
         * @return the type (sample or stream type)
         */
        Type getItemType() const
        {
            return _item_type;
        }

        /**
         * Gets the time
         * @return the time
         */
        fep3::arya::Timestamp getTime() const
        {
            return _time;
        }

        /**
         * gets the sample (if set).
         * @return the sample or nullptr
         */
        data_read_ptr<SAMPLE_TYPE> getSample() const
        {
            return _sample;
        }

        /**
         * gets the stream type (if set).
         * @return the type or nullptr
         */
        data_read_ptr<STREAM_TYPE> getStreamType() const
        {
            return _stream_type;
        }

        /**
         * resets the sample
         */
        void resetSample()
        {
            _sample.reset();
        }

        /**
         * resets the stream type
         */
        void resetStreamType()
        {
            _stream_type.reset();
        }

    private:
        ///@cond no_documentation
        Type _item_type;
        fep3::arya::Timestamp  _time;
        data_read_ptr<SAMPLE_TYPE> _sample;
        data_read_ptr<STREAM_TYPE> _stream_type;
        ///@endcond no_documentation
    };

public:
    /**
     * @brief Item receiver for pop call.
     *
     */
    class IDataItemReceiver
    {
    protected:
        /// DTOR
        ~IDataItemReceiver() = default;

    public:
        /**
         * @brief callback to receive a data read pointer to a sample
         *
         * @param[in] sample the sample currently retrieved by the pop call
         */
        virtual void onReceive(const data_read_ptr<SAMPLE_TYPE>& sample) = 0;
        /**
         * @brief callback to receive a data read pointer to a stream_type
         *
         * @param[in] stream_type the stream type currently retrieved by the pop call
         */
        virtual void onReceive(const data_read_ptr<STREAM_TYPE>& stream_type) = 0;
    };

public:
    /**
    * @brief CTOR
    */
    DataItemQueueBase() = default;

    /**
     * @brief DTOR
     *
     */
    virtual ~DataItemQueueBase() = default;

    /**
    * @brief pushes a sample data read pointer to the queue
    *
    * @param sample the sample data read pointer to push
    * @param time_of_receiving the timestamp at which the sample was received
    * @remark this is thread safe against push, read and pop calls
    */
    virtual void pushSample(const data_read_ptr<SAMPLE_TYPE>& sample, fep3::arya::Timestamp time_of_receiving) = 0;

    /**
    * @brief pushes a stream type data read pointer to the queue
    *
    * @param type the type data read pointer to push
    * @param time_of_receiving the timestamp at which the stream type was received
    * @remark this is thread safe against push, read and pop calls
    */
    virtual void pushType(const data_read_ptr<STREAM_TYPE>& type, fep3::arya::Timestamp time_of_receiving) = 0;

    /**
     * @brief returns the timestamp of the next item available (the item at the front of the queue/the oldest item available)
     *
     * @return Optional containing the Timestamp of the queue front item
     * @return Empty Optional if the queue is empty
     * @remark this is thread safe against push, read and pop calls
     */
    virtual fep3::arya::Optional<fep3::arya::Timestamp> nextTime() = 0;

    /**
    * @brief pops an item from the front of the queue (the oldest item available)
    *
    * @return true if item is popped
    * @return false if the queue is empty
    * @remark this is thread safe against push, read and pop calls
    */
    virtual bool pop() = 0;

    /**
    * @brief pops the item from the front of the queue (the oldest item available) after putting the item to the given \p receiver
    *
    * @param receiver receiver reference where to call back and put the item before the item is popped.
    * @return true if item is popped
    * @return false if the queue is empty
    * @remark this is thread safe against push, read and pop calls
    */
    virtual bool popFront(IDataItemReceiver& receiver) = 0;

    /**
    * @brief pops the item at the front of the queue (the oldest item available)
    *
    * @return {data_read_ptr<SAMPLE_TYPE>, nullptr} if sample item is popped
    * @return {nullptr, data_read_ptr<STREAM_TYPE>} if type item is popped
    * @return {nullptr, nullptr} if queue is empty
    * @remark this is thread safe against push, read and pop calls
    */
    virtual std::tuple<data_read_ptr<SAMPLE_TYPE>, data_read_ptr<STREAM_TYPE>> popFront() = 0;

    /**
    * @brief pops the item from the back of the queue (the latest item available) after putting the item to the given \p receiver
    *
    * @param receiver receiver reference where to call back and put the item before the item is popped.
    * @return true if item is popped
    * @return false if the queue is empty
    * @remark this is thread safe against push, read and pop calls
    */
    virtual bool popBack(IDataItemReceiver& receiver) = 0;

    /**
    * @brief pops the item at the back of the queue (the latest item available)
    *
    * @return {data_read_ptr<SAMPLE_TYPE>, nullptr} if sample item is popped
    * @return {nullptr, data_read_ptr<STREAM_TYPE>} if type item is popped
    * @return {nullptr, nullptr} if queue is empty
    * @remark this is thread safe against push, read and pop calls
    */
    virtual std::tuple<data_read_ptr<SAMPLE_TYPE>, data_read_ptr<STREAM_TYPE>> popBack() = 0;

    /**
    * @brief reads an item at a given index without popping it
    *
    * @param index index of the item to be read.
    * @return {data_read_ptr<SAMPLE_TYPE>, nullptr} if sample item is read
    * @return {nullptr, data_read_ptr<STREAM_TYPE>} if type item is read
    * @return {nullptr, nullptr} if queue is empty
    * @remark this is thread safe against push, read and pop calls
    */
    virtual std::tuple<data_read_ptr<SAMPLE_TYPE>, data_read_ptr<STREAM_TYPE>> read(size_t index) const = 0;

    /**
    * @brief reads the item at the front of the queue (the oldest item available) without popping it
    *
    * @return {data_read_ptr<SAMPLE_TYPE>, nullptr} if sample item is read
    * @return {nullptr, data_read_ptr<STREAM_TYPE>} if type item is read
    * @return {nullptr, nullptr} if queue is empty
    * @remark this is thread safe against push, read and pop calls
    */
    virtual std::tuple<data_read_ptr<SAMPLE_TYPE>, data_read_ptr<STREAM_TYPE>> readFront() = 0;

    /**
    * @brief reads the item at the back of the queue (the latest item available) without popping it
    *
    * @return {data_read_ptr<SAMPLE_TYPE>, nullptr} if sample item is read
    * @return {nullptr, data_read_ptr<STREAM_TYPE>} if type item is read
    * @return {nullptr, nullptr} if queue is empty
    * @remark this is thread safe against push, read and pop calls
    */
    virtual std::tuple<data_read_ptr<SAMPLE_TYPE>, data_read_ptr<STREAM_TYPE>> readBack() const = 0;

    /**
     * @brief returns the maximum capacity of the queue
     *
     * @return maximum capacity of the queue
     */
    virtual size_t capacity() const = 0;

    /**
     * @brief returns the current size of the queue
     *
     * @return current size of the queue
     */
    virtual size_t size() const = 0;

    /**
     * @brief removes all items of the queue
     */
    virtual void clear() = 0;

    /**
     * @brief returns the type of the queue
     *
     * @return QueueType the type of the queue
     * Either QueueType::fixed or QueueType::dynamic
     */
    virtual QueueType getQueueType() const = 0;
};
}
}
}
}
