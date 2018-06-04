#pragma once

#include "Common.h"
#include "Timer.hpp"
#include "Eval.h"
#include "BuildOrder.h"
#include "CombatSearch.h"
#include "CombatSearchParameters.h"
#include "CombatSearchResults.h"
#include "CombatSearch_IntegralData.h"

namespace BOSS
{

class CombatSearch_Integral : public CombatSearch
{
    CombatSearch_IntegralData   m_integral;

	virtual void                recurse(const GameState & s, size_t depth);

public:
	
	CombatSearch_Integral(const CombatSearchParameters p = CombatSearchParameters());
	
    virtual void printResults();
    virtual void writeResultsFile(const std::string & dir, const std::string & filename);
};

}