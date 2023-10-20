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

#include <fep3/components/configuration/propertynode_intf.h>

#include <gmock/gmock.h>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace arya {

struct PropertyNode : public fep3::arya::IPropertyNode {
    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(std::string, getValue, (), (const, override));
    MOCK_METHOD(std::string, getTypeName, (), (const, override));
    // mocking a method with default parameter requires a helper
    MOCK_METHOD(Result, setValueImpl, (const std::string&, const std::string));
    virtual Result setValue(const std::string& value, const std::string& type_name = "")
    {
        return setValueImpl(value, type_name);
    }
    MOCK_METHOD(bool, isEqual, (const IPropertyNode&), (const, override));
    MOCK_METHOD(void, reset, (), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<IPropertyNode>>, getChildren, (), (const, override));
    MOCK_METHOD(size_t, getNumberOfChildren, (), (const, override));
    MOCK_METHOD(std::shared_ptr<IPropertyNode>, getChild, (const std::string&), (const, override));
    MOCK_METHOD(bool, isChild, (const std::string&), (const, override));
};

} // namespace arya
using arya::PropertyNode;
} // namespace mock
} // namespace fep3

///@endcond nodoc