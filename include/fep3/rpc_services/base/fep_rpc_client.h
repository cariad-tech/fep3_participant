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

// used because of FEP System Library API Exception
#ifndef _FEP3_RPC_CLIENT_HEADER_
#define _FEP3_RPC_CLIENT_HEADER_

// very important to have this relative! system library!
#include "fep_rpc_client_intf.h"

#include <memory>

///@cond nodoc
namespace fep3 {
namespace rpc {
namespace arya {

class IRPCClientPtr {
protected:
    ~IRPCClientPtr() = default;

public:
    virtual bool reset(const std::shared_ptr<arya::IRPCServiceClient>& other) = 0;
};

template <typename rpc_interface>
class RPCClient : public IRPCClientPtr {
private:
    std::shared_ptr<arya::IRPCServiceClient> _service_client;
    rpc_interface* _interface = nullptr;

public:
    RPCClient() = default;

    RPCClient(const RPCClient& oVal)
    {
        reset(oVal._service_client);
    }

    RPCClient& operator=(const RPCClient& oVal)
    {
        reset(oVal._service_client);
        return *this;
    }

    RPCClient(RPCClient&& oVal)
    {
        std::swap(_service_client, oVal._service_client);
        std::swap(_interface, oVal._interface);
    }

    RPCClient& operator=(RPCClient&& other)
    {
        std::swap(_service_client, other._service_client);
        std::swap(_interface, other._interface);
        return *this;
    }

    RPCClient(const std::shared_ptr<arya::IRPCServiceClient>& service_client)
    {
        reset(service_client);
    }

    RPCClient& operator=(const std::shared_ptr<arya::IRPCServiceClient>& service_client)
    {
        reset(service_client);
        return *this;
    }

    ~RPCClient()
    {
        reset();
    }

    explicit operator bool() const
    {
        return (_interface != nullptr);
    }

    rpc_interface& getInterface() const
    {
        return *_interface;
    }

    rpc_interface* operator->() const
    {
        return _interface;
    }

    std::shared_ptr<arya::IRPCServiceClient> getServiceClient() const
    {
        return _service_client;
    }

    bool reset(const std::shared_ptr<arya::IRPCServiceClient>& service_client) override
    {
        if (service_client) {
            std::string iid;
            iid = service_client->getRPCServiceIID();
            if (iid == getRPCIID<rpc_interface>()) {
                _interface = dynamic_cast<rpc_interface*>(service_client.get());
                if (_interface == nullptr) {
                    return false;
                }
                _service_client = service_client;
            }
            else {
                reset();
                return false;
            }
        }
        else {
            reset();
        }
        return true;
    }

    void reset()
    {
        _interface = nullptr;
        _service_client.reset();
    }
};

} // namespace arya
using arya::RPCClient;
} // namespace rpc
} // namespace fep3
///@endcond nodoc

#endif //_FEP3_RPC_CLIENT_HEADER_
