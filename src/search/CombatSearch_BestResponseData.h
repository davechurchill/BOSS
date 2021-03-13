#pragma once

#include "Common.h"
#include "GameState.h"
#include "Eval.h"
#include "BuildOrder.h"

namespace BOSS
{
    
class CombatSearch_BestResponseData
{
    GameState               m_enemyInitialState;
    BuildOrder              m_enemyBuildOrder;

    std::vector<GameState>  m_enemyStates;
    std::vector< std::pair<double, double> >     m_enemyArmyValues;
    std::vector< std::pair<double, double> >     m_selfArmyValues;

    double                  m_bestEval;
    BuildOrder              m_bestBuildOrder;
    GameState               m_bestState;

    double compareBuildOrder(const GameState & state, const BuildOrder & buildOrder);
    size_t getStateIndex(const GameState & state);

    void calculateArmyValues(const GameState & state, const BuildOrder & buildOrder, std::vector< std::pair<double, double> > & values);

public:

    CombatSearch_BestResponseData(const GameState & enemyState, const BuildOrder & enemyBuildOrder);

    void update(const GameState & initialState, const GameState & currentState, const BuildOrder & buildOrder);

    const BuildOrder & getBestBuildOrder() const;

};

}
