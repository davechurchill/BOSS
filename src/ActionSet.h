#pragma once

#include "Common.h"
#include <vector>

namespace BOSS
{

class ActionType;
class ActionSet
{
	std::vector<ActionType> m_actions;

public:

    ActionSet();

    size_t size() const;

    bool isEmpty() const;
    bool contains(const ActionType & action) const;
    void add(const ActionType & action);
    void add(const ActionSet & set);
    void remove(const ActionType & action);
    void remove(const ActionSet & set);
    void clear();

          ActionType & operator[] (const size_t & index);
    const ActionType & operator[] (const size_t & index) const;

    const std::string toString() const;
};

}