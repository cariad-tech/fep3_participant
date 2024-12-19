/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "signal_tester.h"

#include "data_registry_rpc_client.h"

#include <fep3/components/service_bus/service_bus_intf.h>

#include <algorithm>

namespace {
const std::string test_participant_name = "test_participant_name";
} // namespace

namespace fep3::base {
bool operator==(const SignalInfo& lhs, const SignalInfo& rhs)
{
    return lhs._name == rhs._name;
}

bool operator<(const SignalInfo& lhs, const SignalInfo& rhs)
{
    return lhs._name < rhs._name;
}

SignalTester::SignalTester(fep3::arya::IComponents& components)
{
    _data_registry = components.getComponent<fep3::IDataRegistry>();
    assert(_data_registry);

    auto service_bus = components.getComponent<fep3::IServiceBus>();
    assert(_data_registry);
    auto requester = service_bus->getRequester("test_participant_name");
    _rpc_client = std::make_unique<DataRegistryRpcClient>(
        fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName(), requester);
}

SignalTester::~SignalTester() = default;

void SignalTester::checkSignalsBefore()
{
    _signals_before = _rpc_client->getRegisteredSignals();
}

void SignalTester::checkSignalsAfter(const std::string& comp_iid)
{
    auto signals_after = _rpc_client->getRegisteredSignals();
    std::vector<SignalInfo> new_signals;

    std::sort(signals_after.begin(), signals_after.end());
    std::sort(_signals_before.begin(), _signals_before.end());

    std::set_difference(signals_after.begin(),
                        signals_after.end(),
                        _signals_before.begin(),
                        _signals_before.end(),
                        std::back_inserter(new_signals));

    _signal_infos[comp_iid].insert(
        _signal_infos[comp_iid].end(), new_signals.begin(), new_signals.end());
}

std::vector<SignalInfo> SignalTester::getComponentSignals(const std::vector<std::string>& comp_iids)
{
    std::vector<SignalInfo> ret;
    for (const auto& comp_iid: comp_iids) {
        if (_signal_infos.count(comp_iid) > 0)
            ret.insert(
                ret.end(), _signal_infos.at(comp_iid).begin(), _signal_infos.at(comp_iid).end());
    }
    return ret;
}

std::map<std::string, std::vector<SignalInfo>> SignalTester::getSignalInfos() const
{
    return _signal_infos;
}
} // namespace fep3::base
