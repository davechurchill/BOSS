#include "MonteCarloTreeSearch.h"
#include "NaiveBuildOrderSearch.h"
#include "Tools.h"

#include <algorithm>
#include <cmath>
#include <limits>

using namespace BOSS;

MonteCarloTreeSearch::Node::Node()
    : parent(-1)
    , actionsGenerated(false)
    , visits(0)
    , totalReward(0.0)
{
}

MonteCarloTreeSearch::Node::Node(const GameState & s, const BuildOrder & bo, int parentIndex)
    : state(s)
    , buildOrder(bo)
    , parent(parentIndex)
    , actionsGenerated(false)
    , visits(0)
    , totalReward(0.0)
{
}

MonteCarloTreeSearch::MonteCarloTreeSearch()
    : m_searchTimeLimitMS(30000)
    , m_iterationLimit(0)
    , m_explorationConstant(0.5)
    , m_printNewBest(false)
    , m_useRepetitions(true)
    , m_useIncreasingRepetitions(true)
    , m_useLandmarkLowerBound(true)
    , m_useResourceLowerBound(true)
    , m_useAlwaysMakeWorkers(true)
    , m_useSupplyBounding(true)
    , m_supplyBoundingThreshold(1.5)
{
}

void MonteCarloTreeSearch::setGoal(const BuildOrderSearchGoal & goal)          { m_goal = goal; }
void MonteCarloTreeSearch::setState(const GameState & state)                  { m_initialState = state; }
void MonteCarloTreeSearch::setTimeLimit(int ms)                               { m_searchTimeLimitMS = ms; }
void MonteCarloTreeSearch::setIterationLimit(size_t iterations)               { m_iterationLimit = iterations; }
void MonteCarloTreeSearch::setExplorationConstant(double explorationConstant) { m_explorationConstant = explorationConstant; }
void MonteCarloTreeSearch::setPrintNewBest(bool printNewBest)                 { m_printNewBest = printNewBest; }
void MonteCarloTreeSearch::setUseRepetitions(bool val)                        { m_useRepetitions = val; }
void MonteCarloTreeSearch::setUseIncreasingRepetitions(bool val)              { m_useIncreasingRepetitions = val; }
void MonteCarloTreeSearch::setUseLandmarkLowerBound(bool val)                 { m_useLandmarkLowerBound = val; }
void MonteCarloTreeSearch::setUseResourceLowerBound(bool val)                 { m_useResourceLowerBound = val; }
void MonteCarloTreeSearch::setUseAlwaysMakeWorkers(bool val)                  { m_useAlwaysMakeWorkers = val; }
void MonteCarloTreeSearch::setUseSupplyBounding(bool val)                     { m_useSupplyBounding = val; }
void MonteCarloTreeSearch::setSupplyBoundingThreshold(double val)             { m_supplyBoundingThreshold = val; }

void MonteCarloTreeSearch::search()
{
    BOSS_ASSERT(m_initialState.getRace() != Races::None, "Must set initial state before MCTS search");
    BOSS_ASSERT(m_goal.hasGoal(), "Must set goal before MCTS search");

    m_searchTimer.start();
    calculateSearchSettings();

    m_nodes.clear();
    m_results = DFBB_BuildOrderSearchResults();

    m_nodes.push_back(Node(m_initialState, BuildOrder(), -1));

    int finishTime = 0;
    BuildOrder buildOrder;
    GameState finalState;
    if (evaluateNode(0, finishTime, buildOrder, finalState))
    {
        updateBestSolution(finishTime, buildOrder, finalState);
        backpropagate(0, 1.0, finishTime);
    }

    while (true)
    {
        if (isTimeOut())
        {
            m_results.timedOut = true;
            break;
        }

        if (m_iterationLimit > 0 && m_results.nodesExpanded >= m_iterationLimit)
        {
            break;
        }

        const size_t selectedNode = selectNode();
        int nodeFinishTime = 0;
        BuildOrder nodeBuildOrder;
        GameState nodeFinalState;
        double reward = 0.0;

        if (evaluateNode(selectedNode, nodeFinishTime, nodeBuildOrder, nodeFinalState))
        {
            updateBestSolution(nodeFinishTime, nodeBuildOrder, nodeFinalState);
            const int baseline = std::max(1, m_results.upperBound);
            reward = static_cast<double>(baseline) / static_cast<double>(std::max(1, nodeFinishTime));
        }

        backpropagate(selectedNode, reward, nodeFinishTime);
        m_results.nodesExpanded++;

        if (m_nodes.size() == 1 && m_nodes.front().actionsGenerated && m_nodes.front().untriedActions.empty() && m_nodes.front().children.empty())
        {
            break;
        }
    }

    m_results.solved = m_results.solutionFound;
    m_results.timeElapsed = m_searchTimer.getElapsedTimeInMilliSec();
}

