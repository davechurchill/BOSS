#include "BuildOrderPlotter.h"
#include "BOSSConfig.h"

using namespace BOSS;

void BuildOrderPlotter::QuickPlot(const GameState& state, const std::vector<BuildOrder>& buildOrders)
{
    BuildOrderPlotter plotter;
    for (size_t i = 0; i < buildOrders.size(); i++)
    {
        plotter.addPlot("QuickPlot", state, buildOrders[i]);
    }
    plotter.setOutputDir("results");
    plotter.doBuildOrderPlot();
}

BuildOrderPlotter::BuildOrderPlotter()
{

}

void BuildOrderPlotter::setOutputDir(const std::string & dir)
{
    m_outputDir = dir;
}

void BuildOrderPlotter::addPlot(const std::string & name, const GameState & state, const BuildOrder & buildOrder)
{
    m_buildOrderNames.push_back(name);
    m_allPlots.push_back(BuildOrderPlotData(state, buildOrder));
}

void BuildOrderPlotter::doPlots()
{
    std::string baseName = (m_allPlots.size() == 1) ? m_buildOrderNames[0] + "_BuildOrder" : "AllBuildOrders";

    writeBuildOrderPlot(m_allPlots, m_outputDir + "/" + baseName + ".gpl");
    writeHTMLPlot(m_outputDir + "/" + baseName + ".html");

    // write resource plots
    for (size_t i(0); i < m_allPlots.size(); i++)
    {
        std::stringstream rss;
        rss << m_outputDir << "/" << m_buildOrderNames[i] << "_ResourcePlot.gpl";
        writeResourcePlot(m_allPlots[i], rss.str());
    }

    // write army plots
    for (size_t i(0); i < m_allPlots.size(); i++)
    {
        std::stringstream rss;
        rss << m_outputDir << "/" << m_buildOrderNames[i] << "_ArmyPlot.gpl";
        writeArmyValuePlot(m_allPlots[i], rss.str());
    }
}

void BuildOrderPlotter::doBuildOrderPlot()
{
    std::string buildOrderFilename = "AllBuildOrders.gpl";

    // if we only have one build order, name the file after it
    if (m_allPlots.size() == 1)
    {
        buildOrderFilename = m_buildOrderNames[0] + "_BuildOrder.gpl";
    }

    writeBuildOrderPlot(m_allPlots, m_outputDir + "/" + buildOrderFilename);
}

void BuildOrderPlotter::writeResourcePlot(const BuildOrderPlotData & plot, const std::string & filename)
{
    std::string noext = RemoveFileExtension(filename);
    std::stringstream mineralss;

    for (size_t i(0); i < plot.m_minerals.size(); ++i)
    {
        mineralss << plot.m_minerals[i].first << " " << plot.m_minerals[i].second << std::endl;
    }

    std::stringstream gasss;

    for (size_t i(0); i < plot.m_gas.size(); ++i)
    {
        gasss << plot.m_gas[i].first << " " << plot.m_gas[i].second << std::endl;
    }

    WriteGnuPlot(noext + "_minerals", mineralss.str(), " with lines ");
    WriteGnuPlot(noext + "_gas", gasss.str(), " with lines ");
}

