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

class BuildOrderPlotData
{
    friend class BuildOrderPlotter;

    const GameState         m_initialState;
    const BuildOrder        m_buildOrder;
        
    std::vector<int>        m_startTimes;
    std::vector<int>        m_finishTimes;
    std::vector<int>        m_layers;
    std::vector<double>     m_armyValues;
    std::vector< std::pair<int,int> > m_minerals;
    std::vector< std::pair<int,int> > m_gas;
    std::vector<Rectangle>  m_rectangles;

    int                     m_maxLayer;
    int                     m_maxFinishTime;
    int                     m_boxHeight;
    int                     m_boxHeightBuffer;

    void calculateStartEndTimes();
    void calculatePlot();


public:

    BuildOrderPlotData(const GameState & initialState, const BuildOrder & buildOrder);

};

}