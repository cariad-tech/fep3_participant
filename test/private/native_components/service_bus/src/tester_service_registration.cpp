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

#include <gtest/gtest.h>
#include <fep3/components/service_bus/rpc/fep_rpc.h>
#include <fep3/rpc_services/base/fep_rpc_client.h>

#include <testclientstub.h>
#include <testserverstub.h>

#include <fep3/native_components/service_bus/service_bus.h>

class ITestInterface
{
public:
    FEP_RPC_IID("ITestInterface", "test_service");
};

class TestService : public fep3::rpc::RPCService<::test::rpc_stubs::TestInterfaceServer,
    ITestInterface>
{
    int _value = 0;
public:
    static int GetRunlevel_call_count;
    static int GetObjects_call_count;
    static int GetRPCIIDForObjects_call_count;
    static int SetRunlevel_call_count;

    virtual std::string GetObjects()
    {
        ++GetObjects_call_count;
        return "bla, blubb, bla";
    }
    virtual std::string GetRPCIIDForObject(const std::string& strObject)
    {
        ++GetRPCIIDForObjects_call_count;
        if (strObject == "bla")
        {
            return "blubb";
        }
        else if (strObject == "blubb")
        {
            return "bla";
        }
        return {};
    }
    virtual int GetRunlevel()
    {
        ++GetRunlevel_call_count;
        return _value;
    }
    virtual Json::Value SetRunlevel(int nRunLevel)
    {
        ++SetRunlevel_call_count;
        _value = nRunLevel;

        Json::Value val;
        val["ErrorCode"] = _value;
        val["Description"] = "Desc";
        val["Line"] = 1234;
        val["File"] = "File";
        val["Function"] = "Foo";
        return val;
    }
};
int TestService::GetObjects_call_count;
int TestService::GetRunlevel_call_count;
int TestService::SetRunlevel_call_count;
int TestService::GetRPCIIDForObjects_call_count;

class TestServiceSimple : public fep3::rpc::RPCService<::test::rpc_stubs::TestInterfaceServer,
    ITestInterface>
{
    int _value = 1;
public:
    int GetRunlevel_call_count = 0;
    int GetObjects_call_count = 0;
    int GetRPCIIDForObjects_call_count = 0;
    int SetRunlevel_call_count = 0;

    virtual std::string GetObjects()
    {
        ++GetObjects_call_count;
        return "OK";
    }
    virtual std::string GetRPCIIDForObject(const std::string& strObject)
    {
        ++GetRPCIIDForObjects_call_count;
        return strObject;
    }
    virtual int GetRunlevel()
    {
        ++GetRunlevel_call_count;
        return _value;
    }
    virtual Json::Value SetRunlevel(int nRunLevel)
    {
        ++SetRunlevel_call_count;
        _value = nRunLevel;

        Json::Value val;
        val["ErrorCode"] = _value;
        val["Description"] = "D";
        val["Line"] = 4321;
        val["File"] = "Fl";
        val["Function"] = "Func";
        return val;
    }
};

class TestClient : public fep3::rpc::RPCServiceClient<::test::rpc_stubs::TestInterfaceClient,
    ITestInterface>
{
public:
    TestClient(const char* service_name,
        const std::shared_ptr<fep3::IRPCRequester>& rpc) : fep3::rpc::RPCServiceClient<::test::rpc_stubs::TestInterfaceClient,
        ITestInterface>(service_name, rpc)
    {
    }
};

/**
 * @detail Test the registration, unregistration and memorymanagment of the ServiceBus
 * @req_id FEPSDK-ServiceBus
 *
 */
