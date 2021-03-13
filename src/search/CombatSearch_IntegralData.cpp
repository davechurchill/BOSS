#include "CombatSearch_IntegralData.h"

using namespace BOSS;

CombatSearch_IntegralData::CombatSearch_IntegralData()
    : m_bestIntegralValue(0)
{
    m_integralStack.push_back(IntegralData(0,0,0));
}

void CombatSearch_IntegralData::update(const GameState & state, const BuildOrder & buildOrder)
{
    double value = Eval::ArmyTotalResourceSum(state);
    double timeElapsed = state.getCurrentFrame() - m_integralStack.back().timeAdded; 
    double valueToAdd = m_integralStack.back().eval * timeElapsed;
    IntegralData entry(value, m_integralStack.back().integral + valueToAdd, state.getCurrentFrame());
    m_integralStack.push_back(entry);

    // we have found a new best if:
    // 1. the new army integral is higher than the previous best
    // 2. the new army integral is the same as the old best but the build order is 'better'
    if (    (m_integralStack.back().integral >  m_bestIntegralValue) 
        || ((m_integralStack.back().integral == m_bestIntegralValue) && Eval::BuildOrderBetter(buildOrder, m_bestIntegralBuildOrder)))
    {
        m_bestIntegralValue = m_integralStack.back().integral;
        m_bestIntegralStack = m_integralStack;
        m_bestIntegralBuildOrder = buildOrder;

        // print the newly found best to console
        printIntegralData(m_integralStack.size()-1);
    }
}

void CombatSearch_IntegralData::pop()
{
    m_integralStack.pop_back();
}

void CombatSearch_IntegralData::printIntegralData(const size_t index) const
{
    printf("%7d %10.2lf %13.2lf   ", m_bestIntegralStack[index].timeAdded, m_bestIntegralStack[index].eval, m_bestIntegralStack[index].integral);
    std::cout << m_bestIntegralBuildOrder.getNameString(2) << std::endl;   
}

void CombatSearch_IntegralData::print() const
{
    std::cout << "\nFinal CombatSearchIntegral Results\n\n";
    std::cout << "  Frame   ArmyEval  ArmyIntegral   BuildOrder\n";
    
    for (size_t i(0); i<m_bestIntegralStack.size(); ++i)
    {
        printIntegralData(i);
    }
}

const BuildOrder & CombatSearch_IntegralData::getBestBuildOrder() const
{
    return m_bestIntegralBuildOrder;
}