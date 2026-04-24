#include "AStarBuildOrderSearch.h"
#include "NaiveBuildOrderSearch.h"
#include "Tools.h"

#include <algorithm>
#include <cmath>
#include <limits>

using namespace BOSS;

bool AStarBuildOrderSearch::QueueEntryCompare::operator()(const QueueEntry & lhs, const QueueEntry & rhs) const
{
    if (lhs.priority != rhs.priority)
    {
        return lhs.priority > rhs.priority;
    }

    if (lhs.finishTime != rhs.finishTime)
    {
        return lhs.finishTime > rhs.finishTime;
    }

    return lhs.sequence > rhs.sequence;
}

AStarBuildOrderSearch::AStarBuildOrderSearch()
    : m_searchTimeLimitMS(30000)
    , m_printNewBest(false)
    , m_smartSearch(true)
    , m_ordering(ActionOrderingType::None)
    , m_useRepetitions(true)
    , m_useIncreasingRepetitions(true)
    , m_useLandmarkLowerBound(true)
    , m_useResourceLowerBound(true)
    , m_useAlwaysMakeWorkers(true)
    , m_useSupplyBounding(true)
    , m_supplyBoundingThreshold(1.5)
    , m_sequence(0)
{
}

void AStarBuildOrderSearch::setGoal(const BuildOrderSearchGoal & goal)                         { m_goal = goal; }
void AStarBuildOrderSearch::setState(const GameState & state)                                 { m_initialState = state; }
void AStarBuildOrderSearch::setTimeLimit(int ms)                                              { m_searchTimeLimitMS = ms; }
void AStarBuildOrderSearch::setPrintNewBest(bool printNewBest)                                { m_printNewBest = printNewBest; }
void AStarBuildOrderSearch::setSmartSearch(bool smartSearch)                                  { m_smartSearch = smartSearch; }
void AStarBuildOrderSearch::setOrdering(ActionOrderingType ordering)                          { m_ordering = ordering; }
void AStarBuildOrderSearch::setRelevantActions(const std::vector<ActionType> & relevantActions){ m_relevantActions = relevantActions; }
void AStarBuildOrderSearch::setUseRepetitions(bool val)                                       { m_useRepetitions = val; }
void AStarBuildOrderSearch::setUseIncreasingRepetitions(bool val)                             { m_useIncreasingRepetitions = val; }
void AStarBuildOrderSearch::setUseLandmarkLowerBound(bool val)                                { m_useLandmarkLowerBound = val; }
void AStarBuildOrderSearch::setUseResourceLowerBound(bool val)                                { m_useResourceLowerBound = val; }
void AStarBuildOrderSearch::setUseAlwaysMakeWorkers(bool val)                                 { m_useAlwaysMakeWorkers = val; }
void AStarBuildOrderSearch::setUseSupplyBounding(bool val)                                    { m_useSupplyBounding = val; }
void AStarBuildOrderSearch::setSupplyBoundingThreshold(double val)                            { m_supplyBoundingThreshold = val; }

