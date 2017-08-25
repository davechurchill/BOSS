#pragma once

#include "Common.h"
#include "ActionType.h"
#include "GameState.h"
#include "BuildOrder.h"
#include "Position.hpp"

namespace BOSS
{

class Rectangle
{
public:

    std::string labelText;
    Position topLeft;
    Position bottomRight;
    Rectangle(const std::string & label, Position & tl, const Position & br) : labelText(label), topLeft(tl), bottomRight(br) { }
};

class BuildOrderPlot
{
    const GameState         m_initialState;
    const BuildOrder        m_buildOrder;
        
    std::vector<int>        m_startTimes;
    std::vector<int>        m_finishTimes;
    std::vector<int>        m_layers;
    std::vector<double>     m_armyValues;
    std::vector< std::pair<int,int> > m_minerals;
    std::vector< std::pair<int,int> > m_gas;
    std::vector<Rectangle>  m_rectangles;

    std::vector<BuildOrderPlot> m_otherPlots;

    int                     m_maxLayer;
    int                     m_maxFinishTime;
    int                     m_boxHeight;
    int                     m_boxHeightBuffer;

    void calculateStartEndTimes();
    void calculatePlot();


public:

    BuildOrderPlot(const GameState & initialState, const BuildOrder & buildOrder);

    void writeResourcePlot(const std::string & filename);
    void writeRectanglePlot(const std::string & filename);
    void writeArmyValuePlot(const std::string & filename);
    void writeHybridPlot(const std::string & filename);

    void addPlot(const BuildOrderPlot & plot);

    static std::string GetFileNameFromPath(const std::string & path);
    static std::string RemoveFileExtension(const std::string & path);
    static void WriteGnuPlot(const std::string & filename, const std::string & data, const std::string & args);
};

}