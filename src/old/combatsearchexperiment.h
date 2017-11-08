#pragma once

#include "BOSS.h"
#include "JSONTools.h"
#include <memory>

namespace BOSS
{

class CombatSearchExperiment
{
    std::string                 _name;
    //CombatSearchParameters      _params;
    RaceID                      _race;
    std::vector<std::string>    _searchTypes;

    RaceID                      _enemyRace;
    BuildOrder                  _enemyBuildOrder;

public:

    CombatSearchExperiment();
    CombatSearchExperiment(const std::string & name, const json & j);

    void run();
};
}
