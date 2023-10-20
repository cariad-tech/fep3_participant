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

#include "data_io.h"

#include <fep3/base/stream_type/default_stream_type.h>

namespace fep3 {
namespace native {
namespace arya {
/**
 * Base class for all data signals. Has a name and a stream type.
 */
class DataRegistry::DataSignal {
public:
    DataSignal() = delete;
    DataSignal(DataSignal&&) = default;
    DataSignal(const DataSignal&) = default;
    DataSignal& operator=(DataSignal&&) = default;
    DataSignal& operator=(const DataSignal&) = default;
    DataSignal(const std::string name,
               const std::string alias,
               const IStreamType& type,
               bool dynamic_type)
        : _name(std::move(name)), _alias(std::move(alias)), _type(type), _dynamic_type(dynamic_type)
    {
    }
    virtual ~DataSignal() = default;

    std::string getName() const;
    std::string getAlias() const;
    void setAlias(const std::string&);
    base::StreamType getType() const;
    void setType(const IStreamType& type);

    bool hasDynamicType() const;

private:
    std::string _name{};
    std::string _alias{};
    base::StreamType _type{fep3::base::arya::meta_type_raw};
    bool _dynamic_type{false};
};

/**
 * Internal input signal class that holds the reader registered at the simulation bus
 * and that redirects all incoming data of its signal to all registered data receive listeners.
 */
class DataRegistry::DataSignalIn
    : public DataSignal,
      public ISimulationBus::IDataReceiver,
      public std::enable_shared_from_this<ISimulationBus::IDataReceiver> {
public:
    DataSignalIn() = delete;
    DataSignalIn(DataSignalIn&&) = default;
    DataSignalIn(const DataSignalIn&) = default;
    DataSignalIn& operator=(DataSignalIn&&) = default;
    DataSignalIn& operator=(const DataSignalIn&) = default;
    DataSignalIn(const std::string& name,
                 const std::string& alias,
                 const IStreamType& type,
                 bool dynamic_type)
        : DataSignal(name, alias, type, dynamic_type)
    {
    }
    ~DataSignalIn() override;

    fep3::Result registerAtSimulationBus(ISimulationBus& simulation_bus);
    void unregisterFromSimulationBus();

    fep3::Result registerAtSignalMapping(SignalMapping& mapping);

    void registerDataListener(const std::shared_ptr<IDataReceiver>& listener);
    void unregisterDataListener(const std::shared_ptr<IDataReceiver>& listener);

    // Creates a new reader, adds it to the internal list and returns a proxy object to it
    std::unique_ptr<IDataRegistry::IDataReader> getReader(const size_t queue_capacity);

public:
    void operator()(const data_read_ptr<const IStreamType>& type) override;
    void operator()(const data_read_ptr<const IDataSample>& sample) override;

private:
    std::unique_ptr<ISimulationBus::IDataReader> _sim_bus_reader;

    typedef std::list<std::weak_ptr<DataRegistry::DataReader>> DataReaderList;
    std::shared_ptr<DataReaderList> _readers{std::make_shared<DataReaderList>()};
    std::vector<std::shared_ptr<IDataReceiver>> _listeners{};

    size_t getMaxQueueSize() const;
};

/**
 * Internal output signal class that holds the writer registered at the simulation bus.
 */
class DataRegistry::DataSignalOut : public DataSignal, public ISimulationBus::IDataWriter {
public:
    DataSignalOut() = delete;
    DataSignalOut(DataSignalOut&&) = default;
    DataSignalOut(const DataSignalOut&) = default;
    DataSignalOut& operator=(DataSignalOut&&) = default;
    DataSignalOut& operator=(const DataSignalOut&) = default;
    DataSignalOut(const std::string& name,
                  const std::string& alias,
                  const IStreamType& type,
                  bool dynamic_type)
        : DataSignal(name, alias, type, dynamic_type)
    {
    }
    ~DataSignalOut() override;

    fep3::Result registerAtSimulationBus(ISimulationBus& simulation_bus);
    void unregisterFromSimulationBus();

    // Creates a new writer, adds it to the internal list and returns a proxy object to it
    std::unique_ptr<IDataRegistry::IDataWriter> getWriter(const size_t queue_capacity);

public:
    fep3::Result write(const IDataSample& data_sample) override;
    fep3::Result write(const IStreamType& stream_type) override;
    fep3::Result transmit() override;

private:
    std::unique_ptr<ISimulationBus::IDataWriter> _sim_bus_writer;
    typedef std::list<std::weak_ptr<DataRegistry::DataWriter>> DataWriterList;
    std::shared_ptr<DataWriterList> _writers{std::make_shared<DataWriterList>()};
    size_t getMaxQueueSize() const;
};
} // namespace arya
} // namespace native
} // namespace fep3