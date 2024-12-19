/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/base/component.h>
#include <fep3/components/clock_sync/clock_sync_service_intf.h>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace arya {

class ClockSyncService : public fep3::base::arya::Component<fep3::arya::IClockSyncService> {
public:
};

} // namespace arya
using arya::ClockSyncService;
} // namespace mock
} // namespace fep3

///@endcond nodoc