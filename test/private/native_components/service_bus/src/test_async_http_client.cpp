/**
 * Copyright 2024 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "test_async_http_client_base.h"

#include <boost/thread/barrier.hpp>

#include <array>
#include <common/gtest_asserts.h>
#include <thread>

using namespace fep3;
using namespace fep3::native;
using namespace ::testing;

TEST_F(BoostJsonRpcClientTest, singleJsonRPCRequest__successful)
{
    std::string response;
    response = _client->GetObjects();
    ASSERT_EQ(response, "OK");
}

TEST_F(BoostRpcClientTestBase, sendRequestnoServer__failed)
{
    RPCResponseData rpc_response{};

    auto client = _factory.getRequester(_server_address);
    rpc_response.result = client->sendRequest(_service_name, _request_str, rpc_response.callback);
    ASSERT_FEP3_RESULT(rpc_response.result, ERR_NOT_CONNECTED);
}

TEST_F(BoostRpcClientTest, sendRequestServerNotListeningDuringSendRequest__failed)
{
    _server->stopListening();

    _rpc_response.result =
        _client->sendRequest(_service_name, _request_str, _rpc_response.callback);
    ASSERT_FEP3_RESULT(_rpc_response.result, ERR_NOT_CONNECTED);
}
// fep3::arya::IRPCRequester timeout test disabled due to long default timeout time.
// This test shall be used for local tests only. The timeout behavior is tested with
// fep3::experimental::IRPCRequester in test case: sendRequestExplicitTimeout__failed
TEST_F(BoostRpcClientTestServiceBlocked, DISABLED_sendRequestTimeout__failed)
{
    _rpc_response.result =
        _client->sendRequest(_service_name, _request_str, _rpc_response.callback);
    ASSERT_FEP3_RESULT(_rpc_response.result, fep3::ERR_TIMEOUT);
}
// fep3::arya::IRPCRequester timeout test disabled due to long default timeout time.
// This test shall be used for local tests only. The timeout behavior is tested with
// fep3::experimental::IRPCRequester in test case: resendRequestAfterExplicitTimeout__successful
TEST_F(BoostRpcClientTestServiceBlocked, DISABLED_resendRequestAfterTimeout__successful)
{
    _rpc_response.result =
        _client->sendRequest(_service_name, _request_str, _rpc_response.callback);
    ASSERT_FEP3_RESULT(_rpc_response.result, fep3::ERR_TIMEOUT);

    releaseService();

    _rpc_response.result =
        _client->sendRequest(_service_name, _request_str, _rpc_response.callback);
    ASSERT_FEP3_RESULT(_rpc_response.result, fep3::ERR_NOERROR);
    ASSERT_EQ(_rpc_response.callback.getResponse(), _response_str);
}

/// test disconnect
TEST_F(BoostRpcClientTest, sendAfterDisconnect__successful)
{
    _server->stopListening();
    startListeningOnFreePort(*_server);

    _rpc_response.result =
        _client->sendRequest(_service_name, _request_str, _rpc_response.callback);
    ASSERT_FEP3_RESULT(_rpc_response.result, fep3::ERR_NOERROR);
    ASSERT_EQ(_rpc_response.callback.getResponse(), _response_str);
}

TEST_F(BoostRpcClientTestBase, constructDestruct__successful)
{
    auto client = _factory.getRequester(_server_address);
    client.reset();
}

// test with receive body larger than mtu
TEST_F(BoostRpcClientTestMTUReceive, rpcReceiveLargerThanMTU__successful)
{
    _rpc_response.result =
        _client->sendRequest(_service_name, _request_str, _rpc_response.callback);
    ASSERT_FEP3_RESULT(_rpc_response.result, fep3::ERR_NOERROR);
    ASSERT_EQ(_rpc_response.callback.getResponse(), _response_str);
}

// test with send body bigger than mtu
TEST_F(BoostRpcClientTestMTUSend, rpcSendLargerThanMTU__successful)
{
    _rpc_response.result =
        _client->sendRequest(_service_name, _request_str, _rpc_response.callback);
    ASSERT_FEP3_RESULT(_rpc_response.result, fep3::ERR_NOERROR);
    ASSERT_EQ(_rpc_response.callback.getResponse(), _response_str);
}

// send to non existing url
TEST_F(BoostRpcClientTestBase, sendToNonExistingUrl__failed)
{
    auto rpc_service = RPCService(_request_str, _response_str);
    RPCResponseData rpc_response;

    auto server = _factory.createJsonRpcServer(_participant_name, _system_name, true);
    server->registerRPCObject(_service_name, &rpc_service);
    startListeningOnFreePort(*server);

    auto client = _factory.getRequester("http://0.0.0.256:9092");

    rpc_response.result = client->sendRequest(_service_name, _request_str, rpc_response.callback);
    ASSERT_FEP3_RESULT(rpc_response.result, fep3::ERR_NOT_CONNECTED);
}

// send to non existing RPC service - http protocol error
TEST_F(BoostRpcClientTest, sendToNonExistingRpcService__failed)
{
    _rpc_response.result =
        _client->sendRequest("dummy_service", _request_str, _rpc_response.callback);
    ASSERT_FEP3_RESULT(_rpc_response.result, fep3::ERR_FAILED);
}

// send an empty RPC request - http protocol error
TEST_F(BoostRpcClientTest, sendEmptyRPCRequest)
{
    _rpc_response.result = _client->sendRequest(_service_name, "", _rpc_response.callback);
    ASSERT_FEP3_RESULT(_rpc_response.result, fep3::ERR_FAILED);
}

// send an unsupported RPC request - http protocol error
TEST_F(BoostRpcClientTest, sendUnsupportedRPCRequest)
{
    _rpc_response.result =
        _client->sendRequest(_service_name, "dummy_request", _rpc_response.callback);
    ASSERT_FEP3_RESULT(_rpc_response.result, fep3::ERR_FAILED);
}

// receive an invalid response string length - http message received by the client has a empty body
TEST_F(BoostRpcClientTestInvalidResponse, receiveInvalidResponseStringLength__failed)
{
    _rpc_response.result =
        _client->sendRequest(_service_name, _request_str, _rpc_response.callback);
    ASSERT_FEP3_RESULT(_rpc_response.result, fep3::ERR_EMPTY);
}

// use 2 clients same services
TEST_F(BoostRpcMultiClientTest, twoClientsSameService__successful)
{
    std::vector<ServerConfiguration> config = {
        {_service_name, std::make_shared<RPCService>(_request_str, _response_str)}};
    const int clients_numOf = 2;

    initialize(clients_numOf, config);

    for (int i = 0; i < clients_numOf; i++) {
        _rpc_responses[i].result =
            _clients[i]->sendRequest(_service_name, _request_str, _rpc_responses[i].callback);
        ASSERT_FEP3_RESULT(_rpc_responses[i].result, fep3::ERR_NOERROR);
        ASSERT_EQ(_rpc_responses[i].callback.getResponse(), _response_str);
    }
}

// use 2 clients with different services
TEST_F(BoostRpcMultiClientTest, twoClientsDifferentService__successful)
{
    auto rpc_service = std::make_shared<RPCService>(_request_str, _response_str);
    std::vector<ServerConfiguration> config = {{"my_service_1", rpc_service},
                                               {"my_service_2", rpc_service}};
    const int clients_numOf = 2;
    initialize(clients_numOf, config);

    for (int i = 0; i < clients_numOf; i++) {
        _rpc_responses[i].result = _clients[i]->sendRequest(
            config[i].service_name, _request_str, _rpc_responses[i].callback);
        ASSERT_FEP3_RESULT(_rpc_responses[i].result, fep3::ERR_NOERROR);
        ASSERT_EQ(_rpc_responses[i].callback.getResponse(), _response_str);
    }
}

/* Multithreading - multiple client requests
 * - one client (one boost http client context thread) sends six different requests from different
 * threads to server (RPCResponse::set() runs in six different threads). All requests are served
 * from the same client thread.
 * - request 0 and 2 are blocked on server's side until response from request 1 is processed by the
 * client
 * -- when response from request 1 is received, count-down latch and release request 0 on server
 * side
 * -- when response from request 0 is received, count-down latch and release request 2 on server
 * side
 */
