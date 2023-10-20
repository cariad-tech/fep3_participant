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

#include <fep3/base/sample/data_sample.h>
#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/components/base/components_intf.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/data_registry/data_registry_intf.h>

namespace fep3 {
namespace core {
namespace arya {

/// Value for queue capacity definition if dynamic queue is chosen
constexpr size_t DATA_WRITER_QUEUE_SIZE_DYNAMIC = 0;
/// Value for queue capacity definition if static queue size is chosen
constexpr size_t DATA_WRITER_QUEUE_SIZE_DEFAULT = 1;

/**
 * @brief Data Writer helper class to write data to a fep3::IDataRegistry::IDataWriter
 * if registered to the fep3::IDataRegistry
 */
class DataWriter {
public:
    /**
     * @brief Construct a new Data Writer with an infinite capacity queue.
     * Pushing or popping items from the queue increases or decreases the queue's size.
     * This may lead to out-of-memory situations if big numbers of items are pushed into the queue.
     */
    DataWriter();

    /**
     * @brief Construct a new Data Writer with an infinite capacity queue.
     * Pushing or popping items from the queue increases or decreases the queue's size.
     * This may lead to out-of-memory situations if big numbers of items are pushed into the queue.
     *
     * @param[in] name name of the outgoing data
     * @param[in] stream_type type of the outgoing data
     */
    DataWriter(std::string name, const fep3::base::arya::StreamType& stream_type);

    /**
     * @brief Construct a new Data Writer with a fixed capacity queue.
     * If the queue's capacity is exceeded, e.g. the queue is full and more data items are pushed
     * into the queue, old, not yet used data items are dropped.
     *
     * @param[in] name name of the outgoing data
     * @param[in] stream_type type of the outgoing data
     * @param[in] queue_capacity the size of the underlying data sample queue
     */
    DataWriter(std::string name,
               const fep3::base::arya::StreamType& stream_type,
               size_t queue_capacity);

    /**
     * @brief Deleted Copy CTOR
     */
    DataWriter(const DataWriter&) = delete;

    /**
     * @brief Deleted Copy assignment operator
     *
     * @return DataWriter&
     */
    DataWriter& operator=(const DataWriter&) = delete;

    /**
     * @brief Deleted Move CTOR
     */
    DataWriter(DataWriter&&) = delete;

    /**
     * @brief Deleted Move assignment operator
     *
     * @return DataWriter&
     */
    DataWriter& operator=(DataWriter&&) = delete;

    /**
     * @brief Default DTOR
     */
    ~DataWriter() = default;

    /**
     * @brief registers and retrieves a data writer within the data registry
     *
     * @param[in] data_registry the data registry to register to
     * @return fep3::Result
     */
    fep3::Result addToDataRegistry(fep3::arya::IDataRegistry& data_registry);

    /**
     * @brief registers the clock_service_time_getter to get the time of the samples if not set
     *
     * @param[in] clock_service_time_getter the function to get the clock time from
     * @return fep3::Result
     */
    fep3::Result addClockTimeGetter(std::function<fep3::Timestamp()> clock_service_time_getter);

    /**
     * @deprecated
     * @brief remove the writer's reference to the data registry without removing the corresponding
     * writer from the data registry
     *
     * @return fep3::Result
     * @see fep3::DataWriter::removeFromDataRegistry(fep3::arya::IDataRegistry& data_registry)
     */
    [[deprecated("fep3::DataWriter::removeFromDataRegistry() is deprecated. Please use "
                 "fep3::DataWriter::removeFromDataRegistry(fep3::arya::IDataRegistry& "
                 "data_registry) instead.")]] fep3::Result
    removeFromDataRegistry();

    /**
     * @brief remove the writer's reference to the data registry and removes the corresponding
     * writer from the registry
     *
     * @param[in] data_registry the data registry to register to
     * @return fep3::Result
     */
    fep3::Result removeFromDataRegistry(fep3::arya::IDataRegistry& data_registry);

    /**
     * @brief removes the clock reference
     *
     * @return fep3::Result
     */
    fep3::Result removeClockTimeGetter();

    /**
     * @brief writes a data sample to the writer
     * If a clock time getter was set before via @ref addClockTimeGetter and the timestamp of @p
     * data_sample is zero, the timestamp is set to the current time of the clock.
     *
     * @param[in] data_sample the sample to write
     * @return fep3::Result
     */
    fep3::Result write(const fep3::arya::IDataSample& data_sample);

    /**
     * @brief writes a given raw memory to the writer, if an implementation of
     * fep3::base::RawMemoryClassType or fep3::base::RawMemoryStandardType exists
     *
     * @tparam T the type of the memory
     * @param[in] data_to_write the memory value
     * @return fep3::Result
     * @see IDataRegistry::IDataWriter::write
     */
    template <typename T>
    fep3::Result writeByType(T& data_to_write);

    /**
     * @brief writes a stream types to the data writer
     *
     * @param[in] stream_type the stream type to write
     * @return fep3::Result
     * @see IDataRegistry::IDataWriter::write
     */
    fep3::Result write(const fep3::arya::IStreamType& stream_type);