void BuildOrderPlotter::writeBuildOrderPlot(const std::vector<BuildOrderPlotData> & plots, const std::string & filename)
{
    if (plots.empty()) { return; }

    std::stringstream ss;
    int maxY = 0;
    for (size_t p(0); p < plots.size(); ++p)
    {
        maxY += (plots[p].m_maxLayer + 2) * (plots[p].m_boxHeight + plots[p].m_boxHeightBuffer) + 15;
    }

    int maxFinishTime = 0;
    for (const auto& plot : plots)
    {
        maxFinishTime = std::max(maxFinishTime, plot.m_maxFinishTime);
    }

    //ss << "set title \"Title Goes Here\"" << std::endl;
    //ss << "set xlabel \"Time (frames)\"" << std::endl;
    ss << "set style rect fc lt -1 fs transparent solid 0.15" << std::endl;
    ss << "set xrange [" << -(maxFinishTime*.03) << ":" << 1.03*maxFinishTime << "]" << std::endl;
    ss << "set yrange [-15:" << maxY << "]" << std::endl;
    ss << "unset ytics" << std::endl;
    ss << "set grid xtics" << std::endl;
    ss << "set nokey" << std::endl;
    //ss << "set size ratio " << (0.05 * m_maxLayer) << std::endl;
    ss << "set terminal wxt size 960,300" << std::endl;
    ss << "plotHeight = " << 1000 << std::endl;
    ss << "boxHeight = " << plots[0].m_boxHeight << std::endl;
    ss << "boxHeightBuffer = " << plots[0].m_boxHeightBuffer << std::endl;
    ss << "boxWidthScale = " << 1.0 << std::endl;
    
    int currentLayer = 0;
    size_t currentObject = plots[0].m_buildOrder.size();

    for (size_t p(0); p < plots.size(); ++p)
    {
        const BuildOrder & buildOrder = plots[p].m_buildOrder;

        for (size_t i(0); i < buildOrder.size(); ++i)
        {
            const Rectangle & rect = plots[p].m_rectangles[i];
            const int rectWidth = (rect.bottomRight.x() - rect.topLeft.x());
            const int rectCenterX = rect.bottomRight.x() - (rectWidth / 2);
        
            std::stringstream pos;
            pos << "(boxWidthScale * " << rectCenterX << "),";
            pos << "((boxHeight + boxHeightBuffer) * " << (plots[p].m_layers[i] + currentLayer) << " + boxHeight/2)";

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

        currentLayer += plots[p].m_maxLayer + 2;
        currentObject += buildOrder.size();
    }

    ss << "plot -10000" << std::endl;

    std::ofstream out(filename);
    out << ss.str();
    out.close();
}

void BuildOrderPlotter::writeHTMLPlot(const std::string & filename)
{
    const std::string plotJSON = getPlotJSON(m_allPlots);

    std::stringstream ss;
    ss << R"(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<style>
body {
    background-color: #ffffff;
    font-family: arial;
    font-size: 13px;
}
.buildOrderItem {
    box-sizing: border-box;
    font-family: arial;
    font-size: 11px;
    position: absolute;
    border: 1px solid #000000;
    overflow: hidden;
    white-space: nowrap;
    display: flex;
    align-items: center;
    justify-content: center;
}
.plotLabel {
    position: absolute;
    font-size: 13px;
    font-weight: bold;
    color: #333;
}
#drawArea {
    position: relative;
    padding: 10px;
}
</style>
</head>
<body>
<div id="drawArea"></div>
<script>
let boxHeight = 25;
let boxHeightBuffer = 4;
let xScale = 0.15;

function GetTime(frames) {
    let minutes = "" + parseInt(frames / (24 * 60)) + "m";
    let seconds = "" + parseInt((frames / 24) % 60) + "s";
    return minutes + " " + seconds;
}

function GetVerticalLine(x, y, w, h) {
    return '<div style="top:' + y + 'px;left:' + x + 'px;width:' + w +
           'px;height:' + h + 'px;position:absolute;border-left:1px dashed #dddddd;"></div>';
}

function GetBuildOrderItemDiv(plotIndex, index) {
    let data = plotData[plotIndex].buildOrder[index];
    let x = data[1] * xScale;
    let y = data[5] * (boxHeight + boxHeightBuffer);
    let w = Math.max((data[2] - data[1]) * xScale, 1);
    let h = boxHeight;
    let name = data[0];
    let color = data[6];

    let title = name + "\n";
    title += "  Start:    " + data[1] + " (" + GetTime(data[1]) + ")\n";
    title += "  End:      " + data[2] + " (" + GetTime(data[2]) + ")\n";
    title += "  Minerals: " + parseInt(data[3]) + "\n";
    title += "  Gas:      " + parseInt(data[4]);

    return '<div title="' + title + '" class="buildOrderItem" style="' +
           'top:' + y + 'px;left:' + x + 'px;width:' + w + 'px;height:' + h + 'px;' +
           'background-color:' + color + ';">' + name + '</div>';
}

function DrawBuildOrderPlots() {
    let drawArea = document.getElementById('drawArea');
    let currentY = 0;

    for (let p = 0; p < plotData.length; ++p) {
        let bo = plotData[p].buildOrder;

        let maxX = 0;
        let maxY = 0;
        for (let i = 0; i < bo.length; ++i) {
            maxX = Math.max(maxX, bo[i][2] * xScale);
            maxY = Math.max(maxY, (bo[i][5] + 1) * (boxHeight + boxHeightBuffer));
        }

        let label = '<div class="plotLabel" style="top:' + currentY + 'px;">' +
                    plotData[p].name + '</div>';
        drawArea.innerHTML += label;
        currentY += 20;

        let boDiv = '<div style="width:' + maxX + 'px;height:' + maxY + 'px;' +
                    'position:absolute;border:1px dashed #ccc;top:' + currentY + 'px;">';

        for (let x = 1440 * xScale; x < maxX; x += 1440 * xScale) {
            boDiv += GetVerticalLine(x, -20, 0, maxY + 20);
        }
        for (let i = 0; i < bo.length; ++i) {
            boDiv += GetBuildOrderItemDiv(p, i);
        }
        boDiv += '</div>';
        drawArea.innerHTML += boDiv;
        currentY += maxY + 50;
    }
    drawArea.style.height = currentY + 'px';
}

)";

    ss << plotJSON << ";\n\n";

    ss << R"(// adapt to the same format DrawBuildOrderPlots expects
