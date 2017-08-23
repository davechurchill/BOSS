#pragma once

#include "Common.h"
#include "BuildOrder.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

namespace BOSS
{
namespace JSONTools
{
    template <class T>
    void ReadInt(const char * key, const rapidjson::Value & value, T & dest)
    {
        if (value.HasMember(key))
        {
            BOSS_ASSERT(value[key].IsInt(), "%s should be an int", key);
            dest = (T)value[key].GetInt();
        }
    }

    void ReadBool(const char * key, const rapidjson::Value & value, bool & dest);
    void ReadString(const char * key, const rapidjson::Value & value, std::string & dest);

    std::string ReadFile(const std::string & filename);
    void ParseJSONString(rapidjson::Document & document, const std::string & json);
    void ParseJSONFile(rapidjson::Document & document, const std::string & filename);

    BuildOrder GetBuildOrder(const std::string & jsonString);
    BuildOrder GetBuildOrder(const rapidjson::Value & stateVal);
    
    std::string GetBuildOrderString(const std::vector<ActionType> & buildOrder);
}
}