    /**
     * @brief writes raw memory + a time stamp to the connected IDataWriter as sample
     *
     * @param[in] time the time of the sample (usually simulation time)
     * @param[in] data pointer to the data to copy from
     * @param[in] data_size size in bytes to write
     * @return fep3::Result
     * @see IDataRegistry::IDataWriter::write
     */
    fep3::Result write(fep3::arya::Timestamp time, const void* data, size_t data_size);

    /**
     * @brief will flush the writers queue
     *        usually this is called while the executeDataOut call of the scheduler!
     *
     *
     *
     * @param[in] tmtime the current simtime of the flush call.
     * @return fep3::Result
     */
    virtual fep3::Result flushNow(fep3::arya::Timestamp tmtime);

    /**
     * @brief return the size of the writer queue
     * @return size_t the size of the writer queue
     */
    size_t getQueueSize()
    {
        return _queue_size;
    }

    /**
     * @brief get name of the reader
     * @return the name
     */
    virtual std::string getName() const;

private:
    /// the name of outgoing data
    std::string _name;
    /// the type of outgoing data
    fep3::base::arya::StreamType _stream_type;
    /// the writer if registered to the data registry
    std::unique_ptr<fep3::arya::IDataRegistry::IDataWriter> _connected_writer;
    size_t _queue_size;

    std::function<fep3::Timestamp()> _clock_service_time_getter;

    uint32_t _counter = 0;
};

/**
 * @brief helper function to register a data writer to a data registry which is part of the given
 * component registry
 *
 * @param[in] components the components registry to get the data registry from where to register the
 * data writer
 * @param[in] writer the writer to register
 * @return fep3::Result
 */
fep3::Result addToComponents(arya::DataWriter& writer, const fep3::arya::IComponents& components);

/**
 * @brief helper function to unregister a data writer from a data registry which is part of the
 * given component registry
 *
 * @param[in] components the components registry to get the data registry from where to unregister
 * the data writer
 * @param[in] writer the writer to unregister
 * @return fep3::Result
 */
fep3::Result removeFromComponents(arya::DataWriter& writer,
                                  const fep3::arya::IComponents& components);

template <typename T>
fep3::Result DataWriter::writeByType(T& data_to_write)
{
    fep3::base::arya::DataSampleType<T> sample_wrapup(data_to_write);
    return write(sample_wrapup);
}

} // end of namespace arya
using arya::DataWriter;

/**
 * @brief Convenience function always referring to the latest, versioned implementation
 * @remark Current implementation @see fep3::core::arya::addToComponents()
 * @copydetails fep3::core::arya::addToComponents(arya::DataWriter&, const fep3::arya::IComponents&)
 */
inline fep3::Result addToComponents(arya::DataWriter& writer,
                                    const fep3::arya::IComponents& components)
{
    return arya::addToComponents(writer, components);
}

/**
 * @brief Convenience function always referring to the latest, versioned implementation
 * @remark Current implementation @see fep3::core::arya::removeFromComponents()
 * @copydetails arya::removeFromComponents(arya::DataWriter&, const fep3::arya::IComponents&)
 */
inline fep3::Result removeFromComponents(arya::DataWriter& writer,
                                         const fep3::arya::IComponents& components)
{
    return arya::removeFromComponents(writer, components);
}

} // end of namespace core
} // end of namespace fep3

/**
 * @brief streaming operator to write a stream type
 *
 * @param[in] writer
 * @param[in] stream_type
 * @return fep::DataWriter&
 */
inline fep3::core::arya::DataWriter& operator<<(fep3::core::arya::DataWriter& writer,
                                                const fep3::base::arya::StreamType& stream_type)
{
    writer.write(stream_type);
    return writer;
}

/**
 * @brief streaming operator to write a sample
 *
 * @param[in] writer
 * @param[in] value
 * @return fep::DataWriter&
 */
inline fep3::core::arya::DataWriter& operator<<(fep3::core::arya::DataWriter& writer,
                                                const fep3::arya::IDataSample& value)
{
    writer.write(value);
    return writer;
}

/**
 * @brief streaming operator to write data
 *
 * @tparam T the type to write
 * @param[in] writer the writer to write to
 * @param[in] value  the value of type @p T to writer
 * @throw  std::runtime_error if the sample to write to is not suitable for writing @p value (e. g.
 * if memory of sample is too small)
 * @return fep::DataWriter& the writer
 * @see fep::DataWriter::writeByType
 */
template <typename T>
fep3::core::arya::DataWriter& operator<<(fep3::core::arya::DataWriter& writer, T& value)
{
    const auto writing_result = writer.writeByType(value);
    if (!writing_result) {
        throw std::runtime_error(std::string() + "writing value to writer '" + writer.getName() +
                                 "' failed with error code " +
                                 std::to_string(writing_result.getErrorCode()) +
                                 " and error description: " + writing_result.getDescription());
    }
    return writer;
}
