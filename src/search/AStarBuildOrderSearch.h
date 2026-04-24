#pragma once

#include "ActionOrdering.hpp"
#include "ActionSet.h"
#include "BuildOrder.h"
#include "BuildOrderSearchGoal.h"
#include "Common.h"
#include "DFBB_BuildOrderSearchParameters.h"
#include "DFBB_BuildOrderSearchResults.h"
#include "GameState.h"
#include "Timer.hpp"

#include <queue>
#include <unordered_map>

namespace BOSS
{

class AStarBuildOrderSearch
{
    struct SearchNode
    {
        GameState   state;
        BuildOrder  buildOrder;
        int         finishTime;
        int         lowerBound;
        size_t      sequence;
    };

    struct QueueEntry
    {
        size_t nodeIndex;
        int    priority;
        int    finishTime;
        size_t sequence;
    };

    struct QueueEntryCompare
    {
        bool operator()(const QueueEntry & lhs, const QueueEntry & rhs) const;
    };

    BuildOrderSearchGoal                m_goal;
    GameState                           m_initialState;
    DFBB_BuildOrderSearchParameters     m_params;
    DFBB_BuildOrderSearchResults        m_results;
    Timer                               m_searchTimer;

    std::vector<ActionType>             m_relevantActions;
    std::vector<size_t>                 m_actionOrderRanks;
    std::vector<size_t>                 m_stateOrderRanks;
    std::vector<SearchNode>             m_nodes;
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, QueueEntryCompare> m_openQueue;
    std::unordered_map<std::string, int> m_bestStateFinish;

    int                                 m_searchTimeLimitMS;
    bool                                m_printNewBest;
    bool                                m_smartSearch;
    ActionOrderingType                  m_ordering;
    bool                                m_useRepetitions;
    bool                                m_useIncreasingRepetitions;
    bool                                m_useLandmarkLowerBound;
    bool                                m_useResourceLowerBound;
    bool                                m_useAlwaysMakeWorkers;
    bool                                m_useSupplyBounding;
    double                              m_supplyBoundingThreshold;
    size_t                              m_sequence;

    void calculateSearchSettings();
    void initializeParameters();
    void initializeActionOrdering();
    void initializeIncumbent();
    void setPrerequisiteGoalMax();
    void recurseOverStrictDependencies(const ActionType & actionType);
    void setRelevantActions();
    void setRepetitions();

    size_t calculateSupplyProvidersRequired();
    size_t calculateRefineriesRequired();
    size_t getRepetitions(const GameState & state, const ActionType & actionType);
    bool isTimeOut();
    bool shouldPruneAction(const GameState & state, const ActionType & actionType);
    bool shouldRememberState(const GameState & state);
    int estimateLowerBound(const GameState & state);
    std::string getStateKey(const GameState & state) const;
    void generateLegalActions(const GameState & state, ActionSet & legalActions);
    void applyOrdering(const GameState & state, ActionSet & legalActions);
    void pushNode(const GameState & state, const BuildOrder & buildOrder);
    void updateBestSolution(const GameState & state, const BuildOrder & buildOrder);

    const RaceID getRace() const;

public:
    AStarBuildOrderSearch();

    void setGoal(const BuildOrderSearchGoal & goal);
    void setState(const GameState & state);
    void setTimeLimit(int ms);
    void setPrintNewBest(bool printNewBest);
    void setSmartSearch(bool smartSearch);
    void setOrdering(ActionOrderingType ordering);
    void setRelevantActions(const std::vector<ActionType> & relevantActions);
    void setUseRepetitions(bool val);
    void setUseIncreasingRepetitions(bool val);
    void setUseLandmarkLowerBound(bool val);
    void setUseResourceLowerBound(bool val);
    void setUseAlwaysMakeWorkers(bool val);
    void setUseSupplyBounding(bool val);
    void setSupplyBoundingThreshold(double val);

    void search();

    const DFBB_BuildOrderSearchResults & getResults() const;
};

}
