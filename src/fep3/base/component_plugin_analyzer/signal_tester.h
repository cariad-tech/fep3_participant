/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <fep3/base/component_plugin_analyzer/component_description_base.h>
#include <fep3/components/base/components_intf.h>
#include <fep3/components/data_registry/data_registry_intf.h>

#include <map>
#include <string>
#include <vector>

namespace fep3::base {
class DataRegistryRpcClient;

class SignalTester {
public:
    SignalTester(fep3::arya::IComponents& components);
    ~SignalTester();
    void checkSignalsBefore();

    void checkSignalsAfter(const std::string& comp_iid);

    std::vector<SignalInfo> getComponentSignals(const std::vector<std::string>& comp_iid);

    std::map<std::string, std::vector<SignalInfo>> getSignalInfos() const;

private:
    fep3::IDataRegistry* _data_registry;
    std::map<std::string, std::vector<SignalInfo>> _signal_infos;
    std::vector<SignalInfo> _signals_before;
    std::unique_ptr<DataRegistryRpcClient> _rpc_client;
};
} // namespace fep3::base
