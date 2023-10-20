/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2023 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */
#pragma once

#include <fep3/core/data_io_container_intf.h>

#include <gmock/gmock.h>

namespace fep3 {
namespace mock {

class DataIOContainer : public fep3::core::IDataIOContainer {
public:
    MOCK_METHOD(fep3::core::DataReader*,
                addDataIn,
                (const std::string&,
                 const fep3::arya::IStreamType&,
                 size_t,
                 const fep3::catelyn::JobConfiguration&));

    MOCK_METHOD(fep3::core::DataWriter*,
                addDataOut,
                (const std::string&, const fep3::arya::IStreamType&, size_t));

    MOCK_METHOD(fep3::core::DataWriter*,
                addDynamicDataOut,
                (const std::string&, const fep3::arya::IStreamType&));

    MOCK_METHOD(fep3::Result, executeDataIn, (fep3::Timestamp));
    MOCK_METHOD(fep3::Result, executeDataOut, (fep3::Timestamp));
    MOCK_METHOD(fep3::Result,
                addToDataRegistry,
                (fep3::arya::IDataRegistry&, fep3::arya::IClockService&));
    MOCK_METHOD(void, removeFromDataRegistry, (fep3::arya::IDataRegistry&));
};

} // namespace mock
} // namespace fep3
