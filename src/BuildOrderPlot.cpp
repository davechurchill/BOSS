#include "BuildOrderPlot.h"
#include "Eval.h"

using namespace BOSS;

BuildOrderPlot::BuildOrderPlot(const GameState & initialState, const BuildOrder & buildOrder)
    : m_initialState(initialState)
    , m_buildOrder(buildOrder)
    , m_boxHeight(20)
    , m_boxHeightBuffer(3)
    , m_maxLayer(0)
    , m_maxFinishTime(0)
{
    calculateStartEndTimes();
    calculatePlot();
}

void BuildOrderPlot::calculateStartEndTimes()
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

        std::pair<int, int> mineralsBefore(state.getCurrentFrame(), state.getMinerals() + type.mineralPrice());
        std::pair<int, int> mineralsAfter(state.getCurrentFrame(), state.getMinerals());

        std::pair<int, int> gasBefore(state.getCurrentFrame(), state.getGas() + type.gasPrice());
        std::pair<int, int> gasAfter(state.getCurrentFrame(), state.getGas());

        m_minerals.push_back(mineralsBefore);
        m_minerals.push_back(mineralsAfter);
        m_gas.push_back(gasBefore);
        m_gas.push_back(gasAfter);
    }
}

void BuildOrderPlot::calculatePlot()
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

void BuildOrderPlot::addPlot(const BuildOrderPlot & plot)
{
    m_otherPlots.push_back(plot);
}

void BuildOrderPlot::writeResourcePlot(const std::string & filename)
{
    std::string noext = RemoveFileExtension(filename);
    std::stringstream mineralss;

    for (size_t i(0); i < m_minerals.size(); ++i)
    {
        mineralss << m_minerals[i].first << " " << m_minerals[i].second << std::endl;
    }

    std::stringstream gasss;

    for (size_t i(0); i < m_gas.size(); ++i)
    {
        gasss << m_gas[i].first << " " << m_gas[i].second << std::endl;
    }

    WriteGnuPlot(noext + "_minerals", mineralss.str(), " with lines ");
    WriteGnuPlot(noext + "_gas", gasss.str(), " with lines ");
}

