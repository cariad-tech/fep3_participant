/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "dds_service_finder.h"

namespace fep3 {
namespace native {
template <typename T>
struct ConversionFunction {
};

template <>
struct ConversionFunction<fep3::native::ServiceFinderDDS> {
    using UpdateEvent = fep3::native::ServiceUpdateEvent;
    static fep3::native::ServiceUpdateEvent convertEvent(const UpdateEvent& update_event)
    {
        return update_event;
    }
};

} // namespace native
} // namespace fep3
