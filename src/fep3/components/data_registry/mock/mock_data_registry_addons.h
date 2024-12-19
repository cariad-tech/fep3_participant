/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/data_registry/mock_data_registry.h>

#include <helper/gmock_destruction_helper.h>

namespace fep3 {
namespace mock {

class DataRegistryAddons : public fep3::mock::arya::DataRegistry {
public:
    class DataReader : public fep3::mock::arya::DataRegistry::DataReader,
                       public test::helper::Dieable {
    };

    struct DataWriter : public fep3::mock::arya::DataRegistry::DataWriter,
                        public test::helper::Dieable {
    };
};

} // namespace mock
} // namespace fep3