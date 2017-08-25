#include "BuildOrderSearchGoal.h"

using namespace BOSS;

BuildOrderSearchGoal::BuildOrderSearchGoal()
    : m_supplyRequiredVal(0)
    , m_goalUnits(ActionTypes::GetAllActionTypes().size())
    , m_goalUnitsMax(ActionTypes::GetAllActionTypes().size())
{
 
}

void BuildOrderSearchGoal::calculateSupplyRequired()
{
    m_supplyRequiredVal = 0;
    for (ActionID a(0); a<m_goalUnits.size(); ++a)
    {
        m_supplyRequiredVal += m_goalUnits[a] * ActionTypes::GetActionType(a).supplyCost();
    }
}

bool BuildOrderSearchGoal::operator == (const BuildOrderSearchGoal & g)
{
    for (ActionID a(0); a<m_goalUnits.size(); ++a)
    {
        if ((m_goalUnits[a] != g.m_goalUnits[a]) || (m_goalUnitsMax[a] != g.m_goalUnitsMax[a]))
        {
            return false;
        }
    }

    return true;
}

void BuildOrderSearchGoal::setGoal(const ActionType & a, const size_t num)
{
    BOSS_ASSERT(a.getID() >= 0 && a.getID() < m_goalUnits.size(), "Action type not valid");

    m_goalUnits[a.getID()] = num;

    calculateSupplyRequired();
}

bool BuildOrderSearchGoal::hasGoal() const
{
    for (ActionID a(0); a<m_goalUnits.size(); ++a)
    {
        if (m_goalUnits[a] > 0)
        {
            return true;
        }
    }

    return false;
}

void BuildOrderSearchGoal::setGoalMax(const ActionType & a, const size_t num)
{
    BOSS_ASSERT(a.getID() >= 0 && a.getID() < m_goalUnitsMax.size(), "Action type not valid");

    m_goalUnitsMax[a.getID()] = num;
}

size_t BuildOrderSearchGoal::getGoal(const ActionType & a) const
{
    BOSS_ASSERT(a.getID() >= 0 && a.getID() < m_goalUnits.size(), "Action type not valid");

    return m_goalUnits[a.getID()];
}

size_t BuildOrderSearchGoal::getGoalMax(const ActionType & a) const
{
    BOSS_ASSERT(a.getID() >= 0 && a.getID() < m_goalUnitsMax.size(), "Action type not valid");

    return m_goalUnitsMax[a.getID()];
}

size_t BuildOrderSearchGoal::supplyRequired() const
{
    return m_supplyRequiredVal;
}

std::string BuildOrderSearchGoal::toString() const
{
    std::stringstream ss;
    ss << "\nSearch Goal Information\n\n";

    for (ActionID a(0); a<m_goalUnits.size(); ++a)
    {
        if (m_goalUnits[a] > 0)
        {
            ss << "        REQ " << m_goalUnits[a] << " " <<  ActionTypes::GetActionType(a).getName() << "\n";
        }
    }

    for (ActionID a(0); a<m_goalUnitsMax.size(); ++a)
    {
        if (m_goalUnitsMax[a] > 0)
        {
            ss << "        MAX " << m_goalUnitsMax[a]  << " " << ActionTypes::GetActionType(a).getName() << "\n";
        }
    }

    return ss.str();
}

bool BuildOrderSearchGoal::isAchievedBy(const GameState & state)
{
    static const ActionType & Hatchery      = ActionTypes::GetActionType("Zerg_Hatchery");
    static const ActionType & Lair          = ActionTypes::GetActionType("Zerg_Lair");
    static const ActionType & Hive          = ActionTypes::GetActionType("Zerg_Hive");
    static const ActionType & Spire         = ActionTypes::GetActionType("Zerg_Spire");
    static const ActionType & GreaterSpire  = ActionTypes::GetActionType("Zerg_Greater_Spire");

    for (auto & actionType : ActionTypes::GetAllActionTypes())
    {
        size_t have = state.getNumTotal(actionType);

        if (state.getRace() == Races::Zerg)
        {
            if (actionType == Hatchery)
            {
                have += state.getNumTotal(Lair);
                have += state.getNumTotal(Hive);
            }
            else if (actionType == Lair)
            {
                have += state.getNumTotal(Hive);
            }
            else if (actionType == Spire)
            {
                have += state.getNumTotal(GreaterSpire);
            }
        }

        if (have < getGoal(actionType))
        {
            return false;
        }
    }

    return true;
}