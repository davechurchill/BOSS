#include "JSONTools.h"
#include "GameState.h"

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

GameState JSONTools::GetGameState(const rapidjson::Value & stateVal)
{
    BOSS_ASSERT(stateVal.HasMember("race") && stateVal["race"].IsString(), "State doesn't have a race");
    
    const RaceID race = Races::GetRaceID(stateVal["race"].GetString());

    BOSS_ASSERT(race != Races::None, "Unknown race (make sure to use a single upper case): %s", stateVal["race"].GetString());

    GameState state;

    if (stateVal.HasMember("minerals") && stateVal["minerals"].IsInt())
    {   
        state.setMinerals(stateVal["minerals"].GetInt());
    }

    if (stateVal.HasMember("gas") && stateVal["gas"].IsInt())
    {   
        state.setGas(stateVal["gas"].GetInt());
    }

    if (stateVal.HasMember("units") && stateVal["units"].IsArray())
    {
        const rapidjson::Value & units = stateVal["units"];
        for (size_t i(0); i < units.Size(); ++i)
        {
            const rapidjson::Value & unit = units[i];
            BOSS_ASSERT(unit.IsArray() && unit.Size() == 2 && unit[0u].IsString() && unit[1u].IsInt(), "Unit has to be array of size 2");

            for (int n(0); n < unit[1u].GetInt(); ++n)
            {
                state.addInstance(ActionTypes::GetActionType(unit[0u].GetString()));
            }
        }
    }

    return state;
}

BuildOrderSearchGoal JSONTools::GetBuildOrderSearchGoal(const std::string & jsonString)
{
    rapidjson::Document document;
    JSONTools::ParseJSONString(document, jsonString);
    return GetBuildOrderSearchGoal(document);
}

GameState JSONTools::GetGameState(const std::string & jsonString)
{
    rapidjson::Document document;
    JSONTools::ParseJSONString(document, jsonString);
    return GetGameState(document);
}

BuildOrderSearchGoal JSONTools::GetBuildOrderSearchGoal(const rapidjson::Value & val)
{
    BOSS_ASSERT(val.HasMember("race") && val["race"].IsString(), "State doesn't have a race");
    
    const RaceID race = Races::GetRaceID(val["race"].GetString());

    BOSS_ASSERT(race != Races::None, "Unknown race (make sure to use a single upper case): %s", val["race"].GetString());

    BuildOrderSearchGoal goal;

    if (val.HasMember("goal") && val["goal"].IsArray())
    {
        const rapidjson::Value & goalUnits = val["goal"];
        for (size_t i(0); i < goalUnits.Size(); ++i)
        {
            const rapidjson::Value & unit = goalUnits[i];
            BOSS_ASSERT(unit.IsArray() && unit.Size() == 2 && unit[0u].IsString() && unit[1u].IsInt(), "Goal entry has to be array of size 2");

            goal.setGoal(ActionTypes::GetActionType(unit[0u].GetString()), unit[1u].GetInt());
        }
    }

    if (val.HasMember("goalMax") && val["goalMax"].IsArray())
    {
        const rapidjson::Value & goalMax = val["goalMax"];
        for (size_t i(0); i < goalMax.Size(); ++i)
        {
            const rapidjson::Value & unit = goalMax[i];
            BOSS_ASSERT(unit.IsArray() && unit.Size() == 2 && unit[0u].IsString() && unit[1u].IsInt(), "Goal max entry has to be array of size 2");

            goal.setGoalMax(ActionTypes::GetActionType(unit[0u].GetString()), unit[1u].GetInt());
        }
    }

    return goal;
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