TEST_F(BoostRpcClientTestMultiThreaded, singleClientMultipleRequests__successful)
{
    std::vector<std::thread> client_request_threads;

    // create client (single boost http client context thread)
    auto client = _factory.getRequester(_server_address);

    // client sends requests from different threads
    for (int i = 0; i < _requests_num_of; i++) {
        auto client_send = [this, &client](int index) {
            _bar.wait();
            _rpc_responses[index].result = client->sendRequest(
                _service_name, _service_config[index].request, _rpc_responses[index].callback);
        };
        client_request_threads.push_back(std::thread(client_send, i));
    }

    for (auto& t: client_request_threads) {
        t.join();
    }

    //// check responses
    for (int i = 0; i < _requests_num_of; i++) {
        ASSERT_FEP3_RESULT(_rpc_responses[i].result, fep3::ERR_NOERROR);

        // check if client receive the server response
        auto it = std::find_if(_service_config.begin(),
                               _service_config.end(),
                               [&](const RPCServiceBlocked::Configuration& config) {
                                   return config.response.compare(
                                              _rpc_responses[i].callback.getResponse()) == 0;
                               });
        ASSERT_NE(it, _service_config.end());
    }
}

TEST_F(BoostRpcClientTestMultiThreaded, multiClientMultipleRequests__successful)
{
    std::vector<std::thread> client_request_threads;
    std::vector<std::shared_ptr<fep3::arya::IRPCRequester>> clients(_requests_num_of);

    // create server
    auto server = _factory.createJsonRpcServer(_participant_name, _system_name, true);

    // create client (single boost http client context thread)
    std::generate(clients.begin(), clients.end(), [this]() {
        return _factory.getRequester(_server_address);
    });

    // client sends requests from different threads
    for (int i = 0; i < _requests_num_of; i++) {
        auto client_send = [this, &clients](int index) {
            _bar.wait();
            _rpc_responses[index].result = clients[index]->sendRequest(
                _service_name, _service_config[index].request, _rpc_responses[index].callback);
        };
        client_request_threads.push_back(std::thread(client_send, i));
    }

    for (auto& t: client_request_threads) {
        t.join();
    }

    // check responses
    for (int i = 0; i < _requests_num_of; i++) {
        ASSERT_FEP3_RESULT(_rpc_responses[i].result, fep3::ERR_NOERROR);

        // check if client receive the server response
        auto it = std::find_if(_service_config.begin(),
                               _service_config.end(),
                               [&](const RPCServiceBlocked::Configuration& config) {
                                   return config.response.compare(
                                              _rpc_responses[i].callback.getResponse()) == 0;
                               });
        ASSERT_NE(it, _service_config.end());
    }
}

