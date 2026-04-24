#include "DFBBSearchExperiment.h"
#include <algorithm>
#include <iomanip>
#include "DFBB_BuildOrderSmartSearch.h"
#include "DFBB_BuildOrderStackSearch.h"
#include "BuildOrderPlotter.h"
#include "FileTools.h"

using namespace BOSS;

DFBBSearchExperiment::DFBBSearchExperiment()
    : m_searchTimeLimitMS(30000)
    , m_printNewBest(false)
    , m_smartSearch(true)
    , m_orderings({ ActionOrderingType::None })
    , m_useRepetitions(true)
    , m_useIncreasingRepetitions(true)
    , m_useLandmarkLowerBound(true)
    , m_useResourceLowerBound(true)
    , m_useAlwaysMakeWorkers(true)
    , m_useSupplyBounding(true)
    , m_supplyBoundingThreshold(1.5)
{
}

DFBBSearchExperiment::DFBBSearchExperiment(const std::string & name, const json & val)
    : m_name(name)
    , m_searchTimeLimitMS(30000)
    , m_printNewBest(false)
    , m_smartSearch(true)
    , m_orderings({ ActionOrderingType::None })
    , m_useRepetitions(true)
    , m_useIncreasingRepetitions(true)
    , m_useLandmarkLowerBound(true)
    , m_useResourceLowerBound(true)
    , m_useAlwaysMakeWorkers(true)
    , m_useSupplyBounding(true)
    , m_supplyBoundingThreshold(1.5)
{
    BOSS_ASSERT(val.count("State") && val["State"].is_string(), "DFBBSearchExperiment must have a 'State' string");
    m_initialState = BOSSConfig::Instance().GetState(val["State"]);

    BOSS_ASSERT(val.count("Goal") && val["Goal"].is_string(), "DFBBSearchExperiment must have a 'Goal' string");
    m_goal = BOSSConfig::Instance().GetBuildOrderSearchGoalMap(val["Goal"]);

    BOSS_ASSERT(val.count("SearchTimeLimitMS") && val["SearchTimeLimitMS"].is_number_integer(), "DFBBSearchExperiment must have a 'SearchTimeLimitMS' int");
    m_searchTimeLimitMS = val["SearchTimeLimitMS"];

    if (val.count("SmartSearch") && val["SmartSearch"].is_boolean())
    {
        m_smartSearch = val["SmartSearch"];
    }

    if (!m_smartSearch)
    {
        BOSS_ASSERT(val.count("RelevantActions") && val["RelevantActions"].is_array(), "DFBBSearchExperiment with SmartSearch=false must have a 'RelevantActions' array");
        for (const auto & entry : val["RelevantActions"])
        {
            BOSS_ASSERT(entry.is_string(), "RelevantActions element must be an action type string");
            const std::string typeName = entry.get<std::string>();
            BOSS_ASSERT(ActionTypes::TypeExists(typeName), "Action type doesn't exist: %s", typeName.c_str());
            m_relevantActions.push_back(ActionTypes::GetActionType(typeName));
        }
    }

    if (val.count("OutputDir") && val["OutputDir"].is_string())
    {
        m_outputDir = val["OutputDir"].get<std::string>();
    }

    if (val.count("HTMLFile") && val["HTMLFile"].is_string())
    {
        m_htmlFile = val["HTMLFile"].get<std::string>();
    }

    if (val.count("PrintNewBest") && val["PrintNewBest"].is_boolean())
    {
        m_printNewBest = val["PrintNewBest"];
    }

    if (val.count("Ordering") && val["Ordering"].is_array())
    {
        m_orderings.clear();
        for (const auto & entry : val["Ordering"])
        {
            BOSS_ASSERT(entry.is_string(), "Ordering element must be a string");
            m_orderings.push_back(ActionOrdering::GetOrderingType(entry.get<std::string>()));
        }
    }

    JSONTools::ReadBool("UseRepetitions",           val, m_useRepetitions);
    JSONTools::ReadBool("UseIncreasingRepetitions", val, m_useIncreasingRepetitions);
    JSONTools::ReadBool("UseLandmarkLowerBound",    val, m_useLandmarkLowerBound);
    JSONTools::ReadBool("UseResourceLowerBound",    val, m_useResourceLowerBound);
    JSONTools::ReadBool("UseAlwaysMakeWorkers",     val, m_useAlwaysMakeWorkers);
    JSONTools::ReadBool("UseSupplyBounding",        val, m_useSupplyBounding);

    if (val.count("SupplyBoundingThreshold") && val["SupplyBoundingThreshold"].is_number())
    {
        m_supplyBoundingThreshold = val["SupplyBoundingThreshold"];
    }
}

