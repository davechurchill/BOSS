#include "CombatSearch_BestResponseData.h"

using namespace BOSS;

CombatSearch_BestResponseData::CombatSearch_BestResponseData(const GameState & enemyState, const BuildOrder & enemyBuildOrder)
    : m_enemyInitialState(enemyState)
    , m_enemyBuildOrder(enemyBuildOrder)
    , m_bestEval(std::numeric_limits<double>::max())
{
    // compute enemy army values

    calculateArmyValues(m_enemyInitialState, m_enemyBuildOrder, m_enemyArmyValues);
}

void CombatSearch_BestResponseData::calculateArmyValues(const GameState & initialState, const BuildOrder & buildOrder, std::vector< std::pair<double, double> > & values)
{
    values.clear();
    GameState state(initialState);
    for (size_t i(0); i < buildOrder.size(); ++i)
    {
        state.doAction(buildOrder[i]);
        values.push_back(std::pair<double,double>(state.getCurrentFrame(), Eval::ArmyTotalResourceSum(state)));
    }
}

void CombatSearch_BestResponseData::update(const GameState & initialState, const GameState & currentState, const BuildOrder & buildOrder)
{
    double eval = compareBuildOrder(initialState, buildOrder);

    if (eval < m_bestEval)
    {
        m_bestEval = eval;
        m_bestBuildOrder = buildOrder;
        m_bestState = currentState;

        std::cout << eval << "   " << m_bestBuildOrder.getNameString(2) << std::endl;
    }
}

double CombatSearch_BestResponseData::compareBuildOrder(const GameState & initialState, const BuildOrder & buildOrder)
{
    calculateArmyValues(initialState, buildOrder, m_selfArmyValues);

    size_t selfIndex = 0;
    size_t enemyIndex = 0;
    double maxDiff = std::numeric_limits<double>::lowest();
    double sumDiff = 0;
    int n = 0;

    for (size_t ei(0); ei < m_enemyArmyValues.size(); ++ei)
    {
        double enemyTime = m_enemyArmyValues[ei].first;
        double enemyVal = m_enemyArmyValues[ei].second;    
    
        size_t selfIndex = 0;

        // find the corresponding self army value for this time
        for (size_t si(0); si < m_selfArmyValues.size(); ++si)
        {
            if (enemyTime < m_selfArmyValues[si].first)
            {
                break;
            }

            selfIndex = si;
        }
    
        double selfVal = m_selfArmyValues[selfIndex].second;
        double diff = enemyVal - selfVal;
        maxDiff = std::max(maxDiff, diff);
    }

    return maxDiff;
}

size_t CombatSearch_BestResponseData::getStateIndex(const GameState & state)
{
    int frame = state.getCurrentFrame();

    if (frame > m_enemyStates.back().getCurrentFrame())
    {
        return m_enemyStates.size() - 1;
    }

    for (size_t i(0); i < m_enemyStates.size(); ++i)
    {
        if (frame < m_enemyStates[i].getCurrentFrame())
        {
            return i;
        }
    }

    BOSS_ASSERT(false, "Should have found an index");
    return 0;
}

const BuildOrder & CombatSearch_BestResponseData::getBestBuildOrder() const
{
    return m_bestBuildOrder;
}