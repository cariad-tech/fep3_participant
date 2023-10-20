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

#include <fep3/components/simulation_bus/simulation_bus_intf.h>

#include <deque>

namespace fep3 {
namespace base {
namespace arya {

/**
 * Structure containing simulation data access
 * @note This class is meant to be used to implement data triggered simulation data access in a
 * Simulation Bus implementation
 */
template <typename item_queue_type>
struct SimulationDataAccess {
    /**
     * @brief The receiver receiving new data in data triggered reception
     */
    std::shared_ptr<fep3::arya::ISimulationBus::IDataReceiver> _receiver;
    /**
     * @brief The item queue to pop data from
     */
    std::shared_ptr<item_queue_type> _item_queue;
};
/**
 * Collection of data access objects to be filled by @ref fep3::ISimulationBus::IDataReader::reset
 * @tparam item_queue_type Type of the item queue to be used by the data access collection
 */
template <typename item_queue_type>
class SimulationDataAccessCollection : private std::deque<SimulationDataAccess<item_queue_type>> {
public:
    /// const iterator to an entry of the simulation data access collection
    using const_iterator =
        typename std::deque<SimulationDataAccess<item_queue_type>>::const_iterator;

    /**
     * @brief Adds the @p receiver and the @p item_queue to the data access collection
     * @param receiver The receiver to be added
     * @param item_queue The item queue to be added
     * @return Iterator to the added entry
     */
    const_iterator add(const std::shared_ptr<fep3::arya::ISimulationBus::IDataReceiver>& receiver,
                       const std::shared_ptr<item_queue_type>& item_queue)
    {
        return this->insert(cend(), {receiver, item_queue});
    }

    /**
     * @brief Removes an entry from the data access collection
     * @param data_receiver_iterator The iterator to the entry to be removed
     */
    void remove(const const_iterator& data_receiver_iterator)
    {
        this->erase(data_receiver_iterator);
    }

    /**
     * @brief Clears the data access collection
     */
    void clear()
    {
        this->std::deque<arya::SimulationDataAccess<item_queue_type>>::clear();
    }

    using std::deque<SimulationDataAccess<item_queue_type>>::cbegin;
    using std::deque<SimulationDataAccess<item_queue_type>>::cend;
    using std::deque<SimulationDataAccess<item_queue_type>>::size;
};

} // namespace arya
using arya::SimulationDataAccess;
using arya::SimulationDataAccessCollection;
} // namespace base
} // namespace fep3
