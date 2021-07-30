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

#include "data_signal.h"

#include <algorithm>

using namespace fep3;
using namespace fep3::native;

/***************************************************************/
/* DataSignal                                                  */
/***************************************************************/

std::string DataRegistry::DataSignal::getName() const
{
    return _name;
}

std::string DataRegistry::DataSignal::getAlias() const
{
    return _alias;
}

void DataRegistry::DataSignal::setAlias(const std::string & alias)
{
    _alias = alias;
}

base::StreamType DataRegistry::DataSignal::getType() const
{
    return _type;
}

void DataRegistry::DataSignal::setType(const IStreamType& type)
{
    _type = type;
}

bool DataRegistry::DataSignal::hasDynamicType() const
{
    return _dynamic_type;
}

/***************************************************************/
/* DataSignalIn                                                */
/***************************************************************/

DataRegistry::DataSignalIn::~DataSignalIn()
{
    unregisterFromSimulationBus();
}

void DataRegistry::DataSignalIn::registerDataListener(const std::shared_ptr<IDataReceiver>& listener)
{
    for (auto& current_listener : _listeners)
    {
        if (current_listener == listener)
        {
            return;
        }
    }
    _listeners.push_back(listener);
}

void DataRegistry::DataSignalIn::unregisterDataListener(const std::shared_ptr<IDataReceiver>& listener)
{
    auto listener_it = std::find_if(_listeners.begin(), _listeners.end(),
        [&listener](std::shared_ptr<IDataReceiver> current_listener) {return current_listener == listener;});
    if (listener_it != _listeners.end())
    {
        _listeners.erase(listener_it);
    }
}

size_t DataRegistry::DataSignalIn::getMaxQueueSize() const
{
    size_t size_result = 1;
    auto readers = _readers;
    for (const auto& reader : *readers)
    {
        auto reader_locked = reader.lock();
        if (reader_locked)
        {
            auto size_current = reader_locked->capacity();
            if (size_current > size_result)
            {
                size_result = size_current;
            }
        }
    }
    return size_result;
}

fep3::Result DataRegistry::DataSignalIn::registerAtSimulationBus(ISimulationBus& simulation_bus)
{
    try
    {
        if (hasDynamicType())
        {
            _sim_bus_reader = simulation_bus.getReader(getAlias(), getMaxQueueSize());
        }
        else
        {
            _sim_bus_reader = simulation_bus.getReader(getAlias(), getType(), getMaxQueueSize());
        }

        if (!_sim_bus_reader)
        {
            RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Registering data reader %s at simulation bus failed", getAlias().c_str());
        }
        _sim_bus_reader->reset(shared_from_this());

        return{};

    }
    catch (const std::exception& ex)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, ex.what());
    }
}

void DataRegistry::DataSignalIn::unregisterFromSimulationBus()
{
    if (_sim_bus_reader)
    {
        _sim_bus_reader.reset();
    }
}

fep3::Result DataRegistry::DataSignalIn::registerAtSignalMapping(SignalMapping& mapping)
{
    return mapping.registerDataReceiver(shared_from_this(), getName());
}

std::unique_ptr<IDataRegistry::IDataReader> DataRegistry::DataSignalIn::getReader(const size_t queue_capacity)
{
    const auto& iter = _readers->insert(_readers->end(), std::weak_ptr<DataRegistry::DataReader>());
    // Construct shared_ptr with custom deleter
    std::shared_ptr<DataRegistry::DataReader> reader{new DataRegistry::DataReader(queue_capacity),
        [readers_ptr = std::weak_ptr<DataReaderList>(_readers), iter](DataRegistry::DataReader* ptr)
        {
            delete ptr;
            auto readers = readers_ptr.lock();
            if (readers)
            {
                readers->erase(iter);
            }
        }
    };
    *iter = reader;
    return std::make_unique<DataRegistry::DataReaderProxy>(reader);
}

void DataRegistry::DataSignalIn::operator()(const data_read_ptr<const IStreamType>& type)
{
    setType(*type);

    //first of all we receive for the queues 
    auto readers = _readers;
    for (auto& reader : *readers)
    {
        //very very very slow! why we have this very slow weak pointer here ????
        auto locked_reader = reader.lock();
        if (locked_reader)
        {
            //i really do not understand this operator thisng here (it makes no sense)
            locked_reader->operator()(type);
        }
    }
    for (auto& listener : _listeners)
    {
        (*listener)(type);
    }
}

