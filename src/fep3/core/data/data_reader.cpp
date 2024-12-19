/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/base/data_registry/data_registry.h>
#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/core/data/data_reader.h>

namespace {
// Set to 2 to avoid loss of data samples if a received new data sample overwrites an existing old
// data sample which has not been read by the user yet.
// ATTENTION: Not yet read samples might still be overwritten if more samples than data reader
// backlog capacity are received, in case of default capacity this is the case if 2 or more samples
// are received.
constexpr size_t default_data_reader_backlog_capacity{2};
} // namespace

namespace fep3 {
namespace core {
namespace arya {

DataReader::DataReader(const std::function<bool(fep3::Timestamp, fep3::Timestamp)>& time_comparator,
                       size_t purged_sample_log_capacity)
    : _time_comparator(time_comparator),
      DataReaderBacklog(default_data_reader_backlog_capacity,
                        base::StreamType(fep3::base::arya::meta_type_raw),
                        purged_sample_log_capacity)
{
}

DataReader::DataReader(std::string name,
                       const base::StreamType& stream_type,
                       const std::function<bool(fep3::Timestamp, fep3::Timestamp)>& time_comparator,
                       size_t purged_sample_log_capacity)
    : _name(std::move(name)),
      _time_comparator(time_comparator),
      DataReaderBacklog(
          default_data_reader_backlog_capacity, stream_type, purged_sample_log_capacity)
{
}

DataReader::DataReader(std::string name,
                       const base::StreamType& stream_type,
                       size_t queue_size,
                       const std::function<bool(fep3::Timestamp, fep3::Timestamp)>& time_comparator,
                       size_t purged_sample_log_capacity)
    : _name(std::move(name)),
      _time_comparator(time_comparator),
      DataReaderBacklog(queue_size, stream_type, purged_sample_log_capacity)
{
}

fep3::Result DataReader::addToDataRegistry(IDataRegistry& data_registry)
{
    if (const auto& stream_type = readType()) {
        return base::addDataIn(
            data_registry, _connected_reader, _name, *stream_type, getSampleQueueCapacity());
    }
    else {
        return CREATE_ERROR_DESCRIPTION(ERR_POINTER, "DataReader has no stream type set.");
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
    if (_connected_reader) {
        while (_connected_reader->getFrontTime() &&
               _time_comparator(*_connected_reader->getFrontTime(), time_of_update)) {
            if (!_connected_reader->pop(*this)) {
                // something went wrong
                break;
            }
        }
    }
}

std::string DataReader::getName() const
{
    return _name;
}

void DataReader::logPurgedSamples(const fep3::arya::ILogger* logger) const
{
    DataReaderBacklog::logPurgedSamples(_name, logger);
}

fep3::Result addToDataRegistry(IDataRegistry& registry, DataReader& reader)
{
    return reader.addToDataRegistry(registry);
}

fep3::Result addToComponents(DataReader& reader, const IComponents& components)
{
    auto data_registry = components.getComponent<IDataRegistry>();
    if (data_registry) {
        return addToDataRegistry(*data_registry, reader);
    }
    else {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
                                 "%s is not part of the given component registry",
                                 getComponentIID<IDataRegistry>().c_str());
    }
}

fep3::Result removeFromComponents(DataReader& reader, const IComponents& components)
{
    auto data_registry = components.getComponent<IDataRegistry>();
    if (data_registry) {
        return reader.removeFromDataRegistry(*data_registry);
    }
    else {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
                                 "%s is not part of the given component registry",
                                 getComponentIID<IDataRegistry>().c_str());
    }
}

} // namespace arya
} // namespace core
} // namespace fep3
