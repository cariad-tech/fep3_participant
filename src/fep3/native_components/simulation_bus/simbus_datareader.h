/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "data_item_queue.h"
#include "simulation_bus.h"

#include <fep3/components/simulation_bus/simulation_data_access.h>

namespace fep3 {
namespace native {

class SimulationBus::DataReader : public arya::ISimulationBus::IDataReader {
public:
    /**
     * CTOR for a data reader
     * @param item_queue The item queue this data reader shall work on.
     *                   The reader takes (shared) ownership of the @p item_queue
     * @param data_access_collection Weak pointer to the collection of data access.
     *                               Calls to @ref DataReader::reset will add data access
     *                               to this collection.
     */
    DataReader(const std::shared_ptr<DataItemQueue<>>& item_queue,
               const std::weak_ptr<base::SimulationDataAccessCollection<DataItemQueue<>>>&
                   data_access_collection);
    ~DataReader() override;
    DataReader(const DataReader&) = delete;
    DataReader(DataReader&&) = delete;
    DataReader& operator=(const DataReader&) = delete;
    DataReader& operator=(DataReader&&) = delete;

    virtual size_t size() const override;

    virtual size_t capacity() const override;

    virtual bool pop(arya::ISimulationBus::IDataReceiver& onReceive) override;

    virtual void reset(
        const std::shared_ptr<arya::ISimulationBus::IDataReceiver>& receiver = {}) override;

    virtual Optional<Timestamp> getFrontTime() const override;

    template <typename tuple_type>
    static void dispatch(tuple_type& data, fep3::arya::ISimulationBus::IDataReceiver& receiver)
    {
        auto data_sample = std::get<0>(data);
        if (data_sample) {
            receiver(data_sample);
        }

        auto stream_type = std::get<1>(data);
        if (stream_type) {
            receiver(stream_type);
        }
    }

private:
    // the reader takes ownership of the item queue -> shared_ptr
    std::shared_ptr<DataItemQueue<>> _item_queue{nullptr};
    // the reader does not take (permanent) ownership of the data access collection -> weak_ptr
    std::weak_ptr<base::SimulationDataAccessCollection<DataItemQueue<>>> _data_access_collection;
    Optional<base::SimulationDataAccessCollection<DataItemQueue<>>::const_iterator>
        _data_access_iterator;
};

} // namespace native
} // namespace fep3
