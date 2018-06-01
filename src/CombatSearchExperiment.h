#pragma once

#include "BOSS.h"
#include "JSONTools.h"
#include "CombatSearchParameters.h"

namespace BOSS
{

class CombatSearchExperiment
{
    std::string                 m_name;
    CombatSearchParameters      m_params;
    RaceID                      m_race;
    std::vector<std::string>    m_searchTypes;

    RaceID                      m_enemyRace;
    BuildOrder                  m_enemyBuildOrder;

public:

    CombatSearchExperiment();
    CombatSearchExperiment(const std::string & name, const json & experimentVal);

    void run();
};
}
