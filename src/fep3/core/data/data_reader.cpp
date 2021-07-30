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


#include <fep3/core/data/data_reader.h>
#include <fep3/base/data_registry/data_registry.h>

namespace
{
    // Set to 2 to avoid loss of data samples if a received new data sample
    // overwrites an existing old data sample which has not been read by the user yet.
    // ATTENTION: Not yet read samples might still be overwritten if more samples
    // than data reader backlog capacity are received, in case of default capacity this
    // is the case if 2 or more samples are received.
    constexpr size_t default_data_reader_backlog_capacity{2};
}

namespace fep3
{
namespace core
{
namespace arya
{

DataReader::DataReader() :
    _stream_type(fep3::base::arya::meta_type_raw),
    DataReaderBacklog(default_data_reader_backlog_capacity, base::StreamType(fep3::base::arya::meta_type_raw))
{
}

DataReader::DataReader(std::string name, const base::StreamType & stream_type) :
    _name(std::move(name)),
    _stream_type(stream_type),
    DataReaderBacklog(default_data_reader_backlog_capacity, stream_type)
{
}

DataReader::DataReader(std::string name, const base::StreamType & stream_type, size_t queue_size) :
    _name(std::move(name)),
    _stream_type(stream_type),
    DataReaderBacklog(queue_size, stream_type)
{
}

DataReader::DataReader(const DataReader & other) :
    _name(other._name),
    _stream_type(other._stream_type),
    DataReaderBacklog(other.getSampleQueueCapacity(), other._stream_type)
{
}

DataReader& DataReader::operator=(const DataReader& other)
{
    _name = other._name;
    _stream_type = other._stream_type;
    _connected_reader.reset();

    return *this;
}

fep3::Result DataReader::addToDataRegistry(IDataRegistry& data_registry)
{
    _connected_reader = base::addDataIn(data_registry, _name, _stream_type, getSampleQueueCapacity());
    if (_connected_reader)
    {
        return {};
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_DEVICE_NOT_READY, "could not register Data Reader");
    }
}

fep3::Result DataReader::removeFromDataRegistry()
{
    _connected_reader.reset();
    return {};
}

fep3::Result DataReader::removeFromDataRegistry(fep3::arya::IDataRegistry& data_registry)
{
    _connected_reader.reset();
    return base::removeDataIn(data_registry, _name);
}

void DataReader::receiveNow(Timestamp time_of_update)
{
    if (_connected_reader)
    {
        while (_connected_reader->getFrontTime()
            && *_connected_reader->getFrontTime() < time_of_update)
        {
            if (isFailed(_connected_reader->pop(*this)))
            {
                //something went wrong
                break;
            }
        }
    }
}


std::string DataReader::getName() const
{
    return _name;
}

fep3::Result addToDataRegistry(IDataRegistry& registry, DataReader& reader)
{
    return reader.addToDataRegistry(registry);
}

fep3::Result addToComponents(DataReader& reader, const IComponents& components)
{
    auto data_registry = components.getComponent<IDataRegistry>();
    if (data_registry)
    {
        return addToDataRegistry(*data_registry, reader);
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "%s is not part of the given component registry", getComponentIID<IDataRegistry>().c_str());
    }
}

fep3::Result removeFromComponents(DataReader& reader, const IComponents& components)
{
    auto data_registry = components.getComponent<IDataRegistry>();
    if (data_registry)
    {
        return reader.removeFromDataRegistry(*data_registry);
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "%s is not part of the given component registry", getComponentIID<IDataRegistry>().c_str());
    }
}

}
}
}