size_t MonteCarloTreeSearch::selectNode()
{
    size_t nodeIndex = 0;

    while (true)
    {
        if (m_goal.isAchievedBy(m_nodes[nodeIndex].state))
        {
            return nodeIndex;
        }

        if (!m_nodes[nodeIndex].actionsGenerated)
        {
            generateLegalActions(m_nodes[nodeIndex].state, m_nodes[nodeIndex].untriedActions);
            m_nodes[nodeIndex].actionsGenerated = true;
        }

        if (!m_nodes[nodeIndex].untriedActions.empty())
        {
            return expandNode(nodeIndex);
        }

        if (m_nodes[nodeIndex].children.empty())
        {
            return nodeIndex;
        }

        nodeIndex = bestUCTChild(nodeIndex);
    }
}

size_t MonteCarloTreeSearch::expandNode(const size_t nodeIndex)
{
    Node & node = m_nodes[nodeIndex];

    while (!node.untriedActions.empty())
    {
        const ActionType actionType = node.untriedActions.back();
        node.untriedActions.pop_back();

        if (shouldPruneAction(node.state, actionType))
        {
            continue;
        }

        GameState childState(node.state);
        BuildOrder childBuildOrder(node.buildOrder);
        const size_t repetitions = getRepetitions(childState, actionType);
        size_t completedRepetitions = 0;

        for (; completedRepetitions < repetitions; ++completedRepetitions)
        {
            if (!childState.isLegal(actionType))
            {
                break;
            }

            childBuildOrder.add(actionType);
            childState.doAction(actionType);
        }

        if (completedRepetitions == 0)
        {
            continue;
        }

        const size_t childIndex = m_nodes.size();
        m_nodes.push_back(Node(childState, childBuildOrder, static_cast<int>(nodeIndex)));
        m_nodes[nodeIndex].children.push_back(childIndex);
        return childIndex;
    }

    return nodeIndex;
}

size_t MonteCarloTreeSearch::bestUCTChild(const size_t nodeIndex) const
{
    const Node & node = m_nodes[nodeIndex];
    const double parentVisits = std::max<size_t>(1, node.visits);
    double bestScore = -std::numeric_limits<double>::infinity();
    size_t bestChild = node.children.front();

    for (const size_t childIndex : node.children)
    {
        const Node & child = m_nodes[childIndex];
        if (child.visits == 0)
        {
            return childIndex;
        }

        const double exploitation = child.totalReward / static_cast<double>(child.visits);
        const double exploration = m_explorationConstant * std::sqrt(std::log(parentVisits + 1.0) / static_cast<double>(child.visits));
        const double score = exploitation + exploration;

        if (score > bestScore)
        {
            bestScore = score;
            bestChild = childIndex;
        }
    }

    return bestChild;
}

bool MonteCarloTreeSearch::evaluateNode(const size_t nodeIndex, int & finishTime, BuildOrder & buildOrder, GameState & finalState)
{
    const Node & node = m_nodes[nodeIndex];

    if (m_goal.isAchievedBy(node.state))
    {
        finishTime = node.state.getLastActionFinishTime();
        buildOrder = node.buildOrder;
        finalState = node.state;
        return true;
    }

    NaiveBuildOrderSearch naiveSearch(node.state, m_goal);
    const BuildOrder & suffix = naiveSearch.solve();
    if (suffix.empty())
    {
        return false;
    }

    finalState = node.state;
    for (size_t i(0); i < suffix.size(); ++i)
    {
        if (!finalState.isLegal(suffix[i]))
        {
            return false;
        }

        finalState.doAction(suffix[i]);
    }

    if (!m_goal.isAchievedBy(finalState))
    {
        return false;
    }

    buildOrder = node.buildOrder;
    buildOrder.add(suffix);
    finishTime = finalState.getLastActionFinishTime();
    return true;
}

void MonteCarloTreeSearch::backpropagate(size_t nodeIndex, const double reward, const int finishTime)
{
    while (true)
    {
        Node & node = m_nodes[nodeIndex];
        node.visits++;
        node.totalReward += reward;
        if (node.parent < 0)
        {
            break;
        }

        nodeIndex = static_cast<size_t>(node.parent);
    }
}

