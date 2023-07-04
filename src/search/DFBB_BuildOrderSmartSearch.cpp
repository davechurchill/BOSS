#include "DFBB_BuildOrderSmartSearch.h"
#include "ActionSet.h"

using namespace BOSS;

DFBB_BuildOrderSmartSearch::DFBB_BuildOrderSmartSearch() 
    : m_stackSearch(m_params)
    , m_searchTimeLimit(30)
{
}

void DFBB_BuildOrderSmartSearch::doSearch()
{
    BOSS_ASSERT(m_initialState.getRace() != Races::None, "Must set initial state before performing search");

    // if we are resuming a search
    if (m_stackSearch.getResults().timedOut)
    {
        m_stackSearch.setTimeLimit(m_searchTimeLimit);
        m_stackSearch.search();
    }
    else
    {
        calculateSearchSettings();
        m_params.m_goal                     = m_goal;
        m_params.m_initialState             = m_initialState;
        m_params.m_useRepetitions 			= true;
        m_params.m_useIncreasingRepetitions = true;
        m_params.m_useAlwaysMakeWorkers 	= true;
        m_params.m_useSupplyBounding 		= true;
        m_params.m_supplyBoundingThreshold  = 1.5;
        m_params.m_relevantActions          = m_relevantActions;
        m_params.m_searchTimeLimit          = m_searchTimeLimit;

        //BWAPI::Broodwar->printf("Constructing new search object time limit is %lf", _params.searchTimeLimit);
        m_stackSearch = DFBB_BuildOrderStackSearch(m_params);
        m_stackSearch.search();
    }

    m_results = m_stackSearch.getResults();

    if (m_results.solved && !m_results.solutionFound)
    {
        //std::cout << "No solution found better than naive, using naive build order" << std::endl;
        //_results.buildOrder = Tools::GetOptimizedNaiveBuildOrder(_params.initialState, _params.goal);
    }
}

void DFBB_BuildOrderSmartSearch::calculateSearchSettings()
{
    // set the max number of resource depots to what we have since no expanding is allowed
    const ActionType & resourceDepot    = ActionTypes::GetResourceDepot(getRace());
    const ActionType & refinery         = ActionTypes::GetRefinery(getRace());
    const ActionType & worker           = ActionTypes::GetWorker(getRace());
    const ActionType & supplyProvider   = ActionTypes::GetSupplyProvider(getRace());

    m_goal.setGoalMax(resourceDepot, m_initialState.getNumTotal(resourceDepot));

    // set the number of refineries
    m_goal.setGoalMax(refinery, std::min((size_t)3, calculateRefineriesRequired()));

    // set the maximum number of workers to an initial ridiculously high upper bound
    m_goal.setGoalMax(worker, std::min(m_initialState.getNumTotal(worker) + (size_t)20, (size_t)100));

    // set the number of supply providers required
    m_goal.setGoalMax(supplyProvider, calculateSupplyProvidersRequired());
        
    // set the maximums for all goal prerequisites
    setPrerequisiteGoalMax();

    // set relevant actions
    setRelevantActions();

    // set the repetitions
    setRepetitions();

    size_t maxWorkers = 45;
    if (m_goal.getGoal(worker) > maxWorkers)
    {
        m_goal.setGoal(worker, maxWorkers);
    }

    if (m_goal.getGoalMax(worker) > maxWorkers)
    {
        m_goal.setGoalMax(worker, maxWorkers);
    }
}

// calculates maximum number of refineries we'll need
size_t DFBB_BuildOrderSmartSearch::calculateRefineriesRequired()
{
    const ActionType & refinery      = ActionTypes::GetRefinery(getRace());
    const ActionType & resourceDepot = ActionTypes::GetResourceDepot(getRace());

    if (m_goal.getGoal(refinery))
    {
        return m_goal.getGoal(refinery);
    }

    // loop to check if we need gas
    bool gasRequired = false;
    for (const auto & actionType : ActionTypes::GetAllActionTypes())
    {
        if (m_goal.getGoal(actionType) > 0 && actionType.gasPrice() > 0)
        {
            gasRequired = true;
            break;
        }
    }

    return gasRequired ? m_initialState.getNumTotal(resourceDepot) : 0;
}

// handles all goalMax calculations for prerequisites of goal actions
void DFBB_BuildOrderSmartSearch::setPrerequisiteGoalMax()
{
    if (getRace() == Races::Protoss || getRace() == Races::Terran)
    {
        // for each unit in the goal vector
        for (const auto & actionType : ActionTypes::GetAllActionTypes())
        {
            // if we want one of these
            if (m_goal.getGoal(actionType) > 0)
            {
                // set goalMax for each strict dependency equal to 1
                recurseOverStrictDependencies(actionType);
            }
        }

        // vector which stores the number of goal units which are built by [index]
        std::vector<size_t> numGoalUnitsBuiltBy(ActionTypes::GetAllActionTypes().size(), 0);

        for (size_t a(0); a < numGoalUnitsBuiltBy.size(); ++a)
        {
            const ActionType & actionType(a);

            if (m_goal.getGoal(actionType) > 0)
            {
                // add this to the sum
                numGoalUnitsBuiltBy[actionType.whatBuilds().getID()] += m_goal.getGoal(actionType);

                // if it's in the goal, make sure it's in the max
                m_goal.setGoalMax(actionType, std::max(m_goal.getGoal(actionType), m_goal.getGoalMax(actionType)));
            }
        }

        size_t additionalProductionBuildingLimit = 2;

        for (size_t a(0); a < numGoalUnitsBuiltBy.size(); ++a)
        {
            const ActionType & actionType(a);

            // if it's not a resource depot
            if (!actionType.isDepot() && actionType.isBuilding())
            {
                // if this building produces units
                if (numGoalUnitsBuiltBy[actionType.getID()] > 0)
                {
                    // set the goal max to how many units
                    m_goal.setGoalMax(actionType, std::min(m_initialState.getNumTotal(actionType) + additionalProductionBuildingLimit, (size_t)numGoalUnitsBuiltBy[actionType.getID()]));
                }
            }
        }

        // set the upper bound on addons to the upper bound on the building that makes them
        for (const auto & actionType : ActionTypes::GetAllActionTypes())
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
        m_goal.setGoalMax(ActionTypes::GetActionType("Zerg_Spawning_Pool"), 1);
        m_goal.setGoalMax(ActionTypes::GetActionType("Zerg_Extractor"), 1);
        m_goal.setGoalMax(ActionTypes::GetActionType("Zerg_Lair"), 1);
        m_goal.setGoalMax(ActionTypes::GetActionType("Zerg_Spire"), 1);
        m_goal.setGoalMax(ActionTypes::GetActionType("Zerg_Hydralisk_Den"), 1);
    }
}