void BuildOrderPlot::writeRectanglePlot(const std::string & filename)
{
    std::stringstream ss;
    int maxY = (m_maxLayer + 1) * (m_boxHeight + m_boxHeightBuffer) + 15;

    for (size_t p(0); p < m_otherPlots.size(); ++p)
    {
        maxY += (m_otherPlots[p].m_maxLayer + 2) * (m_boxHeight + m_boxHeightBuffer) + 15;
    }

    //ss << "set title \"Title Goes Here\"" << std::endl;
    //ss << "set xlabel \"Time (frames)\"" << std::endl;
    ss << "set style rect fc lt -1 fs transparent solid 0.15" << std::endl;
    ss << "set xrange [" << -(m_maxFinishTime*.03) << ":" << 1.03*m_maxFinishTime << "]" << std::endl;
    ss << "set yrange [-15:" << maxY << "]" << std::endl;
    ss << "unset ytics" << std::endl;
    ss << "set grid xtics" << std::endl;
    ss << "set nokey" << std::endl;
    //ss << "set size ratio " << (0.05 * m_maxLayer) << std::endl;
    ss << "set terminal wxt size 960,300" << std::endl;
    ss << "plotHeight = " << 1000 << std::endl;
    ss << "boxHeight = " << m_boxHeight << std::endl;
    ss << "boxHeightBuffer = " << m_boxHeightBuffer << std::endl;
    ss << "boxWidthScale = " << 1.0 << std::endl;

    for (size_t i(0); i < m_buildOrder.size(); ++i)
    {
        const Rectangle & rect = m_rectangles[i];
        const int rectWidth = (rect.bottomRight.x() - rect.topLeft.x());
        const int rectCenterX = rect.bottomRight.x() - (rectWidth / 2);
        
        std::stringstream pos;
        pos << "(boxWidthScale * " << rectCenterX << "),";
        pos << "((boxHeight + boxHeightBuffer) * " << m_layers[i] << " + boxHeight/2)";

        ss << "set object " << (i+1) << " rect at ";
        ss << pos.str();
        ss << " size ";
        ss << "(boxWidthScale * " << (rectWidth) << "),";
        ss << "(boxHeight) ";
        //ss << "(boxWidthScale * " << m_finishTimes[i] << "),";
        //ss << "((boxHeight + boxHeightBuffer) * " << m_layers[i] << " + boxHeight) ";
        ss << "lw 1";

        if (m_buildOrder[i].isWorker())
        {
            ss << " fc rgb \"cyan\"";
        }
        else if (m_buildOrder[i].isSupplyProvider())
        {
            ss << " fc rgb \"gold\"";
        }
        else if (m_buildOrder[i].isRefinery())
        {
            ss << " fc rgb \"green\"";
        }
        else if (m_buildOrder[i].isBuilding())
        {
            ss << " fc rgb \"brown\"";
        }
        else if (m_buildOrder[i].isUpgrade())
        {
            ss << " fc rgb \"purple\"";
        }

        ss << std::endl;

        ss << "set label " << (i+1) << " at " << pos.str() << " \"" << m_buildOrder[i].getName() << "\" front center";
        ss << std::endl;
    }

    int currentLayer = m_maxLayer + 2;
    int currentObject = m_buildOrder.size();

    for (size_t p(0); p < m_otherPlots.size(); ++p)
    {
        const BuildOrder & buildOrder = m_otherPlots[p].m_buildOrder;

        for (size_t i(0); i < buildOrder.size(); ++i)
        {
            const Rectangle & rect = m_otherPlots[p].m_rectangles[i];
            const int rectWidth = (rect.bottomRight.x() - rect.topLeft.x());
            const int rectCenterX = rect.bottomRight.x() - (rectWidth / 2);
        
            std::stringstream pos;
            pos << "(boxWidthScale * " << rectCenterX << "),";
            pos << "((boxHeight + boxHeightBuffer) * " << (m_otherPlots[p].m_layers[i] + currentLayer) << " + boxHeight/2)";

            ss << "set object " << (currentObject+i+1) << " rect at ";
            ss << pos.str();
            ss << " size ";
            ss << "(boxWidthScale * " << (rectWidth) << "),";
            ss << "(boxHeight) ";
            //ss << "(boxWidthScale * " << m_finishTimes[i] << "),";
            //ss << "((boxHeight + boxHeightBuffer) * " << m_layers[i] << " + boxHeight) ";
            ss << "lw 1";

            if (buildOrder[i].isWorker())
            {
                ss << " fc rgb \"cyan\"";
            }
            else if (buildOrder[i].isSupplyProvider())
            {
                ss << " fc rgb \"gold\"";
            }
            else if (buildOrder[i].isRefinery())
            {
                ss << " fc rgb \"green\"";
            }
            else if (buildOrder[i].isBuilding())
            {
                ss << " fc rgb \"brown\"";
            }
            else if (buildOrder[i].isUpgrade())
            {
                ss << " fc rgb \"purple\"";
            }

            ss << std::endl;

            ss << "set label " << (currentObject+i+1) << " at " << pos.str() << " \"" << buildOrder[i].getName() << "\" front center";
            ss << std::endl;
        }

        currentLayer += m_otherPlots[p].m_maxLayer + 2;
        currentObject += buildOrder.size();
    }

    ss << "plot -10000" << std::endl;

    std::ofstream out(filename);
    out << ss.str();
    out.close();
}

void BuildOrderPlot::writeArmyValuePlot(const std::string & filename)
{
    std::stringstream datass;
    for (size_t i(0); i < m_buildOrder.size(); ++i)
    {
        datass << m_startTimes[i] << " " << m_armyValues[i] << std::endl;
    }
 
    WriteGnuPlot(filename, datass.str(), " with steps");
}

void BuildOrderPlot::WriteGnuPlot(const std::string & filename, const std::string & data, const std::string & args)
{
    std::string file = RemoveFileExtension(GetFileNameFromPath(filename));
    std::string noext = RemoveFileExtension(filename);

    std::ofstream dataout(noext + "_data.txt");
    dataout << data;
    dataout.close();

    std::stringstream ss;
    ss << "set xlabel \"Time (frames)\"" << std::endl;
    ss << "set ylabel \"Resource Over Time\"" << std::endl;
    ss << "plot \"" << (file + "_data.txt") << "\" " << args << std::endl;

    std::ofstream out(noext + ".gpl");
    out << ss.str();
    out.close();
}

std::string BuildOrderPlot::GetFileNameFromPath(const std::string & path)
{
    std::string temp(path);

    const size_t last_slash_idx = temp.find_last_of("\\/");
    if (std::string::npos != last_slash_idx)
    {
        temp.erase(0, last_slash_idx + 1);
    }
    
    return temp;
}

std::string BuildOrderPlot::RemoveFileExtension(const std::string & path)
{
    std::string temp(path);

    const size_t period_idx = temp.rfind('.');
    if (std::string::npos != period_idx)
    {
        temp.erase(period_idx);
    }

    return temp;
}