void MonteCarloTreeSearch::updateBestSolution(const int finishTime, const BuildOrder & buildOrder, const GameState & finalState)
{
    if (finishTime <= 0)
    {
        return;
    }

    if (!m_results.solutionFound || finishTime < m_results.upperBound)
    {
        m_results.upperBound = finishTime;
        m_results.solutionFound = true;
        m_results.buildOrder = buildOrder;
        m_results.finalState = finalState;
        m_results.timeElapsed = m_searchTimer.getElapsedTimeInMilliSec();

        if (m_printNewBest)
        {
            std::cout << finishTime << "   " << buildOrder.getNameString(2) << std::endl;
        }
    }
}

void MonteCarloTreeSearch::generateLegalActions(const GameState & state, std::vector<ActionType> & legalActions)
{
    legalActions.clear();
    const ActionType & worker = ActionTypes::GetWorker(state.getRace());

    for (const ActionType & actionType : m_params.m_relevantActions)
    {
        const size_t numTotal = state.getNumTotal(actionType);

        if (!state.isLegal(actionType))
        {
            continue;
        }

        if (!m_goal.getGoal(actionType) && !m_goal.getGoalMax(actionType))
        {
            continue;
        }

        if (m_goal.getGoal(actionType) && numTotal >= m_goal.getGoal(actionType))
        {
            continue;
        }

        if (m_goal.getGoalMax(actionType) && numTotal >= m_goal.getGoalMax(actionType))
        {
            continue;
        }

        legalActions.push_back(actionType);
    }

    if (m_useSupplyBounding)
    {
        const size_t supplySurplus = state.getMaxSupply() + state.getSupplyInProgress() - state.getCurrentSupply();
        const size_t threshold = static_cast<size_t>(ActionTypes::GetSupplyProvider(state.getRace()).supplyProvided() * m_supplyBoundingThreshold);

        if (supplySurplus >= threshold)
        {
            legalActions.erase(std::remove(legalActions.begin(), legalActions.end(), ActionTypes::GetSupplyProvider(state.getRace())), legalActions.end());
        }
    }

    if (m_useAlwaysMakeWorkers && std::find(legalActions.begin(), legalActions.end(), worker) != legalActions.end())
    {
        bool actionLegalBeforeWorker = false;
        std::vector<ActionType> legalEqualWorker;
        const int workerReady = state.whenCanBuild(worker);

        for (const ActionType & actionType : legalActions)
        {
            const int whenCanPerformAction = state.whenCanBuild(actionType);
            if (whenCanPerformAction < workerReady)
            {
                actionLegalBeforeWorker = true;
                break;
            }

            if ((whenCanPerformAction == workerReady) && (actionType.mineralPrice() == worker.mineralPrice()))
            {
                legalEqualWorker.push_back(actionType);
            }
        }

        if (actionLegalBeforeWorker)
        {
            legalActions.erase(std::remove(legalActions.begin(), legalActions.end(), worker), legalActions.end());
        }
        else
        {
            legalActions = legalEqualWorker;
        }
    }

    std::reverse(legalActions.begin(), legalActions.end());
}

bool MonteCarloTreeSearch::shouldPruneAction(const GameState & state, const ActionType & actionType) const
{
    if (!m_results.solutionFound)
    {
        return false;
    }

    int maxHeuristic = state.whenCanBuild(actionType) + actionType.buildTime();

    if (m_useLandmarkLowerBound || m_useResourceLowerBound)
    {
        const int heuristicTime = state.getCurrentFrame() + Tools::GetLowerBound(state, m_goal);
        maxHeuristic = std::max(maxHeuristic, heuristicTime);
    }

    return maxHeuristic > m_results.upperBound;
}

size_t MonteCarloTreeSearch::getRepetitions(const GameState & state, const ActionType & actionType)
{
    size_t repeat = m_useRepetitions ? m_params.getRepetitions(actionType) : 1;

    if (m_useIncreasingRepetitions)
    {
        repeat = state.getNumTotal(actionType) >= m_params.getRepetitionThreshold(actionType) ? repeat : 1;
    }

    if (m_goal.getGoal(actionType))
    {
        const size_t goalCount = m_goal.getGoal(actionType);
        const size_t currentCount = state.getNumTotal(actionType);
        const size_t remaining = goalCount > currentCount ? (goalCount - currentCount) : 0;
        repeat = std::min(repeat, remaining);
    }
    else if (m_goal.getGoalMax(actionType))
    {
        const size_t goalMaxCount = m_goal.getGoalMax(actionType);
        const size_t currentCount = state.getNumTotal(actionType);
        const size_t remaining = goalMaxCount > currentCount ? (goalMaxCount - currentCount) : 0;
        repeat = std::min(repeat, remaining);
    }

    return std::max<size_t>(1, repeat);
}