// recursively checks the tech tree of Action and sets each to have goalMax of 1
void DFBB_BuildOrderSmartSearch::recurseOverStrictDependencies(const ActionType & actionType)
{
    if (actionType.isDepot() || actionType.isWorker() || actionType.isSupplyProvider() || actionType.isRefinery())
    {
        return;
    }

    ActionSet recursivePrerequisites = actionType.getRecursivePrerequisiteActionCount();

    for (size_t a(0); a < recursivePrerequisites.size(); ++a)
    {
        const ActionType & actionType = recursivePrerequisites[a];

        if (actionType.isDepot() ||actionType.isWorker() || actionType.isSupplyProvider() || actionType.isRefinery())
        {
            continue;
        }

        m_goal.setGoalMax(actionType, std::max((size_t)1, m_goal.getGoalMax(actionType)));
    }
}

void DFBB_BuildOrderSmartSearch::setRelevantActions()
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

size_t DFBB_BuildOrderSmartSearch::calculateSupplyProvidersRequired()
{
    const ActionType & resourceDepot    = ActionTypes::GetResourceDepot(getRace());
    const ActionType & worker           = ActionTypes::GetWorker(getRace());
    const ActionType & supplyProvider   = ActionTypes::GetSupplyProvider(getRace());

    // calculate the upper bound on supply for this goal
    int supplyNeeded = m_goal.getGoalMax(worker) * worker.supplyCost();

    // for each prerequisite of things in the goal which aren't production facilities set one of
    for (const ActionType & actionType : ActionTypes::GetAllActionTypes())
    {
        // add the supply required for this number of goal units and all units currently made
        supplyNeeded += std::max(m_goal.getGoal(actionType), m_initialState.getNumTotal(actionType)) * actionType.supplyCost();
    }

    // set the upper bound on supply based on these values
    size_t supplyFromResourceDepots = m_initialState.getNumTotal(resourceDepot) * resourceDepot.supplyProvided();

    // take this away from the supply needed
    supplyNeeded -= supplyFromResourceDepots;

    // return the number of supply providers required
    return supplyNeeded > 0 ? (size_t)std::ceil((double)supplyNeeded / (double)supplyProvider.supplyProvided()) : 0;
}

void DFBB_BuildOrderSmartSearch::setRepetitions()
{
    //const ActionType & resourceDepot    = ActionTypes::GetResourceDepot(getRace());
    //const ActionType & refinery         = ActionTypes::GetRefinery(getRace());
    //const ActionType & worker           = ActionTypes::GetWorker(getRace());
    //const ActionType & supplyProvider   = ActionTypes::GetSupplyProvider(getRace());

    //_params.setRepetitions(supplyProvider, 1);
    //_params.setRepetitionThreshold(supplyProvider, 3);

    // for each action
    for (const ActionType & actionType : ActionTypes::GetAllActionTypes())
    {
        // if if want 4 or more of something that isn't supply providing
        if (!actionType.isSupplyProvider() && m_goal.getGoal(actionType) >= 5)
        {
            // set the repetitions to half of the value
            m_params.setRepetitions(actionType, std::min((size_t)4, (m_goal.getGoal(actionType) / 2)));
            m_params.setRepetitions(actionType.whatBuilds(), 2);
            m_params.setRepetitionThreshold(actionType.whatBuilds(), 1);
        }
    }
}

void DFBB_BuildOrderSmartSearch::addGoal(const ActionType & a, const size_t & count)
{
    m_goal.setGoal(a,count);
}

void DFBB_BuildOrderSmartSearch::setGoal(const BuildOrderSearchGoal & g)
{
    m_goal = g;    
}

void DFBB_BuildOrderSmartSearch::setState(const GameState & state)
{
    m_initialState = state;
}


void DFBB_BuildOrderSmartSearch::setTimeLimit(int n)
{
    m_searchTimeLimit = n;
}

void DFBB_BuildOrderSmartSearch::search()
{
    doSearch();
}

const BuildOrderSearchResults & DFBB_BuildOrderSmartSearch::getResults() const
{
    return m_results;
}

const DFBB_BuildOrderSearchParameters & DFBB_BuildOrderSmartSearch::getParameters()
{
    calculateSearchSettings();

    m_params.m_goal = m_goal;
    m_params.m_initialState                = m_initialState;
    m_params.m_useRepetitions 				= true;
    m_params.m_useIncreasingRepetitions 	= true;
    m_params.m_useAlwaysMakeWorkers 		= true;
    m_params.m_useSupplyBounding 			= true;

    return m_params;
}

void DFBB_BuildOrderSmartSearch::print()
{
    //initialState.printData();
    printf("\n\n");
}

const RaceID DFBB_BuildOrderSmartSearch::getRace() const
{
    return m_initialState.getRace();
}
