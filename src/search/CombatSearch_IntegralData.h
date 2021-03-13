#pragma once

#include "Common.h"
#include "GameState.h"
#include "Eval.h"
#include "BuildOrder.h"

namespace BOSS
{
    
class IntegralData
{
public:
    double  eval;
    double  integral;
    int     timeAdded;

    IntegralData(double e, double i, int t)
        : eval(e)
        , integral(i)
        , timeAdded(t)
    {
    
    }
    
    IntegralData()
        : eval(0)
        , integral(0)
        , timeAdded(0)
    {
    
    }
};

class CombatSearch_IntegralData
{
    std::vector<IntegralData>       m_integralStack;

    std::vector<IntegralData>       m_bestIntegralStack;
    double                          m_bestIntegralValue;
    BuildOrder                      m_bestIntegralBuildOrder;

public:

    CombatSearch_IntegralData();

    void update(const GameState & state, const BuildOrder & buildOrder);
    void pop();

    void printIntegralData(const size_t index) const;
    void print() const;

    const BuildOrder & getBestBuildOrder() const;
};

}
