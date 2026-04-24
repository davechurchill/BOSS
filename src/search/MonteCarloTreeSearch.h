#pragma once

#include "Common.h"
#include "ActionSet.h"
#include "BuildOrder.h"
#include "BuildOrderSearchGoal.h"
#include "DFBB_BuildOrderSearchParameters.h"
#include "DFBB_BuildOrderSearchResults.h"
#include "GameState.h"
#include "Timer.hpp"

namespace BOSS
{

class MonteCarloTreeSearch
{
    struct Node
    {
        GameState               state;
        BuildOrder              buildOrder;
        std::vector<size_t>     children;
        std::vector<ActionType> untriedActions;
        int                     parent;
        bool                    actionsGenerated;
        size_t                  visits;
        double                  totalReward;

        Node();
        Node(const GameState & s, const BuildOrder & bo, int parentIndex);
    };

    BuildOrderSearchGoal                m_goal;
    GameState                           m_initialState;
    DFBB_BuildOrderSearchParameters     m_params;
    DFBB_BuildOrderSearchResults        m_results;
    Timer                               m_searchTimer;
    std::vector<Node>                   m_nodes;

    int                                 m_searchTimeLimitMS;
    size_t                              m_iterationLimit;
    double                              m_explorationConstant;
    bool                                m_printNewBest;
    bool                                m_useRepetitions;
    bool                                m_useIncreasingRepetitions;
    bool                                m_useLandmarkLowerBound;
    bool                                m_useResourceLowerBound;
    bool                                m_useAlwaysMakeWorkers;
    bool                                m_useSupplyBounding;
    double                              m_supplyBoundingThreshold;

    void calculateSearchSettings();
    void setPrerequisiteGoalMax();
    void recurseOverStrictDependencies(const ActionType & actionType);
    void setRelevantActions();
    void setRepetitions();

    size_t calculateSupplyProvidersRequired();
    size_t calculateRefineriesRequired();
    size_t selectNode();
    size_t expandNode(const size_t nodeIndex);
    size_t bestUCTChild(const size_t nodeIndex) const;
    size_t getRepetitions(const GameState & state, const ActionType & actionType);
    bool isTimeOut();
    bool shouldPruneAction(const GameState & state, const ActionType & actionType) const;
    bool evaluateNode(const size_t nodeIndex, int & finishTime, BuildOrder & buildOrder, GameState & finalState);
    void generateLegalActions(const GameState & state, std::vector<ActionType> & legalActions);
    void backpropagate(size_t nodeIndex, const double reward, const int finishTime);
    void updateBestSolution(const int finishTime, const BuildOrder & buildOrder, const GameState & finalState);

    const RaceID getRace() const;

public:
    MonteCarloTreeSearch();

    void setGoal(const BuildOrderSearchGoal & goal);
    void setState(const GameState & state);
    void setTimeLimit(int ms);
    void setIterationLimit(size_t iterations);
    void setExplorationConstant(double explorationConstant);
    void setPrintNewBest(bool printNewBest);
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
