#pragma once

#include "BOSS.h"
#include "JSONTools.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "BuildOrderSearchGoal.h"

namespace BOSS
{

class BOSSConfig
{
    std::string                                 _configFile;

    std::map<std::string, GameState>            _stateMap;
    std::map<std::string, BuildOrder>           _buildOrderMap;
    std::map<std::string, BuildOrderSearchGoal> _buildOrderSearchGoalMap;
    
    BOSSConfig();

public:

    static BOSSConfig & Instance();
    void ParseParameters(const std::string & configFile);

    const GameState &               GetState(const std::string & key);
    const BuildOrder &              GetBuildOrder(const std::string & key);
    const BuildOrderSearchGoal &    GetBuildOrderSearchGoalMap(const std::string & key);
};
}
