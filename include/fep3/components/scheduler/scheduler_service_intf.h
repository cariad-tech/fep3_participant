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

#include <fep3/components/scheduler/scheduler_registry_intf.h>

/**
 * @brief Main property entry of the scheduling properties
 */
#define FEP3_SCHEDULER_SERVICE_CONFIG "scheduling"

/**
 * @brief The scheduler configuration property name
 * Use this to set the scheduler by configuration.
 */
#define FEP3_SCHEDULER_PROPERTY "scheduler"

/**
 * @brief The scheduler configuration property path to set up the scheduler to use
 */
#define FEP3_SCHEDULER_SERVICE_SCHEDULER FEP3_SCHEDULER_SERVICE_CONFIG "/" FEP3_SCHEDULER_PROPERTY

namespace fep3 {
namespace arya {

/**
 * @brief Interface for the SchedulerService
 */
class ISchedulerService : public arya::ISchedulerRegistry {
public:
    /// The component interface identifier of ISchedulerService
    FEP_COMPONENT_IID("scheduler_service.arya.fep3.iid");

protected:
    /// DTOR
    ~ISchedulerService() = default;

public:
    /**
     * @brief Returns the name of the active scheduler.
     *
     * @return Name of active scheduler
     */
    virtual std::string getActiveSchedulerName() const = 0;
};
} // namespace arya

namespace catelyn {
/**
 * @brief Interface for the SchedulerService
 */
class ISchedulerService : virtual public arya::ISchedulerService,
                          public catelyn::ISchedulerRegistry {
public:
    /** @brief use member function of  arya::ISchedulerService */
    using fep3::arya::ISchedulerService::registerScheduler;
    /** @brief use member function of  catelyn::ISchedulerService */
    using fep3::catelyn::ISchedulerRegistry::registerScheduler;
    /// The component interface identifier of ISchedulerService
    FEP_COMPONENT_IID("scheduler_service.catelyn.fep3.iid");
};
} // namespace catelyn
using catelyn::ISchedulerService;
} // namespace fep3
