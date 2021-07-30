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

#include <fep3/components/service_bus/rpc/rpc_intf.h>

namespace fep3
{
namespace rti_dds
{

class DDSRequester : public fep3::arya::IRPCRequester
{
    public:
        explicit DDSRequester(const std::string& server_address);
        virtual ~DDSRequester();
        fep3::Result sendRequest(const std::string& service_name,
                                 const std::string& request_message,
                                 fep3::arya::IRPCRequester::IRPCResponse& response_callback) const override;

    private:
        std::string _server_address;
};

}
}
