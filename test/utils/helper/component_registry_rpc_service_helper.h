#include <gtest/gtest.h>
#include <json/value.h>
#include <fep3/fep3_errors.h>

namespace fep3
{
namespace test
{
namespace helper
{

    inline void validateJsorError(const Json::Value& json_value)
    {
        using namespace ::testing;
        ASSERT_TRUE(json_value["error"].isMember("error_code")) << "error_code is not a property of the json object";
        ASSERT_TRUE(json_value["error"].isMember("description")) << "description is not a property of the json object";
        ASSERT_TRUE(json_value["error"].isMember("line")) << "line is not a property of the json object";
        ASSERT_TRUE(json_value["error"].isMember("file")) << "file is not a property of the json object";
        ASSERT_TRUE(json_value["error"].isMember("function")) << "function is not a property of the json object";

        ASSERT_EQ(fep3::ERR_INVALID_ARG, json_value["error"]["error_code"].asInt()) << "json property error_code does not match the expected error code";
        ASSERT_FALSE(json_value["error"]["description"].asString().empty()) << "json property description contains empty string";
        ASSERT_FALSE(json_value["error"]["line"].asString().empty()) << "json property line contains empty string";
        ASSERT_FALSE(json_value["error"]["file"].asString().empty()) << "json property file contains empty string";
        ASSERT_FALSE(json_value["error"]["function"].asString().empty()) << "json property function contains empty string";

        ASSERT_FALSE(json_value.isMember("version")) << "unexpected json property";
        ASSERT_FALSE(json_value.isMember("file_path")) << "unexpected json property";
        ASSERT_FALSE(json_value.isMember("participant_version")) << "unexpected json property";
    }

    inline void validateJsonKey(
        const Json::Value& json_value,
        const std::string& key,
        const std::string& expected_key_value = "")
    {
        using namespace ::testing;
        ASSERT_FALSE(json_value.isMember("error"));
        ASSERT_TRUE(json_value.isMember(key));
        if (expected_key_value.empty())
        {
            ASSERT_NE(true, json_value[key].asString().empty()) << key << " is not a property of the json object";
        }
        else
        {
            ASSERT_EQ(expected_key_value, json_value[key].asString()) << "json property contains empty string";
        }
    }
    
    inline void keyContains(const Json::Value& json_value,
        const std::string& key,
        const std::vector<std::string>& values)
    {
        using namespace ::testing;
        ASSERT_FALSE(json_value.isMember("error"));
        ASSERT_TRUE(json_value.isMember(key));

        const std::string key_value = json_value[key].asString();

       ASSERT_TRUE(std::all_of(values.begin(), values.end(), [&](const std::string & value)
            {
                return key_value.find(value) != std::string::npos;
            })) << "json property does not contain the expected strings";

    }

} // namespace helper
} // namespace test
} // namespace fep3
