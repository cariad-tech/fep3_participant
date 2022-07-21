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

#include <string>

#include <fep3/fep3_optional.h>
#include <fep3/fep3_errors.h>
#include <fep3/components/base/component_iid.h>
#include <fep3/components/clock/clock_registry_intf.h>

/**
* @brief The clock service main property tree entry node
*
*/
#define FEP3_CLOCK_SERVICE_CONFIG "clock"

/**
* @brief The main clock configuration property name
* Use this to set the main clock by configuration.
*
*/
#define FEP3_MAIN_CLOCK_PROPERTY "main_clock"
/**
* @brief The clock service main clock configuration node
* Use this to set the main clock by configuration.
*
*/
#define FEP3_CLOCK_SERVICE_MAIN_CLOCK FEP3_CLOCK_SERVICE_CONFIG "/" FEP3_MAIN_CLOCK_PROPERTY

/**
* @brief Step size of the built-in discrete simulation time clock in nanoseconds.
*
*/
#define FEP3_CLOCK_SIM_TIME_STEP_SIZE_PROPERTY "step_size"

/**
* @brief Step size of the built-in discrete simulation time clock in nanoseconds.
*
*/
#define FEP3_CLOCK_SERVICE_CLOCK_SIM_TIME_STEP_SIZE FEP3_CLOCK_SERVICE_CONFIG "/" FEP3_CLOCK_SIM_TIME_STEP_SIZE_PROPERTY
/**
* @brief Minimum value of the built-in 'discrete simulation time clock' step size property in nanoseconds.
*
*/
#define FEP3_CLOCK_SIM_TIME_STEP_SIZE_MIN_VALUE 1
/**
* @brief Maximum value of the built-in 'discrete simulation time clock' step size property in nanoseconds.
* The value is chosen to allow 1000 simulation steps before reaching the hard limit of int64_t.
*
*/
#define FEP3_CLOCK_SIM_TIME_STEP_SIZE_MAX_VALUE fep3::Duration::max().count() / 1000
/**
* @brief Default value of the built-in 'discrete simulation time clock' step size property in nanoseconds.
*
*/
#define FEP3_CLOCK_SIM_TIME_STEP_SIZE_DEFAULT_VALUE 100000000

/**
* @brief Factor at which discrete time steps of the built-in discrete simulation time clock pass compared to the system time.
* A time factor of 2 means the discrete time step passes twice as fast compared to the system time.
* A time factor of 0,0 means no delay exists between discrete time steps.
*
*/
#define FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY "time_factor"

/**
* @brief Factor at which discrete time steps of the built-in discrete simulation time clock pass compared to the system time.
* A time factor of 2 means the discrete time step passes twice as fast compared to the system time.
* A time factor of 0,0 means no delay exists between discrete time steps.
*
*/
#define FEP3_CLOCK_SERVICE_CLOCK_SIM_TIME_TIME_FACTOR FEP3_CLOCK_SERVICE_CONFIG "/" FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY
/**
* @brief Value to configure the built-in 'discrete simulation time clock' time factor property to run in
* 'As Fast As Possible' mode.
*
*/
#define FEP3_CLOCK_SIM_TIME_TIME_FACTOR_AFAP_VALUE 0.0
/**
* @brief Default value of the built-in 'discrete simulation time clock' time factor property.
*
*/
#define FEP3_CLOCK_SIM_TIME_TIME_FACTOR_DEFAULT_VALUE 1.0
/**
* @brief Minimum value of the built-in 'discrete simulation time clock' wall clock step size in nanoseconds.
* The wall clock step size takes into account the @ref FEP3_CLOCK_SERVICE_CLOCK_SIM_TIME_STEP_SIZE and the
* @ref FEP3_CLOCK_SERVICE_CLOCK_SIM_TIME_TIME_FACTOR. The configured step size may not be lower than this
* value after applying the time factor.
*
*/
#define FEP3_CLOCK_WALL_CLOCK_STEP_SIZE_MIN_VALUE 1
/**
* @brief Minimum value of the built-in 'discrete simulation time clock' wall clock step size in nanoseconds.
* The wall clock step size takes into account the @ref FEP3_CLOCK_SERVICE_CLOCK_SIM_TIME_STEP_SIZE and the
* @ref FEP3_CLOCK_SERVICE_CLOCK_SIM_TIME_TIME_FACTOR. The configured step size may not be lower than this
* value after applying the time factor.
* The value is chosen to allow 1000 simulation steps before reaching the hard limit of int64_t.
*
*/
#define FEP3_CLOCK_WALL_CLOCK_STEP_SIZE_MAX_VALUE fep3::Duration::max().count() / 1000
/**
* @brief Name of the clock service built-in clock to retrieve the current system time (continuous clock).
* @see @ref FEP3_CLOCK_SERVICE_MAIN_CLOCK
*/
#define FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME "local_system_realtime"

