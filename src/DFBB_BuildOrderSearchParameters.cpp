#include "DFBB_BuildOrderSearchParameters.h"

using namespace BOSS;

// alternate constructor
DFBB_BuildOrderSearchParameters::DFBB_BuildOrderSearchParameters()
    : m_useRepetitions(true)
    , m_useIncreasingRepetitions(false)
    , m_useAlwaysMakeWorkers(false)
    , m_useSupplyBounding(false)
    , m_supplyBoundingThreshold(1)
    , m_useLandmarkLowerBoundHeuristic(true)
    , m_useResourceLowerBoundHeuristic(true)
    , m_searchTimeLimit(0)
    , m_initialUpperBound(0)
    , m_repetitionValues(ActionTypes::GetAllActionTypes().size(), 1)
    , m_repetitionThresholds(ActionTypes::GetAllActionTypes().size(), 0)
{
    
}

void DFBB_BuildOrderSearchParameters::setRepetitions(const ActionType & a, const size_t & repetitions)
{ 
    BOSS_ASSERT(a.getID() >= 0 && a.getID() < m_repetitionValues.size(), "Action type not valid");

    m_repetitionValues[a.getID()] = repetitions; 
}

void DFBB_BuildOrderSearchParameters::setRepetitionThreshold(const ActionType & a, const size_t & thresh)	
{ 
    BOSS_ASSERT(a.getID() >= 0 && a.getID() < m_repetitionThresholds.size(), "Action type not valid");

    m_repetitionThresholds[a.getID()] = thresh; 
}

const size_t & DFBB_BuildOrderSearchParameters::getRepetitions(const ActionType & a)
{ 
    BOSS_ASSERT(a.getID() >= 0 && a.getID() < m_repetitionValues.size(), "Action type not valid");

    return m_repetitionValues[a.getID()]; 
}

const size_t & DFBB_BuildOrderSearchParameters::getRepetitionThreshold(const ActionType & a)				
{ 
    BOSS_ASSERT(a.getID() >= 0 && a.getID() < m_repetitionThresholds.size(), "Action type not valid");

    return m_repetitionThresholds[a.getID()]; 
}



std::string DFBB_BuildOrderSearchParameters::toString() const
{
    std::stringstream ss;

    ss << "\n\nSearch Parameter Information\n\n";

    ss << (m_useRepetitions ?                    "\tUSE      Repetitions\n" : "");
    ss << (m_useIncreasingRepetitions ?          "\tUSE      Increasing Repetitions\n" : "");
    ss << (m_useLandmarkLowerBoundHeuristic ?    "\tUSE      Landmark Lower Bound\n" : "");
    ss << (m_useResourceLowerBoundHeuristic ?    "\tUSE      Resource Lower Bound\n" : "");
    ss << (m_useAlwaysMakeWorkers ?              "\tUSE      Always Make Workers\n" : "");
    ss << (m_useSupplyBounding ?                 "\tUSE      Supply Bounding\n" : "");
    ss << ("\n");

    for (ActionID a(0); a < m_repetitionValues.size(); ++a)
    {
        if (m_repetitionValues[a] != 1)
        {
            ss << "\tREP " << m_repetitionValues[a] << " " << ActionTypes::GetAllActionTypes()[a].getName() << "\n";
        }
    }

    for (ActionID a(0); a < m_repetitionThresholds.size(); ++a)
    {
        if (m_repetitionThresholds[a] != 0)
        {
            ss << "\tTHR " << m_repetitionThresholds[a] << " " << ActionTypes::GetAllActionTypes()[a].getName() << "\n";
        }
    }

    ss << "\n\n" << m_goal.toString();
    ss << "\n\n" << m_initialState.toString();

    for (size_t i(0); i < m_relevantActions.size(); ++i)
    {
        ss << "Relevant:   " << m_relevantActions[i].getName() << "\n";
    }

    //ss << "\n\n" << m_initialState.getUnitData().getBuildingData().toString();

    return ss.str();
}