#include "BuildOrder.h"

using namespace BOSS;

BuildOrder::BuildOrder()
    : m_typeCount(128, 0)
{

}

void BuildOrder::add(const ActionType & type)
{
    BOSS_ASSERT((m_buildOrder.size() == 0) || (type.getRace() == m_buildOrder.back().getRace()), "Cannot have a build order with multiple races");

    m_buildOrder.push_back(type);
    m_typeCount[type.getID()]++;
}

void BuildOrder::add(const ActionType & type, const int & amount)
{
    for (int i(0); i < amount; ++i)
    {
        add(type);
    }
}

void BuildOrder::add(const BuildOrder & other)
{
    for (size_t i(0); i < other.size(); ++i)
    {
        add(other[i]);
    }
}

void BuildOrder::clear()
{
    m_buildOrder.clear();
    m_typeCount.clear();
}

const bool BuildOrder::empty() const
{
    return size() == 0;
}


const size_t BuildOrder::getTypeCount(const ActionType & type) const
{
    if (empty())
    {
        return 0;
    }

    BOSS_ASSERT(type.getRace() == m_buildOrder[0].getRace(), "Trying to get type count of a different race type");

    return m_typeCount[type.getID()];
}

void BuildOrder::pop_back()
{
    m_buildOrder.pop_back();
}

const ActionType & BuildOrder::operator [] (const size_t & i) const
{
    return m_buildOrder[i];
}

ActionType & BuildOrder::operator [] (const size_t & i) 
{
    return m_buildOrder[i];
}

const size_t BuildOrder::size() const
{
    return m_buildOrder.size();
}

void BuildOrder::sortByPrerequisites()
{
    for (size_t i(0); i < m_buildOrder.size() - 1; ++i)
    {
        for (size_t j(i + 1); j < m_buildOrder.size(); ++j)
        {
            const auto & recursivePre = m_buildOrder[i].getRecursivePrerequisiteActionCount();

            if (recursivePre.contains(m_buildOrder[j]))
            {
                std::swap(m_buildOrder[i], m_buildOrder[j]);
            }
        }
    }
}

std::string BuildOrder::getJSONString() const
{
    std::stringstream ss;

    ss << "\"Build Order\" : [";

    for (size_t i(0); i < m_buildOrder.size(); ++i)
    {
        ss << "\"" << m_buildOrder[i].getName() << "\"" << (i < m_buildOrder.size() - 1 ? ", " : "");
    }

    ss << "]";

    return ss.str();
}

std::string BuildOrder::getNumberedString() const
{
    std::stringstream ss;

    for (size_t i(0); i < m_buildOrder.size(); ++i)
    {
        std::stringstream num;
        num << i;
        while (num.str().length() < 5)
        {
            num << " ";
        }

        ss << num.str() << m_buildOrder[i].getName() << std::endl;
    }

    return ss.str();
}

std::string BuildOrder::getIDString() const
{
    std::stringstream ss;

    for (size_t i(0); i < m_buildOrder.size(); ++i)
    {
        ss << (int)m_buildOrder[i].getID() << " ";
    }

    return ss.str();
}

std::string BuildOrder::getNameString(const size_t charactersPerName) const
{
    std::stringstream ss;

    for (size_t i(0); i < m_buildOrder.size(); ++i)
    {
        std::string name = charactersPerName == 0 ? m_buildOrder[i].getName() : m_buildOrder[i].getName().substr(0, charactersPerName);

        ss << name << " ";
    }

    return ss.str();
}