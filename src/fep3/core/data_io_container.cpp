/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/components/job_registry/job_configuration.h>
#include <fep3/components/job_registry/job_registry_intf.h>
#include <fep3/core/data/data_reader.h>
#include <fep3/core/data/data_writer.h>
#include <fep3/core/data_io_container.h>
#include <fep3/core/default_job_config_visitor.h>
#include <fep3/core/participant.h>

#include <list>

namespace fep3::core {

fep3::core::DataReader* DataIOContainer::addDataIn(
    const std::string& name,
    const fep3::arya::IStreamType& type,
    size_t queue_size,
    const fep3::catelyn::JobConfiguration& job_configuration)
{
    DefaultJobConfigVisitor visitor(
        name, type, _readers, queue_size, _configuration.purged_samples_log_capacity);
    job_configuration.acceptVisitor(visitor);
    return &_readers.back();
}

fep3::core::DataWriter* DataIOContainer::addDataOut(const std::string& name,
                                                    const fep3::arya::IStreamType& type,
                                                    size_t queue_size)
{
    _writers.emplace_back(name, type, queue_size);
    return &_writers.back();
}

fep3::Result DataIOContainer::executeDataIn(fep3::Timestamp time_of_execution)
{
    for (auto& current: _readers) {
        current.receiveNow(time_of_execution);
    }
    return {};
}

fep3::Result DataIOContainer::executeDataOut(fep3::Timestamp time_of_execution)
{
    fep3::Result res{};
    for (auto& current: _writers) {
        // this will empty the data writer queues and write the data usually to the simulation
        // bus usually it will also call the transimission of the queue content this will
        // usually NOT wait for data really transmitted and it will usually NOT wait for the
        // response of the receivers of the data
        res |= current.flushNow(time_of_execution);
    }

    for (auto& current: _readers) {
        if (_configuration.clear_input_signal_queues) {
            current.clear();
        }
    }

    return res;
}

fep3::Result DataIOContainer::addToDataRegistry(fep3::arya::IDataRegistry& data_registry,
                                                fep3::arya::IClockService& clock_service)
{
    std::list<DataReader*> readers;
    std::list<DataWriter*> writers;
    fep3::Result result;

    for (auto& reader: _readers) {
        result = reader.addToDataRegistry(data_registry);
        if (!result) {
            break;
        }
        readers.push_back(&reader);
    }

    // Rollback if some reader fails
    if (!result) {
        for (auto reader: readers) {
            reader->removeFromDataRegistry(data_registry);
        }
        return result;
    }

    for (auto& writer: _writers) {
        result = writer.addToDataRegistry(data_registry);
        if (!result) {
            break;
        }
        writers.push_back(&writer);

        result = writer.addClockTimeGetter([&]() { return clock_service.getTime(); });
        if (!result) {
            break;
        }
    }

    // Rollback if some writer fails
    if (!result) {
        for (auto writer: writers) {
            if (writer->removeFromDataRegistry(data_registry)) {
                writer->removeClockTimeGetter();
            }
        }
        return result;
    }

    return {};
}

void DataIOContainer::removeFromDataRegistry(fep3::arya::IDataRegistry& data_registry)
{
    for (auto& reader: _readers) {
        reader.removeFromDataRegistry(data_registry);
    }

    for (auto& writer: _writers) {
        if (writer.removeFromDataRegistry(data_registry)) {
            writer.removeClockTimeGetter();
        }
    }
}

void DataIOContainer::setConfiguration(const DataIOContainerConfiguration& configuration)
{
    _configuration = configuration;
}

void DataIOContainer::logIOInfo(const fep3::arya::ILogger* logger)
{
    for (auto& reader: _readers) {
        reader.logPurgedSamples(logger);
    }
}

} // namespace fep3::core
