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

void BOSSConfig::ParseConfig(const std::string & configFile)
{
    m_configFile = configFile;

    std::ifstream file(configFile);
    json j;
    file >> j;

    BOSS_ASSERT(j.count("States"), "No 'States' member found");
    BOSS_ASSERT(j.count("Build Orders"), "No 'Build Orders' member found");
	BOSS_ASSERT(j.count("Game Data"), "No 'Game Data' member found");

    // Parse all the States
    for (auto it = j["States"].begin(); it != j["States"].end(); ++it)
    {           
        m_stateMap[it.key()] = JSONTools::GetGameState(it.value());
    }

    // Parse the build orders
    for (auto it = j["Build Orders"].begin(); it != j["Build Orders"].end(); ++it)
    {          
        m_buildOrderMap[it.key()] = JSONTools::GetBuildOrder(it.value());
    }

    // Parse all the Build Order Goals
    if (j.count("Build Order Search Goals"))
    {
        for (auto it = j["Build Order Search Goals"].begin(); it != j["Build Order Search Goals"].end(); ++it)
        {          
            m_buildOrderSearchGoalMap[it.key()] = JSONTools::GetBuildOrderSearchGoal(it.value());
        }
    }
}

const GameState & BOSSConfig::GetState(const std::string & key)
{
    BOSS_ASSERT(m_stateMap.find(key) != m_stateMap.end(), "Couldn't find state: %s", key.c_str());

    return m_stateMap[key];
}

const BuildOrder & BOSSConfig::GetBuildOrder(const std::string & key)
{
    BOSS_ASSERT(m_buildOrderMap.find(key) != m_buildOrderMap.end(), "Couldn't find build order: %s", key.c_str());

    return m_buildOrderMap[key];
}

const BuildOrderSearchGoal & BOSSConfig::GetBuildOrderSearchGoalMap(const std::string & key)
{
    BOSS_ASSERT(m_buildOrderSearchGoalMap.find(key) != m_buildOrderSearchGoalMap.end(), "Couldn't find state: %s", key.c_str());

    return m_buildOrderSearchGoalMap[key];
}