#include "ActionCountSet.h"
#include "ActionType.h"

using namespace BOSS;

ActionCountSet::ActionCountSet()
    : m_actionCounts(ActionTypes::GetAllActionTypes().size() + 1)
{

}

size_t ActionCountSet::size() const
{
    return m_actionCounts.size();
}

bool ActionCountSet::contains(const ActionType & action) const
{
    return getCount(action.getID()) > 0;
}

size_t ActionCountSet::getCount(const ActionType & action) const
{
    return m_actionCounts[action.getID()];
}
    
void ActionCountSet::addCount(const ActionType & action, const size_t & count)
{
    m_actionCounts[action.getID()] += count;
}

void ActionCountSet::setCount(const ActionType & action, const size_t & count)
{
    m_actionCounts[action.getID()] = count;
}

const std::string ActionCountSet::toString() const
{
    std::stringstream ss;
    
    for (size_t i(0); i<size(); ++i)
    {
        if (m_actionCounts[i] > 0)
        {
            ss << "    Prereq:   " << m_actionCounts[i] << " " << ActionTypes::GetAllActionTypes()[i].getName() << "\n";
        }
    }

    return ss.str();
}