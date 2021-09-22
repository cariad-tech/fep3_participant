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

#include <fep3/components/base/components_intf.h>
#include <fep3/components/data_registry/data_registry_intf.h>
#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/base/sample/data_sample.h>
#include "data_reader_backlog.h"

#include <stdexcept>
#include <string>

namespace fep3
{
namespace core
{
namespace arya
{

/**
 * @brief Data Reader helper class to read data from a fep::IDataRegistry::IDataReader
 * if registered at the fep::IDataRegistry
 *
 */
class DataReader : public arya::DataReaderBacklog
{
public:
    /**
     * @brief Construct a new Data Reader
     *
     */
    DataReader();
    /**
     * @brief Construct a new Data Reader
     *
     * @param[in] name name of incoming data
     * @param[in] stream_type type of incoming data
     */
    DataReader(std::string name,
        const fep3::base::arya::StreamType& stream_type);
    /**
    * @brief Construct a new Data Reader
    *
    * @param[in] name name of incoming data
    * @param[in] stream_type type of incoming data
    * @param[in] queue_size size of the data reader's sample backlog
    */
    DataReader(std::string name,
        const fep3::base::arya::StreamType& stream_type,
        size_t queue_size);
    /**
     * @brief Construct a new Data Reader
     *
     * @tparam PLAIN_RAW_TYPE plain old c-type for the incoming data
     * @param[in] name name of the incoming data
     * @see fep3::base::arya::StreamTypePlain
     */
    template<typename PLAIN_RAW_TYPE>
    DataReader(std::string name);
    /**
     * @brief Construct a new Data Reader
     *
     * @tparam PLAIN_RAW_TYPE plain old c-type for the incoming data
     * @param[in] name name of the incoming data
     * @param[in] queue_capacity capacity of the queue (it will not grow dynamically)
     * @see fep3::base::arya::StreamTypePlain
     */
    template<typename PLAIN_RAW_TYPE>
    DataReader(std::string name, size_t queue_capacity);
    /**
     * @brief copy Construct a new Data Reader
     *
     * @param[in] other
     */
    DataReader(const DataReader& other);

    /**
     * @brief assignment operator
     *
     * @param[in] other
     * @return DataReader&
     */
    DataReader& operator=(const DataReader& other);

    /**
     * @brief move cnstruct a new Data Reader
     *
     * @param[in] other
     */
    DataReader(DataReader&& other) = default;
    /**
     * @brief move assignment
     *
     * @param[in] other
     * @return DataReader&
     */
    DataReader& operator=(DataReader&& other) = default;

    ~DataReader() = default;

    /**
     * @brief registers and retrieves to a fep::IDataRegistry::IDataReader
     *
     * @param[in] data_registry the data registry to register to
     * @return fep3::Result
     */
    fep3::Result addToDataRegistry(fep3::arya::IDataRegistry& data_registry);

    /**
     * @deprecated
     * @brief remove the readers reference to the data registry without removing the corresponding reader from the data registry
     *
     * @return fep3::Result
     * @see fep3::DataReader::removeFromDataRegistry(fep3::arya::IDataRegistry& data_registry)
     */
    [[deprecated ("fep3::DataReader::removeFromDataRegistry() is deprecated. Please use fep3::DataReader::removeFromDataRegistry(fep3::arya::IDataRegistry& data_registry) instead.")]]
    fep3::Result removeFromDataRegistry();

    /**
     * @brief remove the readers reference to the data registry and removes the corresponding reader from the registry
     *
     * @param[in] data_registry the data registry to remove from
     * @return fep3::Result
     */
    fep3::Result removeFromDataRegistry(fep3::arya::IDataRegistry& data_registry);

    /**
     * @brief will handle and receive all items from the reader queue until the given time is reached (excluding given time)
     *
     * This method implements the behaviour known from FEP SDK greater than or equal to version 2.3.0.
     * Samples having a timestamp lower than the current simulation time are considered valid for the
     * current simulation step.
     *
     * @param[in] time_of_update samples with a timestamp lower than the time_of_update are received
     */
    virtual void receiveNow(fep3::arya::Timestamp time_of_update);

