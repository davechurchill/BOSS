#include "BOSSConfig.h"

using namespace BOSS;

BOSSConfig::BOSSConfig()
{

}

BOSSConfig & BOSSConfig::Instance()
{
    static BOSSConfig params;
    return params;
}

void BOSSConfig::ParseParameters(const std::string & configFile)
{
    _configFile = configFile;

    std::ifstream file(configFile);
    json j;
    file >> j;

    BOSS_ASSERT(j.count("States"), "No 'States' member found");
    BOSS_ASSERT(j.count("Build Orders"), "No 'Build Orders' member found");

    // Parse all the States
    for (auto it = j["States"].begin(); it != j["States"].end(); ++it)
    {           
        _stateMap[it.key()] = JSONTools::GetGameState(it.value());
    }

    // Parse the build orders
    for (auto it = j["Build Orders"].begin(); it != j["Build Orders"].end(); ++it)
    {          
        _buildOrderMap[it.key()] = JSONTools::GetBuildOrder(it.value());
    }

    // Parse all the Build Order Goals
    if (j.count("Build Order Search Goals"))
    {
        for (auto it = j["Build Order Search Goals"].begin(); it != j["Build Order Search Goals"].end(); ++it)
        {          
            _buildOrderSearchGoalMap[it.key()] = JSONTools::GetBuildOrderSearchGoal(it.value());
        }
    }
}

const GameState & BOSSConfig::GetState(const std::string & key)
{
    BOSS_ASSERT(_stateMap.find(key) != _stateMap.end(), "Couldn't find state: %s", key.c_str());

    return _stateMap[key];
}

const BuildOrder & BOSSConfig::GetBuildOrder(const std::string & key)
{
    BOSS_ASSERT(_buildOrderMap.find(key) != _buildOrderMap.end(), "Couldn't find build order: %s", key.c_str());

    return _buildOrderMap[key];
}

const BuildOrderSearchGoal & BOSSConfig::GetBuildOrderSearchGoalMap(const std::string & key)
{
    BOSS_ASSERT(_buildOrderSearchGoalMap.find(key) != _buildOrderSearchGoalMap.end(), "Couldn't find state: %s", key.c_str());

    return _buildOrderSearchGoalMap[key];
}