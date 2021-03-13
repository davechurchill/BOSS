#pragma once

#include "Common.h"
#include "ActionType.h"


namespace BOSS
{

class BuildOrder
{
    std::vector<ActionType>	m_buildOrder;
    std::vector<size_t>		m_typeCount;

public:

    BuildOrder();

    void            add(const ActionType type);
    void            add(const ActionType type, const int amount);
    void            add(const BuildOrder & other);
    void            clear();
    void            pop_back();
    void            sortByPrerequisites();

    ActionType      operator [] (const size_t i) const;
    ActionType &    operator [] (const size_t i);

    size_t          size() const;
    size_t          getTypeCount(const ActionType type) const;
    bool            empty() const;

    std::string     getJSONString() const;
    std::string     getNumberedString() const;
    std::string     getIDString() const;
    std::string     getNameString(const size_t charactersPerName = 0) const;
};

}