bool MonteCarloTreeSearch::isTimeOut()
{
    return m_searchTimeLimitMS > 0 && m_searchTimer.getElapsedTimeInMilliSec() > m_searchTimeLimitMS;
}

void MonteCarloTreeSearch::calculateSearchSettings()
{
    const ActionType & resourceDepot    = ActionTypes::GetResourceDepot(getRace());
    const ActionType & refinery         = ActionTypes::GetRefinery(getRace());
    const ActionType & worker           = ActionTypes::GetWorker(getRace());
    const ActionType & supplyProvider   = ActionTypes::GetSupplyProvider(getRace());

    m_goal.setGoalMax(resourceDepot, m_initialState.getNumTotal(resourceDepot));
    m_goal.setGoalMax(refinery, std::min((size_t)3, calculateRefineriesRequired()));
    m_goal.setGoalMax(worker, std::min(m_initialState.getNumTotal(worker) + (size_t)20, (size_t)100));
    m_goal.setGoalMax(supplyProvider, calculateSupplyProvidersRequired());

    setPrerequisiteGoalMax();
    setRelevantActions();
    setRepetitions();

    const size_t maxWorkers = 45;
    if (m_goal.getGoal(worker) > maxWorkers)
    {
        m_goal.setGoal(worker, maxWorkers);
    }

    if (m_goal.getGoalMax(worker) > maxWorkers)
    {
        m_goal.setGoalMax(worker, maxWorkers);
    }

    m_params.m_goal = m_goal;
    m_params.m_initialState = m_initialState;
    m_params.m_useRepetitions = m_useRepetitions;
    m_params.m_useIncreasingRepetitions = m_useIncreasingRepetitions;
    m_params.m_useLandmarkLowerBoundHeuristic = m_useLandmarkLowerBound;
    m_params.m_useResourceLowerBoundHeuristic = m_useResourceLowerBound;
    m_params.m_useAlwaysMakeWorkers = m_useAlwaysMakeWorkers;
    m_params.m_useSupplyBounding = m_useSupplyBounding;
    m_params.m_supplyBoundingThreshold = m_supplyBoundingThreshold;
    m_params.m_searchTimeLimit = m_searchTimeLimitMS;
}

size_t MonteCarloTreeSearch::calculateRefineriesRequired()
{
    const ActionType & refinery      = ActionTypes::GetRefinery(getRace());
    const ActionType & resourceDepot = ActionTypes::GetResourceDepot(getRace());

    if (m_goal.getGoal(refinery))
    {
        return m_goal.getGoal(refinery);
    }

    bool gasRequired = false;
    for (const ActionType & actionType : ActionTypes::GetAllActionTypes())
    {
        if (m_goal.getGoal(actionType) > 0 && actionType.gasPrice() > 0)
        {
            gasRequired = true;
            break;
        }
    }

    return gasRequired ? m_initialState.getNumTotal(resourceDepot) : 0;
}

