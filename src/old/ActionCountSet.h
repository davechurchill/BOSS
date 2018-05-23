#pragma once

#include "Common.h"

namespace BOSS
{

class ActionType;
class ActionCountSet
{
	std::vector<size_t> m_actionCounts;

public:

    ActionCountSet();

    size_t size() const;
    bool contains(const ActionType & action) const;
    size_t getCount(const ActionType & action) const;

    void addCount(const ActionType & action, const size_t & count = 1);
    void setCount(const ActionType & action, const size_t & count);

    const std::string toString() const;
};

}