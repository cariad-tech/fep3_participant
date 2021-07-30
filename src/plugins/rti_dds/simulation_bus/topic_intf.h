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

#pragma once

#include <fep3/components/simulation_bus/simulation_bus_intf.h>
#include <fep3/components/simulation_bus/simulation_data_access.h>
#include <plugins/rti_dds/simulation_bus/reader_item_queue.h>

/**
 * Topic provides standardizes access to all topic types
 */
class ITopic
{
protected:
    /// DTOR
    ~ITopic() = default;

public:
    virtual std::string GetTopic() = 0;
    virtual std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> createDataReader
        (size_t queue_capacity
        , const std::weak_ptr<fep3::base::SimulationDataAccessCollection<ReaderItemQueue>>& data_access_collection
        ) = 0;
    virtual std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> createDataWriter(size_t queue_capacity) = 0;
};
