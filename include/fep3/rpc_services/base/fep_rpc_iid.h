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
#ifndef _FEP3_RPC_DEFINITION_H_INCLUDED_
#define _FEP3_RPC_DEFINITION_H_INCLUDED_

/**
 * @c Macro to help define a RPC server interface.
 *
 * Usage: Define the server interface which should not contain anything
 *        besides this macro in a public scope. E.g.:
 *        class IMyInterface {
 *        public:
 *            FEP_RPC_IID("iid.server", "server name");
 *        };
 *
 * @param[in] iid The server ID that must be unique.
 * @param[in] defaultname The default name of the server.
 */
#define FEP_RPC_IID(iid, defaultname)                                                              \
    static constexpr const char* const DEFAULT_NAME = defaultname;                                 \
    static const char* getRPCDefaultName()                                                         \
    {                                                                                              \
        return defaultname;                                                                        \
    }                                                                                              \
    static constexpr const char* const RPC_IID = iid;                                              \
    static const char* getRPCIID()                                                                 \
    {                                                                                              \
        return iid;                                                                                \
    }

/**
 * @c Macro to help define a RPC server interface.
 *
 * Usage: Define the server interface which should not contain anything
 *        besides this macro in a public scope. E.g.:
 *        class IMyInterface {
 *        public:
 *            FEP3_RPC_IID("iid.server", "server name");
 *        };
 *
 * @param[in] iid The server ID that must be unique.
 * @param[in] defaultname The default name of the server.
 */
#define FEP3_RPC_IID(iid, defaultname) FEP_RPC_IID(iid, defaultname)

#endif //_FEP3_RPC_DEFINITION_H_INCLUDED_
