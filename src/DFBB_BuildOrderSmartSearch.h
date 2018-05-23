#pragma once

#include "Common.h"
#include "GameState.h"
#include "DFBB_BuildOrderStackSearch.h"
#include "Timer.hpp"

namespace BOSS
{
class DFBB_BuildOrderSmartSearch
{
    RaceID                              m_race;

    DFBB_BuildOrderSearchParameters		m_params;
    BuildOrderSearchGoal 			    m_goal;

    std::vector<ActionType>             m_relevantActions;

    GameState					        m_initialState;

    int 							    m_searchTimeLimit;

    Timer							    m_searchTimer;

    DFBB_BuildOrderStackSearch          m_stackSearch;
    DFBB_BuildOrderSearchResults        m_results;

    void doSearch();
    void calculateSearchSettings();
    void setPrerequisiteGoalMax();
    void recurseOverStrictDependencies(const ActionType & action);
    void setRelevantActions();
    void setRepetitions();

    size_t calculateSupplyProvidersRequired();
    size_t calculateRefineriesRequired();

    const RaceID getRace() const;

public:

    DFBB_BuildOrderSmartSearch();

    void addGoal(const ActionType & a, const size_t & count);
    void setGoal(const BuildOrderSearchGoal & goal);
    void setState(const GameState & state);
    void print();
    void setTimeLimit(int n);

    void search();

    const DFBB_BuildOrderSearchResults & getResults() const;
    const DFBB_BuildOrderSearchParameters & getParameters();
};

}