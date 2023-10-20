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
#include <fep3/components/participant_info/participant_info_intf.h>

#include <gmock/gmock.h>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace arya {
namespace detail {

template <template <typename...> class component_base_type>
struct ParticipantInfo : public component_base_type<fep3::arya::IParticipantInfo> {
    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(std::string, getSystemName, (), (const, override));
};

} // namespace detail

using ParticipantInfo = detail::ParticipantInfo<fep3::base::arya::Component>;

} // namespace arya
using arya::ParticipantInfo;
} // namespace mock
} // namespace fep3

///@endcond nodoc