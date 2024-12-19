/**
 * Copyright 2024 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <fep3/components/service_bus/rpc/fep_rpc.h>

#include <rpc/json_rpc.h>

#include <boost/thread/latch.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <boost_http_factory.h>
#include <http_server.h>
#include <testclientstub.h>
#include <testserverstub.h>

using namespace fep3;
using namespace ::testing;

namespace {
constexpr size_t max_message_size = 65536;

/** borrowed from:
 * @author [Ates Goral](https://stackoverflow.com/users/23501/ates-goral) here:
 *         https://stackoverflow.com/a/440240
 * @copyright [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/)
 *            (See: [What is the license for the content I post?]
 *            (https://stackoverflow.com/help/licensing))
 * @see Comment section of this SO answer: https://stackoverflow.com/a/440240
 */
std::string randomString(size_t length)
{
    auto randchar = []() -> char {
        const char charset[] = "0123456789"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

} // namespace

class RPCServiceBase : public ::rpc::IRPCObject {
public:
    RPCServiceBase(const std::string& request, const char* strResponse, const size_t nResponseSize)
        : _request(request), _strResponse(strResponse), _nResponseSize(nResponseSize)
    {
    }

    virtual ~RPCServiceBase() = default;

    a_util::result::Result HandleCall(const char* strRequest,
                                      size_t nRequestSize,
                                      ::rpc::IResponse& oResponse) override
    {
        if ((!strRequest) || (!nRequestSize)) {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG, "Invalid request");
        }

        if (std::string req(strRequest); _request.compare(req) == 0) {
            oResponse.Set(_strResponse, _nResponseSize);
            return {};
        }
        RETURN_ERROR_DESCRIPTION(ERR_NOT_IMPL, "Request not implemented");
    };

private:
    const std::string& _request;
    const char* _strResponse;
    const size_t _nResponseSize;
};

class RPCService : public RPCServiceBase {
public:
    RPCService(const std::string& request, const std::string& response)
        : RPCServiceBase(request, response.c_str(), response.length())
    {
    }
};

class RPCServiceBlocked : public ::rpc::IRPCObject {
public:
    struct Configuration {
        std::string request;
        std::string response;
        std::reference_wrapper<boost::latch> request_wait;
    };

    explicit RPCServiceBlocked(const std::vector<Configuration>& config) : _config(config)
    {
    }

    virtual ~RPCServiceBlocked() = default;

    a_util::result::Result HandleCall(const char* strRequest,
                                      size_t nRequestSize,
                                      ::rpc::IResponse& oResponse) override
    {
        if ((!strRequest) || (!nRequestSize)) {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG, "Invalid request");
        }

        std::string req(strRequest);
        // find request and send response
        if (auto it =
                std::find_if(_config.begin(),
                             _config.end(),
                             [&](const Configuration& r) { return req.compare(r.request) == 0; });
            it != _config.end()) {
            // block server request until latch is 0
            it->request_wait.get().wait();
            oResponse.Set(it->response.c_str(), it->response.length());
            return {};
        }
        RETURN_ERROR_DESCRIPTION(ERR_NOT_IMPL, "Request not implemented");
    };

private:
    const std::vector<Configuration>& _config;
};

class RPCResponse : public IRPCRequester::IRPCResponse {
public:
    struct Configuration {
        std::string response;
        std::reference_wrapper<boost::latch> service_wait;
    };

    RPCResponse() = default;

    explicit RPCResponse(const std::vector<Configuration>& config) : _config(config)
    {
    }

    virtual ~RPCResponse() = default;

    fep3::Result set(const std::string& response) override
    {
        _response = response;
        // find latch to unlock server request
        if (auto it = std::find_if(
                _config.begin(),
                _config.end(),
                [&](const Configuration& r) { return r.response.compare(response) == 0; });
            it != _config.end()) {
            it->service_wait.get().count_down();
        }
        return {};
    }
    std::string& getResponse()
    {
        return _response;
    }

private:
    std::string _response;
    std::vector<Configuration> _config;
};

class ITestRPC {
public:
    FEP_RPC_IID("ITestRPC", "test_service");
};

class JsonRPCService
    : public fep3::rpc::RPCService<::test::rpc_stubs::TestInterfaceServer, ITestRPC> {
public:
    std::string GetObjects() override
    {
        return "OK";
    }
    std::string GetRPCIIDForObject(const std::string& strObject) override
    {
        return strObject;
    }
    int GetRunlevel() override
    {
        return _runLevel;
    }
    Json::Value SetRunlevel(int nRunLevel) override
    {
        _runLevel = nRunLevel;

        Json::Value val;
        val["ErrorCode"] = _runLevel;
        return val;
    }

private:
    int _runLevel{42};
};