void DFBBSearchExperiment::printResults(const DFBB_BuildOrderSearchResults & results) const
{
    std::cout << "  Solved:         " << (results.solved ? "yes" : "no") << "\n";
    std::cout << "  Timed Out:      " << (results.timedOut ? "yes" : "no") << "\n";
    std::cout << "  Solution Found: " << (results.solutionFound ? "yes" : "no") << "\n";
    std::cout << "  Upper Bound:    " << results.upperBound << " frames\n";
    std::cout << "  Nodes Expanded: " << results.nodesExpanded << "\n";
    std::cout << "  Time Elapsed:   " << results.timeElapsed << " ms\n";

    if (results.nodesExpanded > 0 && results.timeElapsed > 0)
    {
        std::cout << "  Nodes/sec:      " << (1000.0 * results.nodesExpanded / results.timeElapsed) << "\n";
    }

    if (results.buildOrder.size() > 0)
    {
        std::cout << "\n  Build Order (" << results.buildOrder.size() << " actions):\n    ";
        for (size_t i = 0; i < results.buildOrder.size(); ++i)
        {
            std::cout << results.buildOrder[i].getName();
            if (i + 1 < results.buildOrder.size()) { std::cout << ", "; }
        }
        std::cout << "\n";
    }

    std::cout << "\n";
}

void DFBBSearchExperiment::run()
{
    static std::string stars = "************************************************";
    const std::string searchLabel = m_smartSearch ? "DFBB Smart" : "DFBB";

    std::vector<RunData> runs;
    for (const ActionOrderingType ordering : m_orderings)
    {
        const std::string orderingLabel = ActionOrdering::GetOrderingName(ordering);
        std::cout << "\n" << stars << "\n* Running Experiment: " << m_name << " [" << searchLabel << "] [" << orderingLabel << "]\n" << stars << "\n\n";

        RunData run;
        run.label = orderingLabel;

        if (m_smartSearch)
        {
            DFBB_BuildOrderSmartSearch search;
            search.setState(m_initialState);
            search.setGoal(m_goal);
            search.setTimeLimit(m_searchTimeLimitMS);
            search.setPrintNewBest(m_printNewBest);
            search.setOrdering(ordering);
            search.setUseRepetitions(m_useRepetitions);
            search.setUseIncreasingRepetitions(m_useIncreasingRepetitions);
            search.setUseLandmarkLowerBound(m_useLandmarkLowerBound);
            search.setUseResourceLowerBound(m_useResourceLowerBound);
            search.setUseAlwaysMakeWorkers(m_useAlwaysMakeWorkers);
            search.setUseSupplyBounding(m_useSupplyBounding);
            search.setSupplyBoundingThreshold(m_supplyBoundingThreshold);
            search.search();
            run.results = search.getResults();
        }
        else
        {
            DFBB_BuildOrderSearchParameters params;
            params.m_goal                             = m_goal;
            params.m_initialState                     = m_initialState;
            params.m_relevantActions                  = m_relevantActions;
            params.m_searchTimeLimit                  = m_searchTimeLimitMS;
            params.m_printNewBest                     = m_printNewBest;
            params.m_ordering                         = ordering;
            params.m_useRepetitions                   = m_useRepetitions;
            params.m_useIncreasingRepetitions         = m_useIncreasingRepetitions;
            params.m_useLandmarkLowerBoundHeuristic   = m_useLandmarkLowerBound;
            params.m_useResourceLowerBoundHeuristic   = m_useResourceLowerBound;
            params.m_useAlwaysMakeWorkers             = m_useAlwaysMakeWorkers;
            params.m_useSupplyBounding                = m_useSupplyBounding;
            params.m_supplyBoundingThreshold          = m_supplyBoundingThreshold;

            DFBB_BuildOrderStackSearch search(params);
            search.search();
            run.results = search.getResults();
        }

        printResults(run.results);
        runs.push_back(run);

    }

    if (!m_htmlFile.empty())
    {
        writeHTMLFile(runs);
        std::cout << "  HTML written to: " << m_htmlFile << "\n";
    }
}

