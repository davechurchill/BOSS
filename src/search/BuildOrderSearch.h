#pragma once

#include "BuildOrderSearchResults.h"
#include "BuildOrderSearchGoal.h"
#include <Timer.hpp>

using namespace BOSS;

class BuildOrderSearch
{
protected:

    BuildOrderSearchGoal 			    m_goal;

    GameState					        m_initialState;

    int 							    m_searchTimeLimit = 0;

    Timer							    m_searchTimer;

    BuildOrderSearchResults        m_results;

    ActionSet findLooseDependancies() const;

public:
    void addGoal(const ActionType& a, const size_t& count);
    void setGoal(const BuildOrderSearchGoal& goal);
    void setState(const GameState& state);
    void clearGoalsAndResults();
    void setTimeLimit(int n);

    virtual void search() = 0;

    const BuildOrderSearchResults& getResults() const;
};