class JsonRPCConnector : public ::jsonrpc::IClientConnector {
public:
    explicit JsonRPCConnector(const std::string& server_address, const std::string& service_name)
        : _service_name(service_name)
    {
        fep3::native::BoostHttpFactory factory;
        std::string url = server_address + "/" + service_name;
        _requester = factory.getRequester(url);
    }

    ~JsonRPCConnector() final = default;

    void SendRPCMessage(const std::string& message, std::string& result) override
    {
        RPCResponse response;
        _requester->sendRequest(_service_name, message, response);
        result = response.get();
    }

private:
    std::shared_ptr<fep3::arya::IRPCRequester> _requester;
    std::string _service_name;

    class RPCResponse : public IRPCRequester::IRPCResponse {
    public:
        virtual ~RPCResponse() = default;

        fep3::Result set(const std::string& response) override
        {
            _response = response;
            return {};
        }
        std::string& get()
        {
            return _response;
        }

    private:
        std::string _response;
    };
};

struct BoostRpcClientTestBase : public ::testing::Test {
protected:
    struct RPCResponseData {
        RPCResponse callback{};
        a_util::result::Result result{};
    };

    const std::string _service_name{"my_service"};
    std::string _server_address;
    std::string _request_str{"my_request"};
    std::string _response_str{"hello"};
    const std::string _participant_name{"test_participant_1"};
    const std::string _system_name{"test_async_http_system"};

    fep3::native::BoostHttpFactory _factory{};

    void startListeningOnFreePort(fep3::native::IRpcHttpServer& server)
    {
        const std::string localHost{"http://127.0.0.1:"};
        for (int port = 9090; port <= 10090; ++port) {
            auto url = localHost + std::to_string(port);
            auto res = server.startListening(url, 0);
            if (res) {
                _server_address = url;
                return;
            }
        }
        // NOSONAR
        throw std::runtime_error("Test cannot allocate a free port for the http server");
    }
};

struct BoostJsonRpcClientTest : public BoostRpcClientTestBase {
public:
    void SetUp() override
    {
        _server = _factory.createJsonRpcServer(_participant_name, _system_name, true);
        _server->registerRPCObject(_service_name, &_rpc_service);
        startListeningOnFreePort(*_server);

        _rpc_connector = std::make_unique<JsonRPCConnector>(_server_address, _service_name);
        _client = std::make_unique<test::rpc_stubs::TestInterfaceClient>(*_rpc_connector);
    }

protected:
    std::unique_ptr<JsonRPCConnector> _rpc_connector;
    fep3::native::HttpServer::RPCObjectToRPCServerWrapper _rpc_service =
        fep3::native::HttpServer::RPCObjectToRPCServerWrapper(std::make_shared<JsonRPCService>());
    std::unique_ptr<fep3::native::IRpcHttpServer> _server;
    std::unique_ptr<test::rpc_stubs::TestInterfaceClient> _client;
};

struct BoostRpcClientTest : public BoostRpcClientTestBase {
public:
    void SetUp() override
    {
        _rpc_service = std::make_shared<RPCService>(_request_str, _response_str);
        initialize();
    }

    void SetUp(const std::string& request, const char* strResponse, const size_t nResponseSize)
    {
        _rpc_service = std::make_shared<RPCServiceBase>(request, strResponse, nResponseSize);
        initialize();
    }

    void initialize()
    {
        _server = _factory.createJsonRpcServer(_participant_name, _system_name, true);
        _server->registerRPCObject(_service_name, _rpc_service.get());
        startListeningOnFreePort(*_server);

        _client = _factory.getRequester(_server_address);
    }

    void TearDown() override
    {
        _server.reset();
    }

protected:
    std::shared_ptr<RPCServiceBase> _rpc_service;
    std::unique_ptr<fep3::native::IRpcHttpServer> _server;
    std::shared_ptr<fep3::arya::IRPCRequester> _client;
    RPCResponseData _rpc_response{RPCResponseData()};
};

struct BoostRpcClientTestMTUReceive : public BoostRpcClientTest {
public:
    void SetUp() final
    {
        _response_str = randomString(max_message_size * 10);
        BoostRpcClientTest::SetUp();
    }
};

struct BoostRpcClientTestMTUSend : public BoostRpcClientTest {
public:
    void SetUp() final
    {
        _request_str = randomString(max_message_size * 10);
        BoostRpcClientTest::SetUp();
    }
};

struct BoostRpcClientTestInvalidResponse : public BoostRpcClientTest {
public:
    void SetUp() final
    {
        BoostRpcClientTest::SetUp(_request_str, _response_str.c_str(), 0);
    }
};

struct BoostRpcClientTestServiceBlockedBase : public BoostRpcClientTestBase {
public:
    void SetUp() override
    {
        _service_config.emplace_back(RPCServiceBlocked::Configuration{
            _request_str, _response_str, std::ref(_service_latch)});

        _server = _factory.createJsonRpcServer(_participant_name, _system_name, true);
        _server->registerRPCObject(_service_name, &_rpc_service_blocked);

        // start server thread for listening
        startListeningOnFreePort(*_server);
    }