void AStarBuildOrderSearch::search()
{
    BOSS_ASSERT(m_initialState.getRace() != Races::None, "Must set initial state before A* search");
    BOSS_ASSERT(m_goal.hasGoal(), "Must set goal before A* search");

    m_results = DFBB_BuildOrderSearchResults();
    m_nodes.clear();
    m_bestStateFinish.clear();
    m_openQueue = std::priority_queue<QueueEntry, std::vector<QueueEntry>, QueueEntryCompare>();
    m_sequence = 0;

    m_searchTimer.start();
    initializeParameters();
    initializeActionOrdering();
    initializeIncumbent();

    if (m_params.m_goal.isAchievedBy(m_initialState))
    {
        updateBestSolution(m_initialState, BuildOrder());
        m_results.solved = true;
        m_results.timeElapsed = m_searchTimer.getElapsedTimeInMilliSec();
        return;
    }

    pushNode(m_initialState, BuildOrder());

    while (!m_openQueue.empty())
    {
        if (isTimeOut())
        {
            m_results.timedOut = true;
            break;
        }

        const QueueEntry entry = m_openQueue.top();
        m_openQueue.pop();

        if (m_results.solutionFound && entry.priority >= m_results.upperBound)
        {
            break;
        }

        const GameState state = m_nodes[entry.nodeIndex].state;
        const BuildOrder buildOrder = m_nodes[entry.nodeIndex].buildOrder;

        if (m_params.m_goal.isAchievedBy(state))
        {
            updateBestSolution(state, buildOrder);
            continue;
        }

        m_results.nodesExpanded++;

        ActionSet legalActions;
        generateLegalActions(state, legalActions);
        applyOrdering(state, legalActions);

        for (size_t a(0); a < legalActions.size(); ++a)
        {
            const ActionType & actionType = legalActions[a];

            if (shouldPruneAction(state, actionType))
            {
                continue;
            }

            GameState childState(state);
            BuildOrder childBuildOrder(buildOrder);
            const size_t repetitions = getRepetitions(childState, actionType);
            BOSS_ASSERT(repetitions > 0, "Can't have zero repetitions!");

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

            const int childLowerBound = estimateLowerBound(childState);
            if (m_results.solutionFound && childLowerBound >= m_results.upperBound)
            {
                continue;
            }

            if (m_params.m_goal.isAchievedBy(childState))
            {
                updateBestSolution(childState, childBuildOrder);
                continue;
            }

            if (shouldRememberState(childState))
            {
                pushNode(childState, childBuildOrder);
            }
        }
    }

    m_results.solved = !m_results.timedOut;
    m_results.timeElapsed = m_searchTimer.getElapsedTimeInMilliSec();
}