void DFBBSearchExperiment::writeHTMLFile(const std::vector<RunData> & runs) const
{
    std::vector<RunData> sortedRuns = runs;
    std::stable_sort(sortedRuns.begin(), sortedRuns.end(), [](const RunData & lhs, const RunData & rhs)
    {
        return lhs.results.nodesExpanded < rhs.results.nodesExpanded;
    });

    BuildOrderPlotter plotter;
    for (const RunData & run : sortedRuns)
    {
        if (run.results.solutionFound)
        {
            plotter.addPlot(m_name + " [" + run.label + "]", m_initialState, run.results.buildOrder);
        }
    }

    std::stringstream ss;

    ss << R"(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<style>
body { background-color: #ffffff; font-family: arial; font-size: 13px; margin: 20px; }
h2   { color: #333; }
table { border-collapse: collapse; margin-bottom: 30px; }
th { background-color: #4a4a6a; color: white; padding: 6px 12px; text-align: left; }
td { padding: 5px 12px; border-bottom: 1px solid #ddd; }
tr:hover td { background-color: #f5f5f5; }
.yes  { color: #2a7a2a; font-weight: bold; }
.no   { color: #cc3333; font-weight: bold; }
.buildOrderItem {
    box-sizing: border-box; font-family: arial; font-size: 11px;
    position: absolute; border: 1px solid #000000;
    overflow: hidden; white-space: nowrap;
    display: flex; align-items: center; justify-content: center;
}
.plotLabel { position: absolute; font-size: 13px; font-weight: bold; color: #333; }
#drawArea  { position: relative; padding: 10px; }
</style>
</head>
<body>
)";

    ss << "<h2>" << m_name << " &mdash; Search Results</h2>\n";

    // stats table
    ss << "<table>\n";
    ss << "<tr><th>Ordering</th><th>Solved</th><th>Solution Found</th><th>Timed Out</th>"
       << "<th>Upper Bound (frames)</th><th>Nodes Expanded &#8593;</th>"
       << "<th>Time Elapsed (ms)</th><th>Nodes / sec</th></tr>\n";

    for (const RunData & run : sortedRuns)
    {
        const DFBB_BuildOrderSearchResults & r = run.results;
        double nodesPerSec = (r.timeElapsed > 0) ? (1000.0 * r.nodesExpanded / r.timeElapsed) : 0.0;

        ss << "<tr>";
        ss << "<td>" << run.label << "</td>";
        ss << "<td class=\"" << (r.solved        ? "yes\">yes" : "no\">no")  << "</td>";
        ss << "<td class=\"" << (r.solutionFound ? "yes\">yes" : "no\">no")  << "</td>";
        ss << "<td class=\"" << (r.timedOut      ? "no\">yes"  : "yes\">no") << "</td>";
        ss << "<td>" << r.upperBound << "</td>";
        ss << "<td>" << r.nodesExpanded << "</td>";
        ss << "<td>" << std::fixed << std::setprecision(1) << r.timeElapsed << "</td>";
        ss << "<td>" << std::fixed << std::setprecision(0) << nodesPerSec << "</td>";
        ss << "</tr>\n";
    }
    ss << "</table>\n";

    // build order visualization
    if (plotter.getPlots().size() > 0)
    {
        ss << "<h2>Build Orders</h2>\n";
        ss << "<div id=\"drawArea\"></div>\n";
        ss << "<script>\n";
        ss << "let boxHeight = 25;\n";
        ss << "let boxHeightBuffer = 4;\n";
        ss << "let xScale = 0.15;\n";
        ss << R"(
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
        let maxX = 0, maxY = 0;
        for (let i = 0; i < bo.length; ++i) {
            maxX = Math.max(maxX, bo[i][2] * xScale);
            maxY = Math.max(maxY, (bo[i][5] + 1) * (boxHeight + boxHeightBuffer));
        }
        let label = '<div class="plotLabel" style="top:' + currentY + 'px;">' + plotData[p].name + '</div>';
        drawArea.innerHTML += label;
        currentY += 20;
        let boDiv = '<div style="width:' + maxX + 'px;height:' + maxY + 'px;' +
                    'position:absolute;border:1px dashed #ccc;top:' + currentY + 'px;">';
        for (let x = 1440 * xScale; x < maxX; x += 1440 * xScale) {
            boDiv += GetVerticalLine(x, -20, 0, maxY + 20);
        }
        for (let i = 0; i < bo.length; ++i) { boDiv += GetBuildOrderItemDiv(p, i); }
        boDiv += '</div>';
        drawArea.innerHTML += boDiv;
        currentY += maxY + 50;
    }
    drawArea.style.height = currentY + 'px';
}
)";
        ss << plotter.getPlotJSON(plotter.getPlots()) << ";\n";
        ss << "let plotData = plots.map(p => ({ name: p.name, buildOrder: p.buildOrder }));\n";
        ss << "DrawBuildOrderPlots();\n";
        ss << "</script>\n";
    }

    ss << "</body>\n</html>\n";

    std::ofstream out(m_htmlFile);
    out << ss.str();
    out.close();
}