    void TearDown() override
    {
        if (!_service_released) {
            releaseService();
        }
        _server.reset();
    }

    void releaseService()
    {
        _service_latch.count_down();
        _service_released = true;
    }

protected:
    std::vector<RPCServiceBlocked::Configuration> _service_config;
    RPCServiceBlocked _rpc_service_blocked{RPCServiceBlocked(_service_config)};
    std::unique_ptr<fep3::native::IRpcHttpServer> _server;
    bool _service_released{false};
    boost::latch _service_latch{1};
};

struct BoostRpcClientTestServiceBlocked : public BoostRpcClientTestServiceBlockedBase {
public:
    void SetUp() override
    {
        BoostRpcClientTestServiceBlockedBase::SetUp();
        // create boost http client context thread
        _client = _factory.getRequester(_server_address);
    }

protected:
    std::shared_ptr<fep3::arya::IRPCRequester> _client;
    RPCResponseData _rpc_response{};
};

struct BoostRpcAsyncClientTestServiceBlocked : public BoostRpcClientTestServiceBlockedBase {
public:
    void SetUp() override
    {
        BoostRpcClientTestServiceBlockedBase::SetUp();
        // create boost http client context thread
        _client = _factory.getAsyncRequester(_server_address);
    }

protected:
    std::shared_ptr<fep3::experimental::IRPCRequester> _client;
    std::pair<fep3::Result, std::string> _rpc_response{};
};

struct BoostRpcMultiClientTest : public BoostRpcClientTestBase {
public:
    struct ServerConfiguration {
        std::string service_name;
        std::shared_ptr<RPCService> rpc_service;
    };

    void SetUp() override
    {
        _server = _factory.createJsonRpcServer(_participant_name, _system_name, true);
    }

    void initialize(const int clients_numOf, const std::vector<ServerConfiguration>& server_config)
    {
        startListeningOnFreePort(*_server);
        _clients.resize(clients_numOf);

        std::generate(_clients.begin(), _clients.end(), [this]() {
            return _factory.getRequester(_server_address);
        });

        _rpc_responses.resize(clients_numOf);
        std::generate(
            _rpc_responses.begin(), _rpc_responses.end(), []() { return RPCResponseData(); });

        _server_config = server_config;
        for (const auto& cfg: _server_config) {
            _server->registerRPCObject(cfg.service_name, cfg.rpc_service.get());
        }
    }

    void TearDown() override
    {
        _server.reset();
    }

protected:
    std::vector<ServerConfiguration> _server_config;
    std::unique_ptr<fep3::native::IRpcHttpServer> _server;
    std::vector<std::shared_ptr<fep3::arya::IRPCRequester>> _clients;
    std::vector<RPCResponseData> _rpc_responses;
};

struct BoostRpcClientTestMultiThreaded : public BoostRpcClientTestBase {
public:
    void SetUp() override
    {
        // prepare server test data
        std::generate_n(
            std::back_inserter(_service_config), _requests_num_of, [this, i = 0]() mutable {
                RPCServiceBlocked::Configuration ret{_request_str + "_" + std::to_string(i),
                                                     _response_str + "_" + std::to_string(i),
                                                     _service_wait[i]};
                ++i;
                return ret;
            });

        // create client table to unlock server requests
        std::vector<RPCResponse::Configuration> response_config = {
            {_service_config.at(1).response,
             _service_config.at(0)
                 .request_wait}, // on client reception of request_1 --> unlock server request_0
            {_service_config.at(0).response,
             _service_config.at(2)
                 .request_wait} // on client reception of request_0 --> unlock server request_2
        };

        _rpc_responses.resize(_requests_num_of);
        std::generate(_rpc_responses.begin(), _rpc_responses.end(), [&response_config]() {
            RPCResponseData data = {RPCResponse(response_config), a_util::result::Result()};
            return data;
        });

        _server = _factory.createJsonRpcServer(_participant_name, _system_name, true);
        _server->registerRPCObject(_service_name, &_rpc_service_blocked);
        startListeningOnFreePort(*_server);
    }

    void TearDown() override
    {
        _server.reset();
    }

protected:
    static constexpr int _requests_num_of{6};

    std::vector<RPCResponseData> _rpc_responses;
    std::vector<RPCServiceBlocked::Configuration> _service_config;

    boost::barrier _bar{_requests_num_of}; // barrier to sync client sendRequests()
    std::array<boost::latch, _requests_num_of> _service_wait{
        {1, 0, 1, 0, 0, 0}}; // lock server request: 0 and 2

    RPCServiceBlocked _rpc_service_blocked{RPCServiceBlocked(_service_config)};
    std::unique_ptr<fep3::native::IRpcHttpServer> _server;
};