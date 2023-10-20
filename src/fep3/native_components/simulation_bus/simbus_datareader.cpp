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

#include "simbus_datareader.h"

namespace fep3 {
namespace native {

size_t SimulationBus::DataReader::size() const
{
    return _item_queue->size();
}

SimulationBus::DataReader::DataReader(
    const std::shared_ptr<DataItemQueue<>>& item_queue,
    const std::weak_ptr<base::SimulationDataAccessCollection<DataItemQueue<>>>&
        data_access_collection)
    : _item_queue{item_queue}, _data_access_collection{data_access_collection}
{
}

SimulationBus::DataReader::~DataReader()
{
    // remove sink from data sink collection (if any)
    reset();
}

size_t SimulationBus::DataReader::capacity() const
{
    return _item_queue->capacity();
}

bool SimulationBus::DataReader::pop(ISimulationBus::IDataReceiver& onReceive)
{
    if (_item_queue->size() == 0) {
        return false;
    }

    auto res = _item_queue->pop();

    dispatch<decltype(res)>(res, onReceive);

    return true;
}

void SimulationBus::DataReader::reset(
    const std::shared_ptr<arya::ISimulationBus::IDataReceiver>& receiver)
{
    const auto& data_access_collection = _data_access_collection.lock();
    // remove the previous receiver (if any)
    if (_data_access_iterator) {
        if (data_access_collection) {
            data_access_collection->remove(_data_access_iterator.value());
        }
        _data_access_iterator.reset();
    }

    if (data_access_collection && receiver) {
        // add the new receiver to the data access collection and store the iterator to the new
        // entry
        _data_access_iterator = data_access_collection->add(receiver, _item_queue);
    }
}

Optional<Timestamp> SimulationBus::DataReader::getFrontTime() const
{
    return _item_queue->getFrontTime();
}

} // namespace native
} // namespace fep3
