#include "BuildOrderPlotData.h"
#include "Eval.h"

using namespace BOSS;

BuildOrderPlotData::BuildOrderPlotData(const GameState & initialState, const BuildOrder & buildOrder)
    : m_initialState(initialState)
    , m_buildOrder(buildOrder)
    , m_maxLayer(0)
    , m_maxFinishTime(0)
    , m_boxHeight(20)
    , m_boxHeightBuffer(3)
{
    calculateStartEndTimes();
    calculatePlot();
}

void BuildOrderPlotData::calculateStartEndTimes()
{
    GameState state(m_initialState);

    //BOSS_ASSERT(_buildOrder.isLegalFromState(state), "Build order isn't legal!");

    for (size_t i(0); i < m_buildOrder.size(); ++i)
    {
        const ActionType & type = m_buildOrder[i];
        state.doAction(type);

        m_startTimes.push_back(state.getCurrentFrame());

        int finish = state.getCurrentFrame() + type.buildTime();
        if (type.isBuilding() && !type.isAddon() && !type.isMorphed())
        {
            finish += 0; // TODO: building constant walk time
        }

        m_finishTimes.push_back(finish);

        m_maxFinishTime = std::max(m_maxFinishTime, finish);

        m_armyValues.push_back(Eval::ArmyTotalResourceSum(state));

        std::pair<int, int> mineralsBefore(state.getCurrentFrame(), (int)(state.getMinerals() + type.mineralPrice()));
        std::pair<int, int> mineralsAfter(state.getCurrentFrame(), (int)state.getMinerals());

        std::pair<int, int> gasBefore(state.getCurrentFrame(), (int)(state.getGas() + type.gasPrice()));
        std::pair<int, int> gasAfter(state.getCurrentFrame(), (int)state.getGas());

        m_minerals.push_back(mineralsBefore);
        m_minerals.push_back(mineralsAfter);
        m_gas.push_back(gasBefore);
        m_gas.push_back(gasAfter);
    }
}

void BuildOrderPlotData::calculatePlot()
{
    m_layers = std::vector<int>(m_startTimes.size(), -1); 

    // determine the layers for each action
    for (size_t i(0); i < m_startTimes.size(); ++i)
    {
        int start    = m_startTimes[i];
        int finish   = m_finishTimes[i];

        std::vector<int> layerOverlap;

        // loop through everything up to this action and see which layers it can't be in
        for (size_t j(0); j < i; ++j)
        {
            if (start < m_finishTimes[j])
            {
                layerOverlap.push_back(m_layers[j]);
            }
        }

        // find a layer we can assign to this value
        int layerTest = 0;
        while (true)
        {
            if (std::find(layerOverlap.begin(), layerOverlap.end(), layerTest) == layerOverlap.end())
            {
                m_layers[i] = layerTest;
                if (layerTest > m_maxLayer)
                {
                    m_maxLayer = layerTest;
                }
                break;
            }

            layerTest++;
        }
    }

    for (size_t i(0); i < m_buildOrder.size(); ++i)
    {
        Position topLeft(m_startTimes[i], m_layers[i] * (m_boxHeight + m_boxHeightBuffer));
        Position bottomRight(m_finishTimes[i], topLeft.y() + m_boxHeight);

        m_rectangles.push_back(Rectangle(m_buildOrder[i].getName(), topLeft, bottomRight));
    }
}
