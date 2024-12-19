/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "reader_item_queue.h"

#include <fep3/components/simulation_bus/simulation_data_access.h>

/**
 * Topic provides standardizes access to all topic types
 */
class ITopic {
protected:
    /// DTOR
    ~ITopic() = default;

public:
    virtual std::string GetTopic() = 0;
    virtual std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> createDataReader(
        size_t queue_capacity,
        const std::weak_ptr<fep3::base::SimulationDataAccessCollection<ReaderItemQueue>>&
            data_access_collection) = 0;
    virtual std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> createDataWriter(
        size_t queue_capacity) = 0;
};
