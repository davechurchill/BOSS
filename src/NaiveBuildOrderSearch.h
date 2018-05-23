#pragma once

#include "Common.h"
#include "BuildOrderSearchGoal.h"
#include "GameState.h"
#include "BuildOrder.h"
#include "Tools.h"

namespace BOSS
{

class NaiveBuildOrderSearch
{
    GameState                   m_state;
    BuildOrderSearchGoal        m_goal;
    BuildOrder                  m_buildOrder;

    bool                        m_naiveSolved;

    bool                        checkUnsolvable();

public:

    NaiveBuildOrderSearch(const GameState & state, const BuildOrderSearchGoal & goal);

    const BuildOrder & solve();
};

}