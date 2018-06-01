#include "CombatSearch_Integral.h"

using namespace BOSS;

CombatSearch_Integral::CombatSearch_Integral(const CombatSearchParameters p)
{
    m_params = p;

    BOSS_ASSERT(m_params.getInitialState().getRace() != Races::None, "Combat search initial state is invalid");
}

void CombatSearch_Integral::doSearch(const GameState & state, size_t depth)
{
    if (timeLimitReached())
    {
        throw BOSS_COMBATSEARCH_TIMEOUT;
    }

    updateResults(state);

    if (isTerminalNode(state, depth))
    {
        return;
    }

    ActionSet legalActions;
    generateLegalActions(state, legalActions, m_params);
    
    for (size_t a(0); a < legalActions.size(); ++a)
    {
        const size_t index = legalActions.size()-1-a;

        GameState child(state);
        child.doAction(legalActions[index]);
        m_buildOrder.add(legalActions[index]);
        m_integral.update(state, m_buildOrder);
        
        doSearch(child,depth+1);

        m_buildOrder.pop_back();
        m_integral.pop();
    }
}

void CombatSearch_Integral::printResults()
{
    m_integral.print();
}

#include "BuildOrderPlotter.h"
void CombatSearch_Integral::writeResultsFile(const std::string & filename)
{
    BuildOrderPlotter plot;
    plot.addPlot("IntegralPlot", m_params.getInitialState(), m_integral.getBestBuildOrder());
    plot.doPlots();
}