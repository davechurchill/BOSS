#include "CombatSearch_BestResponse.h"

using namespace BOSS;

CombatSearch_BestResponse::CombatSearch_BestResponse(const CombatSearchParameters p)
    : m_bestResponseData(p.getEnemyInitialState(), p.getEnemyBuildOrder())
{
    m_params = p;

    BOSS_ASSERT(m_params.getInitialState().getRace() != Races::None, "Combat search initial state is invalid");
}

void CombatSearch_BestResponse::recurse(const GameState & state, size_t depth)
{
    if (timeLimitReached())
    {
        throw BOSS_COMBATSEARCH_TIMEOUT;
    }

    m_bestResponseData.update(m_params.getInitialState(), state, m_buildOrder);
    updateResults(state);

    if (isTerminalNode(state, depth))
    {
        return;
    }

    ActionSet legalActions;
    generateLegalActions(state, legalActions, m_params);
    
    for (size_t a(0); a < legalActions.size(); ++a)
    {
        size_t ri = legalActions.size() - 1 - a;

        GameState child(state);
        child.doAction(legalActions[ri]);
        m_buildOrder.add(legalActions[ri]);
        
        recurse(child,depth+1);

        m_buildOrder.pop_back();
    }
}

void CombatSearch_BestResponse::printResults()
{

}

#include "BuildOrderPlotter.h"
void CombatSearch_BestResponse::writeResultsFile(const std::string & filename)
{
    BuildOrderPlotter plot;
    plot.addPlot("BestResponseSelf", m_params.getInitialState(), m_bestResponseData.getBestBuildOrder());
    plot.doPlots();

    BuildOrderPlotter plot2;
    plot2.addPlot("BestResponseEnemy", m_params.getEnemyInitialState(), m_params.getEnemyBuildOrder());
    plot2.doPlots();
}