// test with parallel receive when receive body larger than mtu
// one clients sends same request several times
TEST_F(BoostRpcClientTestMTUReceive, oneRequesterReceiveLargerThanMTU__successful)
{
    std::vector<std::thread> client_request_threads;
    const int requests_num_of = 2;
    std::vector<RPCResponseData> rpc_responses(requests_num_of);
    boost::barrier bar{requests_num_of}; // barrier to sync client sendRequests()

    std::generate(rpc_responses.begin(), rpc_responses.end(), []() { return RPCResponseData(); });

    // client sends requests from different threads
    for (int i = 0; i < requests_num_of; i++) {
        auto client_send = [this, &bar, &rpc_responses](int index) {
            bar.wait();
            rpc_responses[index].result =
                _client->sendRequest(_service_name, _request_str, rpc_responses[index].callback);
        };
        client_request_threads.push_back(std::thread(client_send, i));
    }

    for (auto& t: client_request_threads) {
        t.join();
    }

    // check responses
    for (auto& rpc_response: rpc_responses) {
        ASSERT_EQ(rpc_response.result, ERR_NOERROR);
        // check if client receive the server response
        ASSERT_EQ(rpc_response.callback.getResponse(), _response_str);
    }
}

// test with parallel receive when receive body larger than mtu
// two clients sends same request
TEST_F(BoostRpcMultiClientTest, multipleRequesterReceiveLargerThanMTU__successful)
{
    std::string response_str = randomString(max_message_size * 10);
    std::vector<ServerConfiguration> config = {
        {_service_name, std::make_shared<RPCService>(_request_str, response_str)}};
    std::vector<std::thread> client_request_threads;
    const int clients_numOf = 2;
    boost::barrier bar{clients_numOf}; // barrier to sync client sendRequests()

    initialize(clients_numOf, config);

    // client sends requests from different threads
    for (int i = 0; i < clients_numOf; i++) {
        auto client_send = [this, &bar](int index) {
            bar.wait();
            _rpc_responses[index].result = _clients[index]->sendRequest(
                _service_name, _request_str, _rpc_responses[index].callback);
        };
        client_request_threads.push_back(std::thread(client_send, i));
    }

    for (auto& t: client_request_threads) {
        t.join();
    }

    // check responses
    for (auto& rpc_response: _rpc_responses) {
        ASSERT_EQ(rpc_response.result, ERR_NOERROR);
        // check if client receive the server response
        ASSERT_EQ(rpc_response.callback.getResponse(), response_str);
    }
}

