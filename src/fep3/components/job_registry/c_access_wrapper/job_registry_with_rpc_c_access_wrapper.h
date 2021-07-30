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

#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_service.h>
#include <fep3/rpc_services/job_registry/job_registry_rpc_intf_def.h>
#include <fep3/rpc_services/job_registry/job_registry_service_stub.h>
#include <fep3/components/job_registry/c_access_wrapper/job_registry_c_access_wrapper.h>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace access
{
namespace arya
{

/**
 * @brief Class decorating the access class for Job Registry by an RPC service.
 */
class JobRegistryWithRPC
    : public JobRegistry
{
private:
    class RPCJobRegistry : public rpc::arya::RPCService<rpc_stubs::RPCJobRegistryServiceStub, rpc::arya::IRPCJobRegistryDef>
    {
    public:
        explicit RPCJobRegistry(fep3::arya::IJobRegistry& job_registry)
            : _job_registry(job_registry)
        {}

    protected:
        std::string getJobNames() override;
        Json::Value getJobInfo(const std::string& job_name) override;

    private:
        fep3::arya::IJobRegistry& _job_registry;
    };
public:
    /**
     * CTOR
     *
     * @param[in] access Access to the remote object
     * @param[in] shared_binary Shared pointer to the binary this resides in
     */
    inline JobRegistryWithRPC
        (const Access& access
        , const std::shared_ptr<ISharedBinary>& shared_binary
        );
    /// DTOR
    inline ~JobRegistryWithRPC() override = default;

    // methods overriding fep3::arya::ComponentBase
    /// @cond no_documentation
    fep3::Result create() override;
    /// @endcond no_documentation
};

JobRegistryWithRPC::JobRegistryWithRPC
    (const Access& access
    , const std::shared_ptr<ISharedBinary>& shared_binary
    )
    : JobRegistry(access, shared_binary)
{}

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3