let plotData = plots.map(p => ({ name: p.name, buildOrder: p.buildOrder }));
DrawBuildOrderPlots();
</script>
</body>
</html>
)";

    std::ofstream out(filename);
    out << ss.str();
    out.close();
}

std::string BuildOrderPlotter::getPlotJSON(const std::vector<BuildOrderPlotData> & plots)
{
    std::stringstream ss;
    ss << "var plots = [\n";

    for (size_t p(0); p < plots.size(); ++p)
    {
        const BuildOrder & buildOrder = plots[p].m_buildOrder;
        ss << "{ name : \"" << m_buildOrderNames[p] << "\", buildOrder : [";

        for (size_t i(0); i < buildOrder.size(); ++i)
        {
            ss << "[\"" << buildOrder[i].getName() << "\", ";
            ss << plots[p].m_startTimes[i] << ", "; 
            ss << plots[p].m_finishTimes[i] << ", ";
            ss << plots[p].m_minerals[i*2 + 1].second << ", ";
            ss << plots[p].m_gas[i*2 + 1].second << ", ";
            ss << plots[p].m_layers[i] << ", ";
            
                 if (buildOrder[i].isWorker())          { ss << "\"rgb(217, 255, 255)\""; }
            else if (buildOrder[i].isSupplyProvider())  { ss << "\"rgb(255, 249, 217)\""; }
            else if (buildOrder[i].isRefinery())        { ss << "\"rgb(217, 255, 217)\""; }
            else if (buildOrder[i].isBuilding())        { ss << "\"rgb(241, 223, 223)\""; }
            else if (buildOrder[i].isUpgrade())         { ss << "\"rgb(255, 217, 255)\""; }
            else                                        { ss << "\"rgb(190, 190, 190)\""; }
                        
            ss << "]";
            if (i < buildOrder.size() - 1) { ss << ", "; }
        }
        
        ss << "]}";
        if (p < plots.size() - 1) { ss << ", "; }
        ss << "\n";
    }

    ss << "]";

    return ss.str();
}

const std::vector<BuildOrderPlotData> & BuildOrderPlotter::getPlots() const
{
    return m_allPlots;
}

void BuildOrderPlotter::writeArmyValuePlot(const BuildOrderPlotData & plot, const std::string & filename)
{
    std::stringstream datass;
    for (size_t i(0); i < plot.m_buildOrder.size(); ++i)
    {
        datass << plot.m_startTimes[i] << " " << plot.m_armyValues[i] << std::endl;
    }
 
    WriteGnuPlot(filename, datass.str(), " with steps");
}


std::string BuildOrderPlotter::GetFileNameFromPath(const std::string & path)
{
    std::string temp(path);

    const size_t last_slash_idx = temp.find_last_of("\\/");
    if (std::string::npos != last_slash_idx)
    {
        temp.erase(0, last_slash_idx + 1);
    }
    
    return temp;
}

std::string BuildOrderPlotter::RemoveFileExtension(const std::string & path)
{
    std::string temp(path);

    const size_t period_idx = temp.rfind('.');
    if (std::string::npos != period_idx)
    {
        temp.erase(period_idx);
    }

    return temp;
}

void BuildOrderPlotter::WriteGnuPlot(const std::string & filename, const std::string & data, const std::string & args)
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