/**
* @brief Name of the clock service built-in clock to retrieve a simulated time (discrete clock).
* The discrete clock may be configured using @ref FEP3_CLOCK_SIM_TIME_STEP_SIZE_PROPERTY and @ref FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY.
* @see @ref FEP3_CLOCK_SERVICE_MAIN_CLOCK
*
*/
#define FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME "local_system_simtime"

/**
* @brief Timeout for sending time update events like timeUpdating, timeReset, etc. to timing slaves.
* The timeout is applied per slave and update function.
*
*/
#define FEP3_TIME_UPDATE_TIMEOUT_PROPERTY "time_update_timeout"
/**
* @brief Timeout for sending time update events like timeUpdating, timeReset, etc. to timing slaves.
* The timeout is applied per slave and update function.
*
*/
#define FEP3_CLOCK_SERVICE_TIME_UPDATE_TIMEOUT FEP3_CLOCK_SERVICE_CONFIG "/" FEP3_TIME_UPDATE_TIMEOUT_PROPERTY
/**
* @brief Default value of the timeout for sending time update events in namespace in nanoseconds.
*
*/
#define FEP3_TIME_UPDATE_TIMEOUT_DEFAULT_VALUE 5000000000

namespace fep3
{
namespace arya
{

/**
* @brief Interface of the clock service
*
* The clock service may be used to register custom clocks and set the active main clock for the participant.
*/
class IClockService : public arya::IClockRegistry
{
public:
    /**
     * @brief Defintion of the clock service component IID.
     */
    FEP_COMPONENT_IID("clock_service.arya.fep3.iid")

protected:
    /// DTOR
    ~IClockService() = default;

public:
    /**
     * @brief Return the time of the current main clock.
     *
     * @return Current time of the main clock
     */
    virtual arya::Timestamp getTime() const = 0;

    /**
     * @brief Return the time of the clock with name @p clock_name.
     * @see @ref IClockRegistry::registerClock, @ref IClock::getName
     *
     * @param[in] clock_name The name of the clock
     * @return The current time of the clock or no value if no clock with name @p clock_name exists
     */
    virtual arya::Optional<arya::Timestamp> getTime(const std::string& clock_name) const = 0;

    /**
     * @brief Return the clock type of the current main clock.
     *
     * @return The type of the clock
     */
    virtual arya::IClock::ClockType getType() const = 0;

    /**
     * @brief Return the type of the clock with the name @p clock_name.
     *
     * @param[in] clock_name The name of the clock
     * @return The type of the clock or no value if no clock with @p clock_name exists
     */
    virtual arya::Optional<arya::IClock::ClockType> getType(const std::string& clock_name) const = 0;

    /**
     * @brief Get the name of the current main clock.
     *
     * @return Before the component is tensed the value of main clock property node, otherwise the name of the current main clock
     */
    virtual std::string getMainClockName() const= 0;

    /**
     * @brief Register an event sink to receive time events.
     *
     * @param[in] clock_event_sink The event sink to register
     * @return fep3::Result
     * @retval ERR_POINTER        The @p clock_event_sink weak_ptr is expired.
     */
    virtual fep3::Result registerEventSink(const std::weak_ptr<arya::IClock::IEventSink>& clock_event_sink) = 0;

    /**
     * @brief Unregister the event sink.
     *
     * @param[in] clock_event_sink The event sink to unregister
     * @return fep3::Result
     * @retval ERR_POINTER        The @p clock_event_sink weak_ptr is expired.
     */
    virtual fep3::Result unregisterEventSink(const std::weak_ptr<arya::IClock::IEventSink>& clock_event_sink) = 0;
};

} // namespace arya
using arya::IClockService;
} // namespace fep3
