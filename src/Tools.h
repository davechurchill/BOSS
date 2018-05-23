#pragma once

#include "Common.h"
#include "GameState.h"
#include "BuildOrderSearchGoal.h"
#include "BuildOrder.h"
#include "ActionSet.h"

namespace BOSS
{
namespace Tools
{
    int         GetUpperBound(const GameState & state, const BuildOrderSearchGoal & goal);
    int         GetLowerBound(const GameState & state, const BuildOrderSearchGoal & goal);
    int         CalculatePrerequisitesLowerBound(const GameState & state, const ActionSet & needed, int timeSoFar, int depth = 0);
    void        InsertActionIntoBuildOrder(BuildOrder & result, const BuildOrder & buildOrder, const GameState & initialState, const ActionType & action);
    void        CalculatePrerequisitesRequiredToBuild(const GameState & state, const ActionSet & wanted, ActionSet & requiredToBuild);
    BuildOrder  GetOptimizedNaiveBuildOrderOld(const GameState & state, const BuildOrderSearchGoal & goal);
    BuildOrder  GetNaiveBuildOrderAddWorkersOld(const GameState & state, const BuildOrderSearchGoal & goal, size_t maxWorkers);
    int         GetBuildOrderCompletionTime(const GameState & state, const BuildOrder & buildOrder);
}
}