void AStarBuildOrderSearch::initializeParameters()
{
    m_params = DFBB_BuildOrderSearchParameters();

    if (m_smartSearch)
    {
        calculateSearchSettings();
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
    m_params.m_relevantActions = m_relevantActions;
    m_params.m_searchTimeLimit = m_searchTimeLimitMS;
    m_params.m_printNewBest = m_printNewBest;
    m_params.m_ordering = m_ordering;
}

void AStarBuildOrderSearch::initializeActionOrdering()
{
    const size_t numActions = ActionTypes::GetAllActionTypes().size();
    m_actionOrderRanks.clear();
    m_stateOrderRanks.clear();

    if (m_params.m_ordering == ActionOrderingType::NaiveBuild)
    {
        NaiveBuildOrderSearch naiveSearch(m_params.m_initialState, m_params.m_goal);
        const BuildOrder & naiveBuildOrder = naiveSearch.solve();

        m_actionOrderRanks.assign(numActions, numActions);
        for (size_t i(0); i < naiveBuildOrder.size(); ++i)
        {
            const size_t id = naiveBuildOrder[i].getID();
            if (m_actionOrderRanks[id] == numActions)
            {
                m_actionOrderRanks[id] = i;
            }
        }
    }
    else if (m_params.m_ordering == ActionOrderingType::GoalFirst)
    {
        m_actionOrderRanks.assign(numActions, 1);
        for (size_t i(0); i < numActions; ++i)
        {
            if (m_params.m_goal.getGoal(ActionType(i)) > 0)
            {
                m_actionOrderRanks[i] = 0;
            }
        }
    }
    else if (m_params.m_ordering == ActionOrderingType::LeastBuilt ||
             m_params.m_ordering == ActionOrderingType::MostBuilt)
    {
        m_stateOrderRanks.resize(numActions, 0);
    }
}

void AStarBuildOrderSearch::initializeIncumbent()
{
    if (m_params.m_initialUpperBound > 0)
    {
        m_results.upperBound = m_params.m_initialUpperBound;
        return;
    }

    NaiveBuildOrderSearch naiveSearch(m_initialState, m_params.m_goal);
    const BuildOrder & naiveBuildOrder = naiveSearch.solve();

    GameState finalState(m_initialState);
    for (size_t i(0); i < naiveBuildOrder.size(); ++i)
    {
        if (!finalState.isLegal(naiveBuildOrder[i]))
        {
            return;
        }

        finalState.doAction(naiveBuildOrder[i]);
    }

    if (!m_params.m_goal.isAchievedBy(finalState))
    {
        return;
    }

    m_results.upperBound = finalState.getLastActionFinishTime();
    m_results.solutionFound = true;
    m_results.finalState = finalState;
    m_results.buildOrder = naiveBuildOrder;
}

void AStarBuildOrderSearch::pushNode(const GameState & state, const BuildOrder & buildOrder)
{
    const SearchNode node{ state, buildOrder, state.getLastActionFinishTime(), estimateLowerBound(state), m_sequence++ };
    const size_t nodeIndex = m_nodes.size();
    m_nodes.push_back(node);
    m_openQueue.push(QueueEntry{ nodeIndex, node.lowerBound, node.finishTime, node.sequence });
}

bool AStarBuildOrderSearch::shouldRememberState(const GameState & state)
{
    const std::string key = getStateKey(state);
    const int finishTime = state.getLastActionFinishTime();
    const auto it = m_bestStateFinish.find(key);
    if (it != m_bestStateFinish.end() && it->second <= finishTime)
    {
        return false;
    }

    m_bestStateFinish[key] = finishTime;
    return true;
}

std::string AStarBuildOrderSearch::getStateKey(const GameState & state) const
{
    return state.toStringAllUnits() + state.toStringResources();
}

int AStarBuildOrderSearch::estimateLowerBound(const GameState & state)
{
    int lowerBound = state.getLastActionFinishTime();
    if (m_params.m_useLandmarkLowerBoundHeuristic || m_params.m_useResourceLowerBoundHeuristic)
    {
        lowerBound = std::max(lowerBound, state.getCurrentFrame() + Tools::GetLowerBound(state, m_params.m_goal));
    }

    return lowerBound;
}

void AStarBuildOrderSearch::updateBestSolution(const GameState & state, const BuildOrder & buildOrder)
{
    const int finishTime = state.getLastActionFinishTime();
    if (finishTime <= 0)
    {
        return;
    }

    if (!m_results.solutionFound || finishTime < m_results.upperBound)
    {
        m_results.upperBound = finishTime;
        m_results.solutionFound = true;
        m_results.finalState = state;
        m_results.buildOrder = buildOrder;
        m_results.timeElapsed = m_searchTimer.getElapsedTimeInMilliSec();

        if (m_params.m_printNewBest)
        {
            std::cout << finishTime << "   " << buildOrder.getNameString(2) << std::endl;
        }
    }
}

void AStarBuildOrderSearch::generateLegalActions(const GameState & state, ActionSet & legalActions)
{
    legalActions.clear();
    BuildOrderSearchGoal & goal = m_params.m_goal;
    const ActionType & worker = ActionTypes::GetWorker(state.getRace());

    for (size_t a(0); a < m_params.m_relevantActions.size(); ++a)
    {
        const ActionType & actionType = m_params.m_relevantActions[a];
        const size_t numTotal = state.getNumTotal(actionType);

        if (!state.isLegal(actionType))
        {
            continue;
        }

        if (!goal.getGoal(actionType) && !goal.getGoalMax(actionType))
        {
            continue;
        }

        if (goal.getGoal(actionType) && (numTotal >= goal.getGoal(actionType)))
        {
            continue;
        }

        if (goal.getGoalMax(actionType) && (numTotal >= goal.getGoalMax(actionType)))
        {
            continue;
        }

        legalActions.add(actionType);
    }

    if (m_params.m_useSupplyBounding)
    {
        const size_t supplySurplus = state.getMaxSupply() + state.getSupplyInProgress() - state.getCurrentSupply();
        const size_t threshold = static_cast<size_t>(ActionTypes::GetSupplyProvider(state.getRace()).supplyProvided() * m_params.m_supplyBoundingThreshold);

        if (supplySurplus >= threshold)
        {
            legalActions.remove(ActionTypes::GetSupplyProvider(state.getRace()));
        }
    }

    if (m_params.m_useAlwaysMakeWorkers && legalActions.contains(worker))
    {
        bool actionLegalBeforeWorker = false;
        ActionSet legalEqualWorker;
        const int workerReady = state.whenCanBuild(worker);

        for (size_t a(0); a < legalActions.size(); ++a)
        {
            const ActionType & actionType = legalActions[a];
            const int whenCanPerformAction = state.whenCanBuild(actionType);
            if (whenCanPerformAction < workerReady)
            {
                actionLegalBeforeWorker = true;
                break;
            }

            if ((whenCanPerformAction == workerReady) && (actionType.mineralPrice() == worker.mineralPrice()))
            {
                legalEqualWorker.add(actionType);
            }
        }

        if (actionLegalBeforeWorker)
        {
            legalActions.remove(worker);
        }
        else
        {
            legalActions = legalEqualWorker;
        }
    }
}

void AStarBuildOrderSearch::applyOrdering(const GameState & state, ActionSet & legalActions)
{
    if (m_params.m_ordering == ActionOrderingType::LeastBuilt ||
        m_params.m_ordering == ActionOrderingType::MostBuilt)
    {
        for (size_t i(0); i < m_stateOrderRanks.size(); ++i)
        {
            m_stateOrderRanks[i] = state.getNumTotal(ActionType(i));
        }
        ActionOrdering::ApplyOrdering(legalActions, m_params.m_ordering, m_stateOrderRanks);
    }
    else
    {
        ActionOrdering::ApplyOrdering(legalActions, m_params.m_ordering, m_actionOrderRanks);
    }
}

bool AStarBuildOrderSearch::shouldPruneAction(const GameState & state, const ActionType & actionType)
{
    if (!m_results.solutionFound)
    {
        return false;
    }

    int maxHeuristic = state.whenCanBuild(actionType) + actionType.buildTime();
    if (m_params.m_useLandmarkLowerBoundHeuristic || m_params.m_useResourceLowerBoundHeuristic)
    {
        const int heuristicTime = state.getCurrentFrame() + Tools::GetLowerBound(state, m_params.m_goal);
        maxHeuristic = std::max(maxHeuristic, heuristicTime);
    }

    return maxHeuristic >= m_results.upperBound;
}

size_t AStarBuildOrderSearch::getRepetitions(const GameState & state, const ActionType & actionType)
{
    size_t repeat = m_params.m_useRepetitions ? m_params.getRepetitions(actionType) : 1;

    if (m_params.m_useIncreasingRepetitions)
    {
        repeat = state.getNumTotal(actionType) >= m_params.getRepetitionThreshold(actionType) ? repeat : 1;
    }

    if (m_params.m_goal.getGoal(actionType))
    {
        const size_t goalCount = m_params.m_goal.getGoal(actionType);
        const size_t currentCount = state.getNumTotal(actionType);
        const size_t remaining = goalCount > currentCount ? (goalCount - currentCount) : 0;
        repeat = std::min(repeat, remaining);
    }
    else if (m_params.m_goal.getGoalMax(actionType))
    {
        const size_t goalMaxCount = m_params.m_goal.getGoalMax(actionType);
        const size_t currentCount = state.getNumTotal(actionType);
        const size_t remaining = goalMaxCount > currentCount ? (goalMaxCount - currentCount) : 0;
        repeat = std::min(repeat, remaining);
    }

    return repeat;
}

bool AStarBuildOrderSearch::isTimeOut()
{
    return m_searchTimeLimitMS > 0 && (m_results.nodesExpanded % 200 == 0) && (m_searchTimer.getElapsedTimeInMilliSec() > m_searchTimeLimitMS);
}

void AStarBuildOrderSearch::calculateSearchSettings()
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
}

size_t AStarBuildOrderSearch::calculateRefineriesRequired()
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

void AStarBuildOrderSearch::setPrerequisiteGoalMax()
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

void AStarBuildOrderSearch::recurseOverStrictDependencies(const ActionType & actionType)
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

void AStarBuildOrderSearch::setRelevantActions()
{
    m_relevantActions.clear();
    for (const ActionType & actionType : ActionTypes::GetAllActionTypes())
    {
        if (m_goal.getGoalMax(actionType) > 0 || m_goal.getGoal(actionType) > 0)
        {
            m_relevantActions.push_back(actionType);
        }
    }
}

size_t AStarBuildOrderSearch::calculateSupplyProvidersRequired()
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

void AStarBuildOrderSearch::setRepetitions()
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

const RaceID AStarBuildOrderSearch::getRace() const
{
    return m_initialState.getRace();
}

const DFBB_BuildOrderSearchResults & AStarBuildOrderSearch::getResults() const
{
    return m_results;
}
