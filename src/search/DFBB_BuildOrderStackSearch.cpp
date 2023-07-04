#include "DFBB_BuildOrderStackSearch.h"
#include "Tools.h"
#include "ActionSet.h"

using namespace BOSS;

DFBB_BuildOrderStackSearch::DFBB_BuildOrderStackSearch(const DFBB_BuildOrderSearchParameters & p)
    : m_params(p)
    , m_depth(0)
    , m_firstSearch(true)
    , m_wasInterrupted(false)
    , m_stack(100, StackData())
{
    
}

void DFBB_BuildOrderStackSearch::setTimeLimit(double ms)
{
    m_params.m_searchTimeLimit = ms;
}

// function which is called to do the actual search
void DFBB_BuildOrderStackSearch::search()
{
    m_searchTimer.start();

    if (!m_results.solved)
    {
        if (m_firstSearch)
        {
            m_results.upperBound = m_params.m_initialUpperBound ? m_params.m_initialUpperBound : Tools::GetUpperBound(m_params.m_initialState, m_params.m_goal);
            
            // add one frame to the upper bound so our strictly lesser than check still works if we have an exact upper bound
            m_results.upperBound += 1;

            m_stack[0].state = m_params.m_initialState;
            m_firstSearch = false;
            //BWAPI::Broodwar->printf("Upper bound is %d", m_results.upperBound);
            //std::cout << "Upper bound is: " << m_results.upperBound << std::endl;
        }

        try 
        {
            // search on the initial state
            DFBB();

            m_results.timedOut = false;
        }
        catch (int e) 
        {
            if (e == DFBB_TIMEOUT_EXCEPTION)
            {
                //BWAPI::Broodwar->printf("I timed out!");
                m_results.timedOut = true;
            }
        }
        
        double ms = m_searchTimer.getElapsedTimeInMilliSec();
        m_results.solved = !m_results.timedOut;
        m_results.timeElapsed = ms;
    }
}

const BuildOrderSearchResults & DFBB_BuildOrderStackSearch::getResults() const
{
    return m_results;
}