TEST(ServiceBusServer, testRegistrationOfServices)
{
    TestService::GetObjects_call_count = 0;
    TestService::GetRunlevel_call_count = 0;
    TestService::SetRunlevel_call_count = 0;
    TestService::GetRPCIIDForObjects_call_count = 0;

    constexpr const char* const test_server_url = "http://localhost:9900";
    auto test_service = std::make_shared<TestService>();
    fep3::native::ServiceBus bus;

    ASSERT_TRUE(fep3::isOk(bus.createSystemAccess("sysname",
        "",
        true)));

    auto sys_access = bus.getSystemAccess("sysname");
    ASSERT_TRUE(sys_access);

    //now create the server
    ASSERT_TRUE(fep3::isOk(sys_access->createServer("name_of_server",
        test_server_url)));

    //default server set now
    auto server = bus.getServer();
    ASSERT_TRUE(server);

    //register the service
    ASSERT_TRUE(fep3::isOk(server->registerService("test_service", test_service)));

    //register twice is not possible
    ASSERT_FALSE(fep3::isOk(server->registerService("test_service", test_service)));

    //impl test
    fep3::rpc::RPCClient<ITestInterface> my_interface_client;

    //test the client server connections
    TestClient client(ITestInterface::getRPCDefaultName(),
    bus.getRequester(test_server_url, true));

    ASSERT_NO_THROW(
        ASSERT_EQ(TestService::SetRunlevel_call_count, 0);
        auto val = client.SetRunlevel(1234);
        ASSERT_EQ(TestService::SetRunlevel_call_count, 1);
    );

    ASSERT_NO_THROW(
        ASSERT_EQ(TestService::GetRunlevel_call_count, 0);
        auto run_level = client.GetRunlevel();
        ASSERT_EQ(TestService::GetRunlevel_call_count, 1);
        ASSERT_EQ(run_level, 1234);
    );

    ASSERT_NO_THROW(
        ASSERT_EQ(TestService::GetObjects_call_count, 0);
        auto objects = client.GetObjects();
        ASSERT_EQ(TestService::GetObjects_call_count, 1);
        ASSERT_EQ(objects, "bla, blubb, bla");
    );

    ASSERT_NO_THROW(
        ASSERT_EQ(TestService::GetRPCIIDForObjects_call_count, 0);
        auto object_IID = client.GetRPCIIDForObject("bla");
        ASSERT_EQ(TestService::GetRPCIIDForObjects_call_count, 1);
        ASSERT_EQ(object_IID, "blubb");
    );

    ASSERT_NO_THROW(
        auto object_IID = client.GetRPCIIDForObject("blubb");
        ASSERT_EQ(TestService::GetRPCIIDForObjects_call_count, 2);
        ASSERT_EQ(object_IID, "bla");
    );

    ASSERT_NO_THROW(
        auto object_IID = client.GetRPCIIDForObject("test");
        ASSERT_EQ(TestService::GetRPCIIDForObjects_call_count, 3);
        ASSERT_EQ(object_IID, "");
    );

    //unregister the service
    ASSERT_TRUE(fep3::isOk(server->unregisterService("test_service")));

    ASSERT_ANY_THROW(
        auto object_IID = client.GetRPCIIDForObject("test");
    );

    ASSERT_EQ(TestService::GetRPCIIDForObjects_call_count, 3);
}


/**
 * @detail Test the registration and unregistration of multiple services
 * @req_id FEPSDK-3097
 *
 */
TEST(ServiceBusServer, testRegistrationOfMultipleServices)
{
    TestService::GetObjects_call_count = 0;
    TestService::GetRunlevel_call_count = 0;
    TestService::SetRunlevel_call_count = 0;
    TestService::GetRPCIIDForObjects_call_count = 0;

    constexpr const char* const test_server_url = "http://localhost:9901";
    auto test_service = std::make_shared<TestService>();
    auto test_service_simple = std::make_shared<TestServiceSimple>();
    fep3::native::ServiceBus bus;

    ASSERT_TRUE(fep3::isOk(bus.createSystemAccess("sysname",
        "",
        true)));

    auto sys_access = bus.getSystemAccess("sysname");
    ASSERT_TRUE(sys_access);

    //now create the server
    ASSERT_TRUE(fep3::isOk(sys_access->createServer("name_of_server",
        test_server_url)));

    //default server set now
    auto server = bus.getServer();
    ASSERT_TRUE(server);

    //test the client server connections
    TestClient client_1("test_service_1",
        bus.getRequester(test_server_url, true));

    TestClient client_2("test_service_2",
        bus.getRequester(test_server_url, true));

    ASSERT_ANY_THROW(client_1.SetRunlevel(10));
    ASSERT_ANY_THROW(client_2.SetRunlevel(20));

    //register the first service
    ASSERT_TRUE(fep3::isOk(server->registerService("test_service_1", test_service)));

    ASSERT_NO_THROW(
        auto val = client_1.SetRunlevel(1234);
        ASSERT_EQ(TestService::SetRunlevel_call_count, 1);
    );

    ASSERT_ANY_THROW(client_2.SetRunlevel(200));

    //register the second service
    ASSERT_TRUE(fep3::isOk(server->registerService("test_service_2", test_service_simple)));

    ASSERT_NO_THROW(
        auto val = client_1.SetRunlevel(1234);
        ASSERT_EQ(TestService::SetRunlevel_call_count, 2);
    );


    ASSERT_NO_THROW(
        ASSERT_EQ(test_service_simple->SetRunlevel_call_count, 0);
        auto val = client_2.SetRunlevel(1234);
        ASSERT_TRUE(val.isObject());
        auto& val_line = val["Line"];
        ASSERT_TRUE(val_line.isInt());
        ASSERT_EQ(val_line.asInt(), 4321);
        ASSERT_EQ(test_service_simple->SetRunlevel_call_count, 1);
    );

    ASSERT_NO_THROW(
        ASSERT_EQ(TestService::GetRunlevel_call_count, 0);
        auto run_level = client_1.GetRunlevel();
        ASSERT_EQ(TestService::GetRunlevel_call_count, 1);
        ASSERT_EQ(run_level, 1234);
    );

    //unregister the services
    ASSERT_TRUE(fep3::isOk(server->unregisterService("test_service_2")));
    ASSERT_TRUE(fep3::isOk(server->unregisterService("test_service_1")));
}