void DataRegistry::DataSignalIn::operator()(const data_read_ptr<const IDataSample>& sample)
{
    //first of all we receive for the queues
    auto readers = _readers;
    for (auto& reader : *readers)
    {
        //very very very slow! why we have this very slow weak pointer here ????
        auto locked_reader = reader.lock();
        if (locked_reader)
        {
            locked_reader->operator()(sample);
        }
    }
    for (auto& listener : _listeners)
    {
        (*listener)(sample);
    }
}

/***************************************************************/
/* DataSignalOut                                               */
/***************************************************************/

DataRegistry::DataSignalOut::~DataSignalOut()
{
    unregisterFromSimulationBus();
}

size_t DataRegistry::DataSignalOut::getMaxQueueSize() const
{
    size_t size_result = 0;
    auto writers = _writers;
    for (const auto& writer : *writers)
    {
        auto writer_locked = writer.lock();
        if (writer_locked)
        {
            auto size_current = writer_locked->capacity();
            if (size_current > size_result)
            {
                size_result = size_current;
            }
        }
    }
    return size_result;
}

fep3::Result DataRegistry::DataSignalOut::registerAtSimulationBus(ISimulationBus& simulation_bus)
{
    auto max_queue_size = getMaxQueueSize();
    try
    {
        if (hasDynamicType())
        {
            if (max_queue_size > 0)
            {
                _sim_bus_writer = simulation_bus.getWriter(getAlias(), max_queue_size);
            }
            else
            {
                _sim_bus_writer = simulation_bus.getWriter(getAlias());
            }
            if (_sim_bus_writer)
            {
                _sim_bus_writer->write(getType());
            }
        }
        else
        {
            if (max_queue_size > 0)
            {
                _sim_bus_writer = simulation_bus.getWriter(getAlias(), getType(), max_queue_size);
            }
            else
            {
                _sim_bus_writer = simulation_bus.getWriter(getAlias(), getType());
            }
        }
    }
    catch (const std::exception& ex)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, ex.what());
    }
    if (_sim_bus_writer)
    {
        return {};
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "unexpected nullptr while creating a writer %s in simbus", getName().c_str());
    }
}

void DataRegistry::DataSignalOut::unregisterFromSimulationBus()
{
    if (_sim_bus_writer)
    {
        _sim_bus_writer.reset();
    }
}

fep3::Result DataRegistry::DataSignalOut::write(const IDataSample& data_sample)
{
    //usually we should queue and write and transmit at flush call
    //but we have no simulationbus implementation where we can obtain the samples from
    // data writer of simulation bus must be redesigned !!
    if (_sim_bus_writer)
    {
        return _sim_bus_writer->write(data_sample);
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_DEVICE_NOT_READY, "Simulation bus not initialized");
    }
}

fep3::Result DataRegistry::DataSignalOut::write(const IStreamType& stream_type)
{
    setType(stream_type);

    //usually we should queue and write and transmit at flush call
    //but we have no simulationbus implementation where we can obtain the samples from
    // data writer of simulation bus must be redesigned !!
    if (_sim_bus_writer)
    {
        return _sim_bus_writer->write(stream_type);
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_DEVICE_NOT_READY, "Simulation bus not initialized");
    }
}

fep3::Result DataRegistry::DataSignalOut::transmit()
{
    if (_sim_bus_writer)
    {
        return _sim_bus_writer->transmit();
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_DEVICE_NOT_READY, "Simulation bus not initialized");
    }
}

std::unique_ptr<IDataRegistry::IDataWriter> DataRegistry::DataSignalOut::getWriter(const size_t queue_capacity)
{
    const auto& iter = _writers->insert(_writers->end(), std::weak_ptr<DataRegistry::DataWriter>());
    // Construct shared_ptr with custom deleter
    std::shared_ptr<DataRegistry::DataWriter> writer{ new DataRegistry::DataWriter(*this, queue_capacity),
        [writers_ptr = std::weak_ptr<DataWriterList>(_writers), iter](DataRegistry::DataWriter* ptr)
        {
            delete ptr;
            auto writers = writers_ptr.lock();
            if (writers)
            {
                writers->erase(iter);
            }
        }
    };
    *iter = writer;
    return std::make_unique<DataRegistry::DataWriterProxy>(writer);
}