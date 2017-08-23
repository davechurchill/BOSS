#include "JSONTools.h"

using namespace BOSS;

std::string JSONTools::ReadFile(const std::string & filename)
{
    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

void JSONTools::ParseJSONString(rapidjson::Document & document, const std::string & json)
{
    bool parsingFailed = document.Parse<0>(json.c_str()).HasParseError();

    if (parsingFailed)
    {
        int errorPos = document.GetErrorOffset();

        std::stringstream ss;
        ss << std::endl << "JSON Parse Error: " << document.GetParseError() << std::endl;
        ss << "Error Position:   " << errorPos << std::endl;
        ss << "Error Substring:  " << json.substr(errorPos-5, 10) << std::endl;

        BOSS_ASSERT(!parsingFailed, "Error parsing JSON config file: %s", ss.str().c_str());
    }

    BOSS_ASSERT(!parsingFailed, "Parsing of the JSON string failed");

}

void JSONTools::ParseJSONFile(rapidjson::Document & document, const std::string & filename)
{
    JSONTools::ParseJSONString(document, JSONTools::ReadFile(filename));
}

BuildOrder JSONTools::GetBuildOrder(const std::string & jsonString)
{
    rapidjson::Document document;
    JSONTools::ParseJSONString(document, jsonString);
    return GetBuildOrder(document);
}

BuildOrder JSONTools::GetBuildOrder(const rapidjson::Value & stateVal)
{
    BOSS_ASSERT(stateVal.IsArray(), "Build order isn't an array");
    
    BuildOrder buildOrder;

    for (size_t i(0); i < stateVal.Size(); ++i)
    {
        BOSS_ASSERT(stateVal[i].IsString(), "Build order item is not a string");

        buildOrder.add(ActionTypes::GetActionType(stateVal[i].GetString()));
    }
    
    return buildOrder;
}

void JSONTools::ReadBool(const char * key, const rapidjson::Value & value, bool & dest)
{
    if (value.HasMember(key))
    {
        BOSS_ASSERT(value[key].IsBool(), "%s should be a bool", key);
        dest = value[key].GetBool();
    }
}

void JSONTools::ReadString(const char * key, const rapidjson::Value & value, std::string & dest)
{
    if (value.HasMember(key))
    {
        BOSS_ASSERT(value[key].IsString(), "%s should be a string", key);
        dest = value[key].GetString();
    }
}

std::string JSONTools::GetBuildOrderString(const std::vector<ActionType> & buildOrder)
{
    std::stringstream ss;

    ss << "\"Test Build\" : [";

    for (size_t i(0); i < buildOrder.size(); ++i)
    {
        ss << "\"" << buildOrder[i].getName() << "\"" << (i < buildOrder.size() - 1 ? ", " : "");
    }

    ss << "]";

    return ss.str();
}