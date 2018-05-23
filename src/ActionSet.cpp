#include "ActionSet.h"
#include "ActionType.h"

using namespace BOSS;

ActionSet::ActionSet()
{

}

size_t ActionSet::size() const
{
    return m_actions.size();
}

bool ActionSet::isEmpty() const
{
    return m_actions.empty();
}

bool ActionSet::contains(const ActionType & action) const
{
    return std::find(m_actions.begin(), m_actions.end(), action) != m_actions.end();
}

void ActionSet::add(const ActionType & action)
{
    if (!contains(action))
    {
        m_actions.push_back(action);
    }
}

void ActionSet::add(const ActionSet & set)
{
    for (const auto & val : set.m_actions)
    {
        add(val);
    }
}

void ActionSet::remove(const ActionType & action)
{
    m_actions.erase(std::remove(m_actions.begin(), m_actions.end(), action), m_actions.end());
}

void ActionSet::remove(const ActionSet & set)
{
    for (const auto & val : set.m_actions)
    {
        add(val);
    }
}

const std::string ActionSet::toString() const
{
    std::stringstream ss;
    
    for (const auto & val : m_actions)
    {
        ss << "PreReq: " << val.getName() << std::endl;
    }

    return ss.str();
}

void ActionSet::clear()
{
    m_actions.clear();
}

ActionType & ActionSet::operator[] (const size_t & index)
{
    return m_actions[index];
}

const ActionType & ActionSet::operator[] (const size_t & index) const
{
    return m_actions[index];
}