void DFBB_BuildOrderStackSearch::generateLegalActions(const GameState & state, ActionSet & legalActions)
{
    legalActions.clear();
    BuildOrderSearchGoal & goal = m_params.m_goal;
    const ActionType & worker = ActionTypes::GetWorker(state.getRace());
    
    // add all legal relevant actions that are in the goal
    for (size_t a(0); a < m_params.m_relevantActions.size(); ++a)
    {
        const ActionType & actionType = m_params.m_relevantActions[a];
        const std::string & actionName = actionType.getName();
        const size_t numTotal = state.getNumTotal(actionType);

        if (state.isLegal(actionType))
        {
            // if there's none of this action in the goal it's not legal
            if (!goal.getGoal(actionType) && !goal.getGoalMax(actionType))
            {
                continue;
            }

            // if we already have more than the goal it's not legal
            if (goal.getGoal(actionType) && (numTotal >= goal.getGoal(actionType)))
            {
                continue;
            }

            // if we already have more than the goal max it's not legal
            if (goal.getGoalMax(actionType) && (numTotal >= goal.getGoalMax(actionType)))
            {
                continue;
            }
            
            legalActions.add(m_params.m_relevantActions[a]);
        }
    }

    // if we enabled the supply bounding flag
    if (m_params.m_useSupplyBounding)
    {
        size_t supplySurplus = state.getMaxSupply() + state.getSupplyInProgress() - state.getCurrentSupply();
        size_t threshold = (size_t)(ActionTypes::GetSupplyProvider(state.getRace()).supplyProvided() * m_params.m_supplyBoundingThreshold);

        if (supplySurplus >= threshold)
        {
            legalActions.remove(ActionTypes::GetSupplyProvider(state.getRace()));
        }
    }
    
    // if we enabled the always make workers flag, and workers are legal
    if (m_params.m_useAlwaysMakeWorkers && legalActions.contains(worker))
    {
        bool actionLegalBeforeWorker = false;
        ActionSet legalEqualWorker;
        int workerReady = state.whenCanBuild(worker);

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

size_t DFBB_BuildOrderStackSearch::getRepetitions(const GameState & state, const ActionType & a)
{
    // set the repetitions if we are using repetitions, otherwise set to 1
    int repeat = m_params.m_useRepetitions ? m_params.getRepetitions(a) : 1;

    // if we are using increasing repetitions
    if (m_params.m_useIncreasingRepetitions)
    {
        // if we don't have the threshold amount of units, use a repetition value of 1
        repeat = state.getNumTotal(a) >= m_params.getRepetitionThreshold(a) ? repeat : 1;
    }

    // make sure we don't repeat to more than we need for this unit type
    if (m_params.m_goal.getGoal(a))
    {
        repeat = std::min(repeat, (int)m_params.m_goal.getGoal(a) - (int)state.getNumTotal(a));
    }
    else if (m_params.m_goal.getGoalMax(a))
    {
        repeat = std::min(repeat, (int)m_params.m_goal.getGoalMax(a) - (int)state.getNumTotal(a));
    }
    
    return repeat;
}

bool DFBB_BuildOrderStackSearch::isTimeOut()
{
    return (m_params.m_searchTimeLimit && (m_results.nodesExpanded % 200 == 0) && (m_searchTimer.getElapsedTimeInMilliSec() > m_params.m_searchTimeLimit));
}

void DFBB_BuildOrderStackSearch::updateResults(const GameState & state)
{
    int finishTime = state.getLastActionFinishTime();

    // new best solution
    if (finishTime < m_results.upperBound)
    {
        m_results.timeElapsed = m_searchTimer.getElapsedTimeInMilliSec();
        m_results.upperBound = finishTime;
        m_results.solutionFound = true;
        m_results.finalState = state;
        m_results.buildOrder = m_buildOrder;

        //_results.printResults(true);
    }
}

#define ACTION_TYPE     m_stack[m_depth].currentActionType
#define STATE           m_stack[m_depth].state
#define CHILD_STATE     m_stack[m_depth+1].state
#define CHILD_NUM       m_stack[m_depth].currentChildIndex
#define LEGAL_ACTIONS   m_stack[m_depth].legalActions
#define REPETITIONS     m_stack[m_depth].repetitionValue
#define COMPLETED_REPS  m_stack[m_depth].completedRepetitions

#define DFBB_CALL_RETURN  if (m_depth == 0) { return; } else { --m_depth; goto SEARCH_RETURN; }
#define DFBB_CALL_RECURSE { ++m_depth; goto SEARCH_BEGIN; }

// recursive function which does all search logic
void DFBB_BuildOrderStackSearch::DFBB()
{
    int actionFinishTime = 0;
    int heuristicTime = 0;
    int maxHeuristic = 0;

SEARCH_BEGIN:

    m_results.nodesExpanded++;

    if (isTimeOut())
    {
        throw DFBB_TIMEOUT_EXCEPTION;
    }

    generateLegalActions(STATE, LEGAL_ACTIONS);
    for (CHILD_NUM = 0; CHILD_NUM < LEGAL_ACTIONS.size(); ++CHILD_NUM)
    {
        ACTION_TYPE = LEGAL_ACTIONS[CHILD_NUM];

        actionFinishTime = STATE.whenCanBuild(ACTION_TYPE) + ACTION_TYPE.buildTime();
        heuristicTime    = STATE.getCurrentFrame() + Tools::GetLowerBound(STATE, m_params.m_goal);
        maxHeuristic     = (actionFinishTime > heuristicTime) ? actionFinishTime : heuristicTime;

        if (maxHeuristic > m_results.upperBound)
        {
            continue;
        }

        REPETITIONS = getRepetitions(STATE, ACTION_TYPE);
        BOSS_ASSERT(REPETITIONS > 0, "Can't have zero repetitions!");
                
        // do the action as many times as legal to to 'repeat'
        CHILD_STATE = STATE;
        COMPLETED_REPS = 0;
        for (; COMPLETED_REPS < REPETITIONS; ++COMPLETED_REPS)
        {
            if (CHILD_STATE.isLegal(ACTION_TYPE))
            {
                m_buildOrder.add(ACTION_TYPE);
                CHILD_STATE.doAction(ACTION_TYPE);
            }
            else
            {
                break;
            }
        }

        if (m_params.m_goal.isAchievedBy(CHILD_STATE))
        {
            updateResults(CHILD_STATE);
        }
        else
        {
            DFBB_CALL_RECURSE;
        }

SEARCH_RETURN:

        for (size_t r(0); r < COMPLETED_REPS; ++r)
        {
            m_buildOrder.pop_back();
        }
    }

    DFBB_CALL_RETURN;
}