TEST_F(BoostRpcAsyncClientTestServiceBlocked, sendRequestExplicitTimeout__failed)
{
    using namespace std::chrono_literals;
    auto timeout_time = 1s;
    _client->setTimeout(timeout_time);

    _rpc_response = _client->sendRequest(_service_name, _request_str);

    EXPECT_FEP3_RESULT(_rpc_response.first, ERR_TIMEOUT);

    std::string description_substr =
        "timeout after " + std::to_string(timeout_time.count()) + " seconds";
    ASSERT_TRUE(std::string(_rpc_response.first.getDescription()).find(description_substr) !=
                std::string::npos);
}

TEST_F(BoostRpcAsyncClientTestServiceBlocked, resendRequestAfterExplicitTimeout__successful)
{
    using namespace std::chrono_literals;
    auto timeout_time = 1s;
    _client->setTimeout(timeout_time);

    _rpc_response = _client->sendRequest(_service_name, _request_str);

    EXPECT_FEP3_RESULT(_rpc_response.first, ERR_TIMEOUT);

    releaseService();

    _rpc_response = _client->sendRequest(_service_name, _request_str);
    ASSERT_FEP3_RESULT(_rpc_response.first, fep3::ERR_NOERROR);
    ASSERT_EQ(_rpc_response.second, _response_str);
}

// test for timeout time measurement is disabled due to the indefinite execution time of the http
// request in its context thread
TEST_F(BoostRpcAsyncClientTestServiceBlocked, DISABLED_sendRequestExplicitTimeout__measurement)
{
    using namespace std::chrono_literals;
    auto timeout_time = 1s;
    auto epsilon_time = 50ms;
    auto max_timeout_time = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout_time) +
                            std::chrono::duration_cast<std::chrono::nanoseconds>(epsilon_time);

    _client->setTimeout(std::chrono::duration_cast<std::chrono::seconds>(timeout_time));

    auto begin = std::chrono::high_resolution_clock::now();
    _rpc_response = _client->sendRequest(_service_name, _request_str);
    auto end = std::chrono::high_resolution_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
    ASSERT_TRUE(elapsed < max_timeout_time);
}

TEST_F(BoostRpcClientTestBase, getRequester_sameInstance)
{
    TemplateHttpRpcFactoryImpl<AsyncRequester> requester_factory_impl;
    auto requester_a = requester_factory_impl.createRequesterInt(_server_address);
    auto requester_b = requester_factory_impl.createRequesterInt(_server_address);

    ASSERT_TRUE(requester_a.get() == requester_b.get());
}

TEST_F(BoostRpcClientTestBase, getAsyncRequester_parallel)
{
    std::vector<std::thread> client_threads;
    const int clients_numOf = 2;
    std::vector<std::shared_ptr<fep3::experimental::IRPCRequester>> clients;
    boost::barrier bar{clients_numOf}; // barrier to sync client creation

    // client sends requests from different threads
    for (int i = 0; i < clients_numOf; i++) {
        auto client_get = [this, &bar, &clients]() {
            bar.wait();
            auto client = _factory.getAsyncRequester(_server_address);
            clients.emplace_back(client);
        };
        client_threads.push_back(std::thread(client_get));
    }

    for (auto& t: client_threads) {
        t.join();
    }

    ASSERT_TRUE(clients.size() == clients_numOf);
}

TEST_F(BoostRpcClientTestServiceBlockedBase, sendRequestExplicitTimeout__singletonTimeout)
{
    using namespace std::chrono_literals;
    auto timeout_time_a = 1000000s;
    auto timeout_time_b = 1s;

    auto client_a = _factory.getAsyncRequester(_server_address);
    auto client_b = _factory.getAsyncRequester(_server_address);

    client_a->setTimeout(timeout_time_a);
    client_b->setTimeout(timeout_time_b);

    auto [result, response] = client_a->sendRequest(_service_name, _request_str);

    EXPECT_FEP3_RESULT(result, ERR_TIMEOUT);
    EXPECT_TRUE(response.empty());

    std::string description_substr =
        "timeout after " + std::to_string(timeout_time_b.count()) + " seconds";
    ASSERT_TRUE(std::string(result.getDescription()).find(description_substr) != std::string::npos);
}