#include "CombatSearchParameters.h"
#include "ActionType.h"

using namespace BOSS;

// alternate constructor
CombatSearchParameters::CombatSearchParameters()
    : m_useRepetitions                (true)
    , m_useIncreasingRepetitions      (false)
    , m_useWorkerCutoff               (false)
    , m_workerCutoff                  (1)
    , m_useAlwaysMakeWorkers          (false)
    , m_useSupplyBounding             (false)
    , m_supplyBoundingThreshold       (1)
    , m_useLandmarkLowerBoundHeuristic(false)
    , m_useResourceLowerBoundHeuristic(false)
    , m_searchTimeLimit               (0)
    , m_initialUpperBound             (0)
    , m_initialState                  ()
    , m_maxActions                    (ActionTypes::GetAllActionTypes().size(), -1)
    , m_repetitionValues              (ActionTypes::GetAllActionTypes().size(), 1)
    , m_repetitionThresholds          (ActionTypes::GetAllActionTypes().size(), 0)
    , m_printNewBest                  (false)
{
    
}

void CombatSearchParameters::setSearchTimeLimit(const double timeLimitMS)
{
    m_searchTimeLimit = timeLimitMS;
}

double CombatSearchParameters::getSearchTimeLimit() const
{
    return m_searchTimeLimit;
}

void CombatSearchParameters::setRelevantActions(const ActionSet & set)
{
    m_relevantActions = set;
}

const ActionSet & CombatSearchParameters::getRelevantActions() const
{
    return m_relevantActions;
}

void CombatSearchParameters::setInitialState(const GameState & s)
{
    m_initialState = s;
}

const GameState & CombatSearchParameters::getInitialState() const
{
    return m_initialState;
}

void CombatSearchParameters::setEnemyInitialState(const GameState & s)
{
    m_enemyInitialState = s;
}

const GameState & CombatSearchParameters::getEnemyInitialState() const
{
    return m_enemyInitialState;
}

void CombatSearchParameters::setMaxActions(const ActionType & a, int max)
{
    m_maxActions[a.getID()] = max;
}

void CombatSearchParameters::setOpeningBuildOrder(const BuildOrder & buildOrder)
{
    m_openingBuildOrder = buildOrder;
}

const BuildOrder & CombatSearchParameters::getOpeningBuildOrder() const
{
    return m_openingBuildOrder;
}

void CombatSearchParameters::setEnemyBuildOrder(const BuildOrder & buildOrder)
{
    m_enemyBuildOrder = buildOrder;
}

const BuildOrder & CombatSearchParameters::getEnemyBuildOrder() const
{
    return m_enemyBuildOrder;
}

void CombatSearchParameters::setRepetitions(const ActionType & a,int repetitions)
{ 
    m_repetitionValues[a.getID()] = repetitions; 
}

int CombatSearchParameters::getMaxActions(const ActionType & a) const
{ 
    return m_maxActions[a.getID()]; 
}

int CombatSearchParameters::getRepetitions(const ActionType & a) const
{ 
    return m_repetitionValues[a.getID()]; 
}

void CombatSearchParameters::setFrameTimeLimit(const int limit)
{
    m_frameTimeLimit = limit;
}

void CombatSearchParameters::setAlwaysMakeWorkers(const bool flag)
{
    m_useAlwaysMakeWorkers = flag;
}

const bool CombatSearchParameters::getAlwaysMakeWorkers() const
{
    return m_useAlwaysMakeWorkers;
}   

int CombatSearchParameters::getFrameTimeLimit() const
{
    return m_frameTimeLimit;
}



void CombatSearchParameters::print()
{
    printf("\n\nSearch Parameter Information\n\n");

    printf("%s", m_useRepetitions ?                    "\tUSE      Repetitions\n" : "");
    printf("%s", m_useIncreasingRepetitions ?          "\tUSE      Increasing Repetitions\n" : "");
    printf("%s", m_useWorkerCutoff ?                   "\tUSE      Worker Cutoff\n" : "");
    printf("%s", m_useLandmarkLowerBoundHeuristic ?    "\tUSE      Landmark Lower Bound\n" : "");
    printf("%s", m_useResourceLowerBoundHeuristic ?    "\tUSE      Resource Lower Bound\n" : "");
    printf("%s", m_useAlwaysMakeWorkers ?              "\tUSE      Always Make Workers\n" : "");
    printf("%s", m_useSupplyBounding ?                 "\tUSE      Supply Bounding\n" : "");
    printf("\n");

    //for (int a = 0; a < ACTIONS.size(); ++a)
    //{
    //    if (repetitionValues[a] != 1)
    //    {
    //        printf("\tREP %7d %s\n", repetitionValues[a], ACTIONS[a].getName().c_str());
    //    }
    //}

    //for (int a = 0; a < ACTIONS.size(); ++a)
    //{
    //    if (repetitionThresholds[a] != 0)
    //    {
    //        printf("\tTHR %7d %s\n", repetitionThresholds[a], ACTIONS[a].getName().c_str());
    //    }
    //}

    printf("\n\n");
}