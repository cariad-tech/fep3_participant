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

#include <fep3/components/base/component.h>
#include <fep3/components/data_registry/data_registry_intf.h>

#include <gmock/gmock.h>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace arya {

/**
 * @brief Action popping an item from a data reader
 */
ACTION_P(Pop, shared_pointer_to_item)
{
    arg0.operator()(shared_pointer_to_item);
    return true; // receiver was invoked, so return true
}

struct DataRegistry : public fep3::base::arya::Component<fep3::arya::IDataRegistry> {
    MOCK_METHOD(Result,
                registerDataIn,
                (const std::string&, const fep3::arya::IStreamType&, bool),
                (override));
    MOCK_METHOD(Result,
                registerDataOut,
                (const std::string&, const fep3::arya::IStreamType&, bool),
                (override));
    MOCK_METHOD(Result, unregisterDataIn, (const std::string&), (override));
    MOCK_METHOD(Result, unregisterDataOut, (const std::string&), (override));
    MOCK_METHOD(Result,
                registerDataReceiveListener,
                (const std::string&, const std::shared_ptr<IDataReceiver>&),
                (override));
    MOCK_METHOD(Result,
                unregisterDataReceiveListener,
                (const std::string&, const std::shared_ptr<IDataReceiver>&),
                (override));

    MOCK_METHOD(std::unique_ptr<IDataRegistry::IDataReader>,
                getReader,
                (const std::string&),
                (override));
    MOCK_METHOD(std::unique_ptr<IDataRegistry::IDataReader>,
                getReader,
                (const std::string&, size_t),
                (override));
    MOCK_METHOD(std::unique_ptr<IDataRegistry::IDataWriter>,
                getWriter,
                (const std::string&),
                (override));
    MOCK_METHOD(std::unique_ptr<IDataRegistry::IDataWriter>,
                getWriter,
                (const std::string&, size_t),
                (override));

    struct DataReader : public IDataReader {
        MOCK_METHOD(size_t, size, (), (const, override));
        MOCK_METHOD(size_t, capacity, (), (const, override));
        MOCK_METHOD(a_util::result::Result, pop, (IDataRegistry::IDataReceiver&), (override));
        MOCK_METHOD(fep3::arya::Optional<fep3::arya::Timestamp>,
                    getFrontTime,
                    (),
                    (const, override));
    };

    struct DataWriter : public IDataWriter {
        MOCK_METHOD(a_util::result::Result, write, (const fep3::arya::IStreamType&), (override));
        MOCK_METHOD(a_util::result::Result, write, (const fep3::arya::IDataSample&), (override));
        MOCK_METHOD(a_util::result::Result, flush, (), (override));
    };
};

} // namespace arya
using arya::DataRegistry;
} // namespace mock
} // namespace fep3

///@endcond nodoc