void MonteCarloTreeSearch::setPrerequisiteGoalMax()
{
    if (getRace() == Races::Protoss || getRace() == Races::Terran)
    {
        for (const ActionType & actionType : ActionTypes::GetAllActionTypes())
        {
            if (m_goal.getGoal(actionType) > 0)
            {
                recurseOverStrictDependencies(actionType);
            }
        }

        std::vector<size_t> numGoalUnitsBuiltBy(ActionTypes::GetAllActionTypes().size(), 0);
        for (size_t a(0); a < numGoalUnitsBuiltBy.size(); ++a)
        {
            const ActionType actionType(a);
            if (m_goal.getGoal(actionType) > 0)
            {
                numGoalUnitsBuiltBy[actionType.whatBuilds().getID()] += m_goal.getGoal(actionType);
                m_goal.setGoalMax(actionType, std::max(m_goal.getGoal(actionType), m_goal.getGoalMax(actionType)));
            }
        }

        const size_t additionalProductionBuildingLimit = 2;
        for (size_t a(0); a < numGoalUnitsBuiltBy.size(); ++a)
        {
            const ActionType actionType(a);
            if (!actionType.isDepot() && actionType.isBuilding() && numGoalUnitsBuiltBy[actionType.getID()] > 0)
            {
                m_goal.setGoalMax(actionType, std::min(m_initialState.getNumTotal(actionType) + additionalProductionBuildingLimit, numGoalUnitsBuiltBy[actionType.getID()]));
            }
        }

        for (const ActionType & actionType : ActionTypes::GetAllActionTypes())
        {
            if (actionType.isAddon() && m_goal.getGoalMax(actionType) > 0)
            {
                const ActionType & whatBuilds = actionType.whatBuilds();
                if (m_goal.getGoalMax(whatBuilds) > 0)
                {
                    m_goal.setGoalMax(actionType, m_goal.getGoalMax(whatBuilds));
                }
            }
        }
    }
    else if (getRace() == Races::Zerg)
    {
        m_goal.setGoalMax(ActionTypes::GetActionType("SpawningPool"), 1);
        m_goal.setGoalMax(ActionTypes::GetActionType("Extractor"), 1);
        m_goal.setGoalMax(ActionTypes::GetActionType("Lair"), 1);
        m_goal.setGoalMax(ActionTypes::GetActionType("Spire"), 1);
        m_goal.setGoalMax(ActionTypes::GetActionType("HydraliskDen"), 1);
    }
}

void MonteCarloTreeSearch::recurseOverStrictDependencies(const ActionType & actionType)
{
    if (actionType.isDepot() || actionType.isWorker() || actionType.isSupplyProvider() || actionType.isRefinery())
    {
        return;
    }

    ActionSet recursivePrerequisites = actionType.getRecursivePrerequisiteActionCount();
    for (size_t a(0); a < recursivePrerequisites.size(); ++a)
    {
        const ActionType & prereq = recursivePrerequisites[a];
        if (prereq.isDepot() || prereq.isWorker() || prereq.isSupplyProvider() || prereq.isRefinery())
        {
            continue;
        }

        m_goal.setGoalMax(prereq, std::max((size_t)1, m_goal.getGoalMax(prereq)));
    }
}

void MonteCarloTreeSearch::setRelevantActions()
{
    m_params.m_relevantActions.clear();
    for (const ActionType & actionType : ActionTypes::GetAllActionTypes())
    {
        if (m_goal.getGoalMax(actionType) > 0 || m_goal.getGoal(actionType) > 0)
        {
            m_params.m_relevantActions.push_back(actionType);
        }
    }
}

size_t MonteCarloTreeSearch::calculateSupplyProvidersRequired()
{
    const ActionType & resourceDepot    = ActionTypes::GetResourceDepot(getRace());
    const ActionType & worker           = ActionTypes::GetWorker(getRace());
    const ActionType & supplyProvider   = ActionTypes::GetSupplyProvider(getRace());

    long long supplyNeeded = static_cast<long long>(m_goal.getGoalMax(worker)) * worker.supplyCost();
    for (const ActionType & actionType : ActionTypes::GetAllActionTypes())
    {
        if (actionType == worker)
        {
            continue;
        }

        supplyNeeded += static_cast<long long>(std::max(m_goal.getGoal(actionType), m_initialState.getNumTotal(actionType))) * actionType.supplyCost();
    }

    const long long supplyFromResourceDepots = static_cast<long long>(m_initialState.getNumTotal(resourceDepot)) * resourceDepot.supplyProvided();
    supplyNeeded -= supplyFromResourceDepots;

    return supplyNeeded > 0 ? static_cast<size_t>(std::ceil(static_cast<double>(supplyNeeded) / static_cast<double>(supplyProvider.supplyProvided()))) : 0;
}

void MonteCarloTreeSearch::setRepetitions()
{
    for (const ActionType & actionType : ActionTypes::GetAllActionTypes())
    {
        if (!actionType.isSupplyProvider() && m_goal.getGoal(actionType) >= 5)
        {
            m_params.setRepetitions(actionType, std::min((size_t)4, (m_goal.getGoal(actionType) / 2)));
            m_params.setRepetitions(actionType.whatBuilds(), 2);
            m_params.setRepetitionThreshold(actionType.whatBuilds(), 1);
        }
    }
}

const RaceID MonteCarloTreeSearch::getRace() const
{
    return m_initialState.getRace();
}

const DFBB_BuildOrderSearchResults & MonteCarloTreeSearch::getResults() const
{
    return m_results;
}
