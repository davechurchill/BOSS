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

GameState JSONTools::GetGameState(const json & j)
{
    GameState state;

    if (j.count("minerals") && j["minerals"].is_number_integer())
    {   
        state.setMinerals(j["minerals"]);
    }

    if (j.count("gas") && j["gas"].is_number_integer())
    {   
        state.setGas(j["gas"]);
    }

    if (j.count("units") && j["units"].is_array())
    {
        const auto & units = j["units"];
        for (auto & unit : units)
        {
            BOSS_ASSERT(unit.is_array() && unit.size() == 2 && unit[0].is_string() && unit[1].is_number_integer(), "Unit has to be array of size 2");

            for (int n(0); n < unit[1]; ++n)
            {
                state.addUnit(ActionTypes::GetActionType(unit[0]));
            }
        }
    }

    return state;
}

BuildOrderSearchGoal JSONTools::GetBuildOrderSearchGoal(const json & j)
{
    BOSS_ASSERT(j.count("race") && j["race"].is_string(), "State doesn't have a race");
    
    const RaceID race = Races::GetRaceID(j["race"]);

    BOSS_ASSERT(race != Races::None, "Unknown race (make sure to use a single upper case): %s", j["race"].get<std::string>().c_str());

    BuildOrderSearchGoal goal;

    if (j.count("goal") && j["goal"].is_array())
    {
        for (auto & unit : j["goal"])
        {
            BOSS_ASSERT(unit.is_array() && unit.size() == 2 && unit[0].is_string() && unit[1].is_number_integer(), "Goal entry has to be array of size 2");

            goal.setGoal(ActionTypes::GetActionType(unit[0]), unit[1]);
        }
    }

    if (j.count("goalMax") && j["goalMax"].is_array())
    {
        for (auto & unit : j["goalMax"])
        {
            BOSS_ASSERT(unit.is_array() && unit.size() == 2 && unit[0].is_string() && unit[1].is_number_integer(), "Goal max entry has to be array of size 2");

            goal.setGoalMax(ActionTypes::GetActionType(unit[0u]), unit[1u]);
        }
    }

    return goal;
}

BuildOrder JSONTools::GetBuildOrder(const json & j)
{
    BOSS_ASSERT(j.is_array(), "Build order isn't an array");
    
    BuildOrder buildOrder;

    for (auto & type : j)
    {
        BOSS_ASSERT(type.is_string(), "Build order item is not a string");

        buildOrder.add(ActionTypes::GetActionType(type));
    }
    
    return buildOrder;
}

void JSONTools::ReadBool(const char * key, const json & j, bool & dest)
{
    if (j.count(key))
    {
        BOSS_ASSERT(j[key].is_boolean(), "%s should be a bool", key);
        dest = j[key];
    }
}

void JSONTools::ReadString(const char * key, const json & j, std::string & dest)
{
    if (j.count(key))
    {
        BOSS_ASSERT(j[key].is_string(), "%s should be a string", key);
        dest = j[key].get<std::string>();
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