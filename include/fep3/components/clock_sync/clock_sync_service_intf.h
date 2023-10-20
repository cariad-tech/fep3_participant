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

#include <fep3/components/base/component_iid.h>

/**
 * @brief Name of the clock synchronization entry in configuration service
 */
#define FEP3_CLOCKSYNC_SERVICE_CONFIG "clock_synchronization"

/**
 * @brief Name of the property to set the timing master in the clock synchronization service
 */
#define FEP3_TIMING_MASTER_PROPERTY "timing_master"

/**
 * @brief Full path of the property for the timing master to use
 */
#define FEP3_CLOCKSYNC_SERVICE_CONFIG_TIMING_MASTER                                                \
    FEP3_CLOCKSYNC_SERVICE_CONFIG "/" FEP3_TIMING_MASTER_PROPERTY

/**
 * @brief Name of the timing client continuous clock provided by the native synchronization service.
 * The clock periodically synchronizes its time with a given timing master.
 * The period after which the synchronization occurs is configurable via
 * FEP3_CLOCKSYNC_SERVICE_CONFIG_SLAVE_SYNC_CYCLE_TIME and defaults to
 * FEP3_SLAVE_SYNC_CYCLE_TIME_DEFAULT_VALUE. The clock uses the Christian's algorithm to interpolate
 * the time during synchronization steps.
 */
#define FEP3_CLOCK_SLAVE_MASTER_ONDEMAND "slave_master_on_demand"

/**
 * @brief Name of the timing client discrete clock provided by the native synchronization service.
 * The clock receives time update events by a timing master which must be a discrete clock.
 */
#define FEP3_CLOCK_SLAVE_MASTER_ONDEMAND_DISCRETE "slave_master_on_demand_discrete"

/**
 * @brief Period at which the clock of the timing slave continuous clock synchronizes with the
 * timing master. Only relevant for timing slave configuration if the timing slave's main clock is
 * set to FEP3_CLOCK_SLAVE_MASTER_ONDEMAND. The timing slaves's clock cyclically requests the
 * current simulation time from the timing master. The duration in ns which has to pass between
 * those time requests is configured by this property.
 */
#define FEP3_SLAVE_SYNC_CYCLE_TIME_PROPERTY "sync_cycle_time"

/**
 * @brief Period at which the native timing client continuous clock synchronizes with the timing
 * master. Only relevant for timing client configuration if the timing client's main clock is set to
 * 'slave_master_on_demand'. The timing client's slave clock cyclically requests the current
 * simulation time from the timing master. The duration in ns which has to pass between those time
 * requests is configured by this property.
 */
#define FEP3_CLOCKSYNC_SERVICE_CONFIG_SLAVE_SYNC_CYCLE_TIME                                        \
    FEP3_CLOCKSYNC_SERVICE_CONFIG "/" FEP3_SLAVE_SYNC_CYCLE_TIME_PROPERTY

/**
 * @brief Default value of the built-in 'slave_master_on_demand' clock's slave sync cycle time
 * property in ns.
 */
#define FEP3_SLAVE_SYNC_CYCLE_TIME_DEFAULT_VALUE 100000000

namespace fep3 {
namespace arya {
/**
 * @brief Interface of the Clock Sync Service
 */
class IClockSyncService {
public:
    /**
     * @brief Definition of the local clock sync service component ID
     */
    FEP_COMPONENT_IID("clock_sync_service.arya.fep3.iid");

protected:
    /// DTOR
    ~IClockSyncService() = default;
};

} // namespace arya
using arya::IClockSyncService;
} // namespace fep3
