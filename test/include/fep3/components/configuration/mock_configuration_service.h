/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/base/component.h>
#include <fep3/components/configuration/configuration_service_intf.h>

#include <gmock/gmock.h>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace arya {
namespace detail {

template <template <typename...> class component_base_type>
struct ConfigurationService : public component_base_type<fep3::arya::IConfigurationService> {
    MOCK_METHOD(Result, registerNode, (std::shared_ptr<fep3::arya::IPropertyNode>), (override));
    MOCK_METHOD(Result, unregisterNode, (const std::string&), (override));
    MOCK_METHOD(bool, isNodeRegistered, (const std::string&), (const, override));
    MOCK_METHOD(std::shared_ptr<fep3::arya::IPropertyNode>,
                getNode,
                (const std::string&),
                (const, override));
    MOCK_METHOD(std::shared_ptr<const fep3::arya::IPropertyNode>,
                getConstNode,
                (const std::string&),
                (const, override));
};

} // namespace detail

using ConfigurationService = detail::ConfigurationService<fep3::base::arya::Component>;

} // namespace arya
using arya::ConfigurationService;
} // namespace mock
} // namespace fep3

///@endcond nodoc