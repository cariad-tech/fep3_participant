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

#include <gmock/gmock.h>

#include <fep3/components/base/component.h>
#include <fep3/components/data_registry/data_registry_intf.h>
#include <helper/gmock_destruction_helper.h>

// action popping an item
ACTION_P(Pop, shared_pointer_to_item)
{
    arg0.operator()(shared_pointer_to_item);
    return true; // receiver was invoked, so return true
}

namespace fep3
{
namespace mock
{
struct DataRegistryComponent : public base::Component<IDataRegistry>
{
    MOCK_METHOD3(registerDataIn, Result(const std::string&, const arya::IStreamType&, bool));
    MOCK_METHOD3(registerDataOut, Result(const std::string&, const arya::IStreamType&, bool));
    MOCK_METHOD1(unregisterDataIn, Result(const std::string&));
    MOCK_METHOD1(unregisterDataOut, Result(const std::string&));
    MOCK_METHOD2(registerDataReceiveListener, Result(const std::string&, const std::shared_ptr<IDataReceiver>&));
    MOCK_METHOD2(unregisterDataReceiveListener, Result(const std::string&, const std::shared_ptr<IDataReceiver>&));

    virtual std::unique_ptr<IDataRegistry::IDataReader> getReader(const std::string& name) override
    {
        return std::unique_ptr<IDataRegistry::IDataReader>(getReaderProxy(name));
    }
    MOCK_METHOD1(getReaderProxy, IDataRegistry::IDataReader*(const std::string&));

    virtual std::unique_ptr<IDataRegistry::IDataReader> getReader(const std::string& name, size_t queue_capacity) override
    {
        return std::unique_ptr<IDataRegistry::IDataReader>(getReaderProxy(name, queue_capacity));
    }
    MOCK_METHOD2(getReaderProxy, IDataRegistry::IDataReader*(const std::string&, size_t));

    virtual std::unique_ptr<IDataRegistry::IDataWriter> getWriter(const std::string& name) override
    {
        return std::unique_ptr<IDataRegistry::IDataWriter>(getWriterProxy(name));
    }
    MOCK_METHOD1(getWriterProxy, IDataRegistry::IDataWriter*(const std::string&));

    virtual std::unique_ptr<IDataRegistry::IDataWriter> getWriter(const std::string& name, size_t queue_capacity) override
    {
        return std::unique_ptr<IDataRegistry::IDataWriter>(getWriterProxy(name, queue_capacity));
    }
    MOCK_METHOD2(getWriterProxy, IDataRegistry::IDataWriter*(const std::string&, size_t));

    struct DataReader
        : public IDataRegistry::IDataReader
        , public test::helper::Dieable
    {
        MOCK_CONST_METHOD0(size, size_t());
        MOCK_CONST_METHOD0(capacity, size_t());
        MOCK_METHOD1(pop, a_util::result::Result(IDataRegistry::IDataReceiver&));
        MOCK_CONST_METHOD0(getFrontTime, fep3::arya::Optional<fep3::arya::Timestamp>());
    };

    struct DataWriter
        : public IDataRegistry::IDataWriter
        , public test::helper::Dieable
    {
        MOCK_METHOD1(write, a_util::result::Result(const fep3::arya::IStreamType&));
        MOCK_METHOD1(write, a_util::result::Result(const fep3::arya::IDataSample&));
        MOCK_METHOD0(flush, a_util::result::Result());
    };
};
}
}
