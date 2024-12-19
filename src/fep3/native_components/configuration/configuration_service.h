/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/base/properties/propertynode.h>
#include <fep3/components/base/component.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_service.h>
#include <fep3/rpc_services/configuration/configuration_rpc_intf_def.h>
#include <fep3/rpc_services/configuration/configuration_service_stub.h>

namespace fep3 {
namespace native {

class ConfigurationService : public fep3::base::Component<fep3::IConfigurationService> {
public:
    ConfigurationService();
    ~ConfigurationService() = default;

    fep3::Result create() override;
    fep3::Result destroy() override;

    // IConfigurationService
    fep3::Result registerNode(
        std::shared_ptr<fep3::arya::IPropertyNode> property_node) override final;
    fep3::Result unregisterNode(const std::string& name) override final;
    bool isNodeRegistered(const std::string& path) const override final;

    std::shared_ptr<fep3::arya::IPropertyNode> getNode(
        const std::string& path) const override final;
    std::shared_ptr<const fep3::arya::IPropertyNode> getConstNode(
        const std::string& path = "") const override final;

private:
    fep3::Result unregisterService(const IComponents& components);

private:
    std::shared_ptr<base::PropertyNode<IPropertyNode>> _root_node{nullptr};
    std::shared_ptr<IRPCServer::IRPCService> _rpc_service{nullptr};
};

class RPCConfigurationService : public rpc::RPCService<fep3::rpc_stubs::RPCConfigurationServiceStub,
                                                       fep3::rpc::IRPCConfigurationDef> {
public:
    explicit RPCConfigurationService(fep3::IConfigurationService& service) : _service(service)
    {
    }

    // Inherited via RPCService
    std::string getProperties(const std::string& property_path) override;
    std::string getAllProperties(const std::string& property_path) override;
    bool exists(const std::string& property_path) override;
    Json::Value getProperty(const std::string& property_path) override;
    int setProperty(const std::string& property_path,
                    const std::string& type,
                    const std::string& value) override;

private:
    fep3::IConfigurationService& _service;
};

} // namespace native
} // namespace fep3