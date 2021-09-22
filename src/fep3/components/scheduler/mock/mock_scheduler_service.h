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

#include <fep3/components/scheduler/scheduler_service_intf.h>
#include <fep3/components/base/component.h>

namespace fep3
{
namespace mock
{

template<template<typename...> class component_base_type = fep3::base::Component>
struct SchedulerService
    : public component_base_type<ISchedulerService>
{
    MOCK_METHOD1(setActiveScheduler, fep3::Result(const std::string&));
    MOCK_CONST_METHOD0(getActiveSchedulerName, std::string());
    // with current version of gMock in the aev_testing package, move-only types are not supported with MSVC
    // , so we need workarounds as suggested here:
    // https://github.com/google/googletest/blob/master/googlemock/docs/cook_book.md#legacy-workarounds-for-move-only-types-legacymoveonly
    MOCK_METHOD1(registerScheduler_, fep3::Result(IScheduler*));
    fep3::Result registerScheduler(std::unique_ptr<IScheduler> scheduler) override
    {
        return registerScheduler_(scheduler.release());
    }
    MOCK_METHOD1(unregisterScheduler, fep3::Result(const std::string&));
    MOCK_CONST_METHOD0(getSchedulerNames, std::list<std::string>());

};

}
}
