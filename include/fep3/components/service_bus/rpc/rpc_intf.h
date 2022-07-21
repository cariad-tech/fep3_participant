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
#include <string>

#include <fep3/fep3_participant_types.h>
#include <fep3/fep3_errors.h>
#include <memory>

namespace fep3
{
namespace arya
{
/**
 * @brief Instance of one requester to send a client request to a server.
 *
 */
class IRPCRequester
{
public:
    /// Interface of an RPC response
    class IRPCResponse
    {
    protected:
        /// DTOR
        ~IRPCResponse() = default;

    public:
        /**
        * \c Sets the response message.
        *
        * @param[in] response The response data.
        *
        * @retval ERR_NOERROR Everything went as expected.
        */
        virtual fep3::Result set(const std::string& response) = 0;
    };

protected:
    /// DTOR
    ~IRPCRequester() = default;

public:
    /**
     * @brief send a request to the server and will wait for an answer
     * @param[in] service_name the name of the service to reach
     * @param[in] request_message the request message (serialized!)
     * @param[in] response_callback the response callback interface to set the response to
     * @return returns an error code
     * @retval ERR_TIMEOUT Timeout reached with no answer
     * @retval ERR_IO_ERROR server address not reachable or connection closed
     * @retval ERR_NOERROR response_callback will contain a valid response message
     *
     * @remark the \p request_message content must be serialized already (usually i.e. a json-string)
     */
    virtual fep3::Result sendRequest(const std::string& service_name,
                                     const std::string& request_message,
                                     IRPCResponse& response_callback) const = 0;
};

/**
 * @brief one server access point
 *
 */
class IRPCServer
{
public:
    /**
     * @brief Service (a part of a server with a dedicated functionality)
     *
     */
    class IRPCService
    {
    protected:
        /// DTOR
        ~IRPCService() = default;

    public:
        /**
         * get the service id
         * @retval service ID.
         */
        virtual std::string getRPCServiceIIDs() const = 0;

        /**
         * get the interface definition
         * @retval interface definition
         */
        virtual std::string getRPCInterfaceDefinition() const = 0;

        /**
         * \c Handles a request send to this server.
         *
         * @param[in]  content_type The content type of the request.
         * @param[in]  request_message The request message.
         * @param[out] response_message The response message.
         *
         * @retval ERR_NOERROR Everything went as expected.
         * @retval ERR_INVALID_ARG Invalid arguments for the message.
         * @retval ERR_UNEXPECTED Something else went wrong.
         */
        virtual fep3::Result handleRequest(const std::string& content_type,
                                           const std::string& request_message,
                                           arya::IRPCRequester::IRPCResponse& response_message) = 0;
    };

protected:
    /// DTOR
    ~IRPCServer() = default;

public:
    /**
     * @brief Get the url of the server
     *
     * @return std::string the url
     */
    virtual std::string getUrl() const = 0;
    /**
     * @brief Get the name
     *
     * @return std::string the name
     */
    virtual std::string getName() const = 0;
    /**
     * @brief Sets the name
     *
     * @param[in] name the name
     */
    virtual void setName(const std::string& name) = 0;

    /**
     * \c Registers an RPC server with a unique \c service_name
     *
     * @param[in] service_name The name of the server. \c oRPCObjectServerInstance name is updated.
     * @param[in,out] service The server that is registered.
     * \note The server instance must be available as long as the server is registered.
     *
     * @retval ERR_INVALID_ARG If the name already exists
     * @retval ERR_NOERROR Everything went as expected.
     */
    virtual fep3::Result registerService(const std::string& service_name,
                                         const std::shared_ptr<IRPCService>& service) = 0;

    /**
     * \c Unregisters a server by its \c service_name.
     *
     * @param[in] service_name The name of the server.
     * \note Only after the server is unregistered it may be deleted.
     *
     * @retval ERR_NOT_FOUND If the server is not registered.
     * @retval ERR_NOERROR Everything went as expected.
     */
    virtual fep3::Result unregisterService(const std::string& service_name) = 0;

    /**
     * Definition that is used within implementations of IRPCService to identify
     * that the default url can be used.
     */
    static constexpr const char* const _use_default_url = "use_default_url";
};
}
using arya::IRPCRequester;
using arya::IRPCServer;
} //namespace fep3
