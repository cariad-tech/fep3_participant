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


#include "simulation_bus.h"

#include <a_util/result.h>

#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/components/simulation_bus/simulation_data_access.h>
#include "simbus_datareader.h"
#include "simbus_datawriter.h"

#include <algorithm>
#include <future>
#include <mutex>
#include <vector>
#include <set>
#include <unordered_map>

namespace fep3
{
namespace native
{

class SimulationBus::Impl
{
    std::vector<base::StreamMetaType> _supported_meta_types;
    std::set<std::string> _registered_readers;
    std::set<std::string> _registered_writers;

    // We need a sub object for collection data access (data triggered behavior)
    // because the simulation bus cannot be passed to the reader, as lifetime of simulation bus
    // cannot be controlled.
    std::shared_ptr<base::SimulationDataAccessCollection<DataItemQueue<>>> _data_access_collection;

    using Transmitters = std::unordered_map<std::string, std::shared_ptr<Transmitter>>;
    static Transmitters& getTransmitters()
    {
        static Transmitters transmitters;
        return transmitters;
    }

    struct ExitSignal
    {
        std::promise<void> trigger;
        mutable std::mutex data_triggered_reception_mutex;
    } _exitSignal;

public:

    Impl()
        : _data_access_collection(std::make_shared<base::SimulationDataAccessCollection<DataItemQueue<>>>())
    {
        using namespace fep3::base::arya;
        _supported_meta_types.emplace_back(meta_type_plain);
        _supported_meta_types.emplace_back(meta_type_plain_array);
        _supported_meta_types.emplace_back(meta_type_string);
        _supported_meta_types.emplace_back(meta_type_video);
        _supported_meta_types.emplace_back(meta_type_audio);
        _supported_meta_types.emplace_back(meta_type_raw);
        _supported_meta_types.emplace_back(meta_type_ddl);
    }
    ~Impl()
    {
    }

    bool isSupported(const IStreamType& stream_type) const
    {
        auto result = std::find(_supported_meta_types.begin(), _supported_meta_types.end(),
                stream_type);

        return result != _supported_meta_types.end();
    }

    std::unique_ptr<IDataReader> getReader(const std::string& name, const IStreamType&,
            size_t queue_capacity)
    {
        return getReader(name, queue_capacity);
    }

    std::unique_ptr<IDataReader> getReader(const std::string& name, size_t queue_capacity)
    {
        if (registerAndCheckIfExists(_registered_readers, name))
        {
            return nullptr;
        }

        auto receive_queue = std::make_shared<DataItemQueue<>>(queue_capacity);

        getTransmitters()[name]->add(name, receive_queue);

        auto reader = std::make_unique<DataReader>(receive_queue, _data_access_collection);

        return reader;
    }

    std::unique_ptr<IDataWriter> getWriter(const std::string& name, size_t queue_capacity)
    {
        if (registerAndCheckIfExists(_registered_writers, name))
        {
            return nullptr;
        }

        auto writer = std::make_unique<DataWriter>(name, queue_capacity, getTransmitters()[name]);
        return writer;
    }

    void startBlockingReception(const std::function<void()>& reception_preparation_done_callback)
    {
        // RAII ensuring invocation of callback exactly once (even in case exception is thrown)
        std::unique_ptr<bool, std::function<void(bool*)>> reception_preparation_done_callback_caller
            (nullptr, [reception_preparation_done_callback](bool* p)
            {
                reception_preparation_done_callback();
                delete p;
            });
        if(reception_preparation_done_callback)
        {
            reception_preparation_done_callback_caller.reset(new bool);
        }

        if(_data_access_collection)
        {
            const auto& data_access_collection = *_data_access_collection.get();
            {
                // start polling the item queues only if there are any data access entries in the collection
                if(0 < _data_access_collection->size())
                {
                    std::unique_lock<std::mutex> lock(_exitSignal.data_triggered_reception_mutex);
                    std::future<void> futureObj = _exitSignal.trigger.get_future();

                    // the Simulation Bus is now prepared for the reception of data and for a call to stopBlockingReception
                    reception_preparation_done_callback_caller.reset();

                    while (futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout)
                    {
                        for
                            (auto data_access_iterator = data_access_collection.cbegin()
                            ; data_access_iterator != data_access_collection.cend()
                            ; ++data_access_iterator
                            )
                        {
                            auto res = data_access_iterator->_item_queue->pop();
                            DataReader::dispatch<decltype(res)>(res, *data_access_iterator->_receiver.get());
                        }
                    }

                    return;
                }
            }
        }
    }

    void stopBlockingReception()
    {
        _exitSignal.trigger.set_value();

        {
            std::unique_lock<std::mutex> lock(_exitSignal.data_triggered_reception_mutex);
            _exitSignal.trigger = std::promise<void> {};
        }
    }

    fep3::Result reset()
    {
        _registered_readers.clear();
        _registered_writers.clear();
        _data_access_collection->clear();
        getTransmitters().clear();

        return {};
    }

private:
    bool registerAndCheckIfExists(std::set<std::string>& registry, const std::string& name)
    {
        if (registry.find(name) != registry.end())
        {
            return true;
        }

        if (getTransmitters().find(name) == getTransmitters().end())
        {
            getTransmitters()[name] = std::make_shared<Transmitter>();
        }

        registry.emplace(name);

        return false;
    }
};

SimulationBus::SimulationBus() :
        _impl(std::make_unique<SimulationBus::Impl>())
{

}

SimulationBus::~SimulationBus()
{
}

fep3::Result SimulationBus::initialize()
{
    return _impl->reset();
}

fep3::Result SimulationBus::deinitialize()
{
    return _impl->reset();
}

bool SimulationBus::isSupported(const IStreamType& stream_type) const
{
    return _impl->isSupported(stream_type);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> SimulationBus::getReader(const std::string& name,
        const IStreamType& stream_type)
{
    return _impl->getReader(name, stream_type, 1);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> SimulationBus::getReader(const std::string& name,
        const IStreamType& stream_type, size_t queue_capacity)
{
    return _impl->getReader(name, stream_type, queue_capacity);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> SimulationBus::getReader(const std::string& name)
{
    return _impl->getReader(name, 1);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> SimulationBus::getReader(const std::string& name,
        size_t queue_capacity)
{
    return _impl->getReader(name, queue_capacity);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> SimulationBus::getWriter(const std::string& name,
        const IStreamType&)
{
    return _impl->getWriter(name, 1);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> SimulationBus::getWriter(const std::string& name,
        const IStreamType&,
        size_t queue_capacity)
{
    return _impl->getWriter(name, queue_capacity);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> SimulationBus::getWriter(const std::string& name)
{
    return _impl->getWriter(name, 1);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> SimulationBus::getWriter(const std::string& name,
        size_t queue_capacity)
{
    return _impl->getWriter(name, queue_capacity);
}

void SimulationBus::startBlockingReception(const std::function<void()>& reception_preparation_done_callback)
{
    _impl->startBlockingReception(reception_preparation_done_callback);
}

void SimulationBus::stopBlockingReception()
{
    _impl->stopBlockingReception();
}

} // namespace native
} // namespace fep3
