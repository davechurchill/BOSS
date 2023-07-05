#include "BuildOrderSearch.h"

void BuildOrderSearch::addGoal(const ActionType& a, const size_t& count)
{
    m_goal.setGoal(a, count);
}

void BuildOrderSearch::setGoal(const BuildOrderSearchGoal& g)
{
    m_goal = g;
}

void BuildOrderSearch::setState(const GameState& state)
{
    m_initialState = state;
}

void BuildOrderSearch::clearGoalsAndResults()
{
    m_goal = BuildOrderSearchGoal();
    m_results = BuildOrderSearchResults();
}


void BuildOrderSearch::setTimeLimit(int n)
{
    m_searchTimeLimit = n;
}

const BuildOrderSearchResults& BuildOrderSearch::getResults() const
{
    return m_results;
}

ActionSet BuildOrderSearch::findLooseDependancies() const
{
    // Main dependancies
    ActionSet dependancies;
    for (auto& act : BOSS::ActionTypes::GetAllActionTypes())
    {
        size_t n = m_goal.getGoal(act);
        if (n == 0) { continue; }
        dependancies.add(act.getRecursivePrerequisiteActionCount());
        dependancies.add(act);
    }
    return dependancies;
}