    /**
     * @brief get name of the reader
     * @return the name of the reader
     */
    virtual std::string getName() const;

private:
    /// name of data reader
    std::string _name;
    /// initial type of data
    fep3::base::arya::StreamType _stream_type;
    /// IDataReader connected
    std::unique_ptr<fep3::arya::IDataRegistry::IDataReader> _connected_reader;
};


/**
 * @brief helper function to register a data reader to a data registry
 *
 * @param[in] registry the registry to register the data reader to
 * @param[in] reader the reader to register
 * @return fep3::Result
 */
fep3::Result addToDataRegistry(fep3::arya::IDataRegistry& registry, DataReader& reader);

/**
 * @brief helper function to register a data reader to a data registry which is part of the given component registry
 *
 * @param[in] components the components registry to get the data registry from where to register the data reader
 * @param[in] reader the reader to register
 * @return fep3::Result
 */
fep3::Result addToComponents(arya::DataReader& reader, const fep3::arya::IComponents& components);

/**
 * @brief helper function to remove the data reader from the data registry which is part of the given component registry
 *
 * @param[in] components the components registry to get the data registry from where to register the data reader
 * @param[in] reader the reader to remove
 * @return fep3::Result
 */
fep3::Result removeFromComponents(arya::DataReader& reader, const fep3::arya::IComponents& components);

/**
* @brief Construct a new Data Reader
*
* @tparam PLAIN_RAW_TYPE plain old c-type for the incoming data
* @param[in] name name of the incoming data
* @see fep3::base::arya::StreamTypePlain
*/

template<typename PLAIN_RAW_TYPE>
inline DataReader::DataReader(std::string name) :
    _name(std::move(name),
        _stream_type(fep3::base::arya::StreamTypePlain<PLAIN_RAW_TYPE>())),
    arya::DataReaderBacklog(1, _stream_type)
{
}

/**
* @brief Construct a new Data Reader
*
* @tparam PLAIN_RAW_TYPE plain old c-type for the incoming data
* @param[in] name name of the incoming data
* @param[in] queue_capacity capacity of the queue (it will not grow dynamically)
* @see fep3::base::arya::StreamTypePlain
*/

template<typename PLAIN_RAW_TYPE>
inline DataReader::DataReader(std::string name, size_t queue_capacity) :
    _name(std::move(name),
        _stream_type(fep3::base::arya::StreamTypePlain<PLAIN_RAW_TYPE>())),
    arya::DataReaderBacklog(queue_capacity, _stream_type)
{
}

}
using arya::DataReader;

/**
 * @brief helper function to register a data reader to a data registry which is part of the given component registry
 *
 * @param[in] components the components registry to get the data registry from where to register the data reader
 * @param[in] reader the reader to register
 * @return fep3::Result
 */
inline fep3::Result addToComponents(DataReader& reader, const fep3::arya::IComponents& components)
{
    return arya::addToComponents(reader, components);
}

/**
 * @brief helper function to remove the data reader from the data registry which is part of the given component registry
 *
 * @param[in] components the components registry to get the data registry from where to register the data reader
 * @param[in] reader the reader to remove
 * @return fep3::Result
 */
inline fep3::Result removeFromComponents(DataReader& reader, const fep3::arya::IComponents& components)
{
    return arya::removeFromComponents(reader, components);
}

}
} // end of fep namespace

/**
 * @brief streaming operator to read data to the given Memory.
 * The oldest data sample available is popped from the data reader backlog.
 *
 * @tparam T                  the type of memory
 * @param[in] reader             the reader to read from
 * @param[out] value              the value to copy the received sample content to
 * @throw  std::runtime_error if data size of \p value does not match the size of the sample to be read
 * @return const fep::DataReader&
 */
template<typename T>
fep3::core::arya::DataReader& operator>> (fep3::core::arya::DataReader& reader,
    T& value)
{
    fep3::data_read_ptr<const fep3::arya::IDataSample> ptr = reader.popSampleOldest();
    if (ptr)
    {
        fep3::base::arya::DataSampleType<T> sample_wrapup(value);
        auto copied_bytes = ptr->read(sample_wrapup);
        if(copied_bytes != sample_wrapup.size())
        {
            throw std::runtime_error(std::string() + "reading sample from reader " + reader.getName() + " failed");
        }
    }
    return reader;
}

/**
 * @brief streaming operator to read data to the given Memory with an optional output
 * The oldest data sample available is popped from the data reader backlog.
 *
 * @tparam T                    the type of memory for the opitonal output
 * @param[in] reader            the reader to read from
 * @param[out] value            the optional value to copy the received sample content to, it can be empty
 * @return const fep::DataReader&
 */
template<typename T>
fep3::core::arya::DataReader& operator>> (fep3::core::arya::DataReader& reader, 
    fep3::arya::Optional<T>& value)
{
    fep3::data_read_ptr<const fep3::arya::IDataSample> ptr = reader.popSampleOldest();
    if (ptr)
    {
        fep3::base::arya::DataSampleType<T> sample_wrapup(*value);
        auto copied_bytes = ptr->read(sample_wrapup);
        if(copied_bytes != sample_wrapup.size())
        {
            value.reset();
            throw std::runtime_error(std::string() + "reading sample from reader " + reader.getName() + " failed");
        }
        else
        {
            value = *value; // engage the value manually.
        }
    }
    else
    {
        value.reset(); // disengage the value.
    }

    return reader;
}


/**
 * @brief streaming operator to read and copy a type
 *
 * @param[in] reader the reader
 * @param[out] value  the stream type value to read
 * @return const fep::DataReader&
 */
inline fep3::core::arya::DataReader& operator>>(fep3::core::arya::DataReader& reader,
    fep3::base::arya::StreamType& value)
{
    fep3::data_read_ptr<const fep3::arya::IStreamType> ptr = reader.readType();
    if (ptr)
    {
        value = *ptr;
    }
    return reader;
}

/**
 * @brief streaming operator to read a sample read pointer
 * The oldest data sample available is popped from the data reader backlog.
 * @param reader the reader
 * @param value the stream type value to read
 * @return const fep3::core::DataReader&
 */
inline fep3::core::arya::DataReader& operator>>(fep3::core::arya::DataReader& reader,
    fep3::data_read_ptr<const fep3::arya::IDataSample>& value)
{
    value = reader.popSampleOldest();
    return reader;
}

/**
 * @brief streaming operator to read a stream type read pointer
 *
 * @param[in] reader the reader
 * @param[out] value the stream type value to read
 * @return const fep3::core::arya::DataReader&
 */
inline fep3::core::arya::DataReader& operator>> (fep3::core::arya::DataReader& reader,
    fep3::data_read_ptr<const fep3::arya::IStreamType>& value)
{
    value = reader.readType();
    return reader;
}
