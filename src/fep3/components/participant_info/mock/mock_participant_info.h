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

#include <fep3/components/participant_info/participant_info_intf.h>
#include <fep3/components/base/component.h>
#include <fep3/components/base/c_access_wrapper/transferable_component_base.h>

namespace fep3
{
namespace mock
{

template<template<typename...> class component_base_type = fep3::base::Component>
struct ParticipantInfo
    : public component_base_type<IParticipantInfo>
{
    MOCK_CONST_METHOD0(getName, std::string());
    MOCK_CONST_METHOD0(getSystemName, std::string());
};

class TransferableParticipantInfo
    : public ParticipantInfo<fep3::plugin::c::TransferableComponentBase>
{
};

}
}
