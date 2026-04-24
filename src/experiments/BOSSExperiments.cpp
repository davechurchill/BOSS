#include "BOSSExperiments.h"

#include "AStarSearchExperiment.h"
#include "CombatSearchExperiment.h"
#include "DFBBSearchExperiment.h"
#include "DFBB_BuildOrderSmartSearch.h"
#include "BuildOrderPlotter.h"
#include "FileTools.h"
#include "MonteCarloTreeSearchExperiment.h"

using namespace BOSS;

void Experiments::RunExperiments(const std::string & experimentFilename)
{
    std::ifstream file(experimentFilename);
    json j;
    file >> j;

    BOSS_ASSERT(j.count("Experiments"), "No 'Experiments' member found");

    for (auto it = j["Experiments"].begin(); it != j["Experiments"].end(); ++it)
    {
        const std::string &         name = it.key();
        const json &                val  = it.value();
        
        //std::cout << "Found Experiment:   " << name << std::endl;
        BOSS_ASSERT(val.count("Type") && val["Type"].is_string(), "Experiment has no 'Type' string");

        if (val.count("Run") && val["Run"].is_boolean() && (val["Run"] == true))
        {   
            const std::string & type = val["Type"];

            if (type == "CombatSearch")
            {
                RunCombatExperiment(name, val);
            }
            else if (type == "BuildOrderPlot")
            {
                RunBuildOrderPlot(name, val);
            }
            else if (type == "DFBBSearch")
            {
                RunDFBBSearchExperiment(name, val);
            }
            else if (type == "BuildOrderSearch")
            {
                RunBuildOrderSearchExperiment(name, val);
            }
            else if (type == "MonteCarloTreeSearch")
            {
                RunMonteCarloTreeSearchExperiment(name, val);
            }
            else if (type == "AStarSearch")
            {
                RunAStarSearchExperiment(name, val);
            }
            else
            {
                BOSS_ASSERT(false, "Unknown Experiment Type: %s", type.c_str());
            }
        }
    }

    std::cout << "\n\n";
}

void Experiments::RunCombatExperiment(const std::string & name, const json & val)
{
    std::cout << "Combat Search Experiment - " << name << std::endl;

    CombatSearchExperiment exp(name, val);
    exp.run();

    std::cout << "    " << name << " completed" << std::endl;
}

void Experiments::RunDFBBSearchExperiment(const std::string & name, const json & val)
{
    std::cout << "DFBB Search Experiment - " << name << std::endl;

    DFBBSearchExperiment exp(name, val);
    exp.run();

    std::cout << "    " << name << " completed" << std::endl;
}

void Experiments::RunBuildOrderSearchExperiment(const std::string & name, const json & val)
{
    std::cout << "Build Order Search Experiment - " << name << std::endl;

    BOSS_ASSERT(val.count("State") && val["State"].is_string(), "BuildOrderSearch experiment must have a 'State' string");
    BOSS_ASSERT(val.count("Goal") && val["Goal"].is_string(), "BuildOrderSearch experiment must have a 'Goal' string");

    DFBB_BuildOrderSmartSearch search;
    search.setState(BOSSConfig::Instance().GetState(val["State"]));
    search.setGoal(BOSSConfig::Instance().GetBuildOrderSearchGoalMap(val["Goal"]));

    if (val.count("SearchTimeLimitMS") && val["SearchTimeLimitMS"].is_number_integer())
    {
        search.setTimeLimit(val["SearchTimeLimitMS"]);
    }

    if (val.count("PrintNewBest") && val["PrintNewBest"].is_boolean())
    {
        search.setPrintNewBest(val["PrintNewBest"]);
    }

    search.search();
    const DFBB_BuildOrderSearchResults & results = search.getResults();
    results.printResults();

    if (val.count("OutputFile") && val["OutputFile"].is_string())
    {
        std::ofstream out(val["OutputFile"].get<std::string>());
        out << "Solved: " << (results.solved ? "yes" : "no") << "\n";
        out << "Timed Out: " << (results.timedOut ? "yes" : "no") << "\n";
        out << "Solution Found: " << (results.solutionFound ? "yes" : "no") << "\n";
        out << "Upper Bound: " << results.upperBound << "\n";
        out << "Nodes Expanded: " << results.nodesExpanded << "\n";
        out << "Time Elapsed: " << results.timeElapsed << "\n";
        out << "Build Order: " << results.buildOrder.getNameString() << "\n";
    }

    std::cout << "    " << name << " completed" << std::endl;
}

void Experiments::RunMonteCarloTreeSearchExperiment(const std::string & name, const json & val)
{
    std::cout << "Monte Carlo Tree Search Experiment - " << name << std::endl;

    MonteCarloTreeSearchExperiment exp(name, val);
    exp.run();

    std::cout << "    " << name << " completed" << std::endl;
}

void Experiments::RunAStarSearchExperiment(const std::string & name, const json & val)
{
    std::cout << "A* Search Experiment - " << name << std::endl;

    AStarSearchExperiment exp(name, val);
    exp.run();

    std::cout << "    " << name << " completed" << std::endl;
}

void Experiments::RunBuildOrderPlot(const std::string & name, const json & j)
{
    std::cout << "Build Order Plot Experiment - " << name << std::endl;

    BOSS_ASSERT(j.count("Scenarios") && j["Scenarios"].is_array(), "Experiment has no Scenarios array");
    BOSS_ASSERT(j.count("OutputDir") && j["OutputDir"].is_string(), "Experiment has no OutputFile string");
    
    BuildOrderPlotter plotter;
    std::string outputDir(j["OutputDir"].get<std::string>());
    FileTools::MakeDirectory(outputDir);
    outputDir = outputDir + "/" + Assert::CurrentDateTime() + "_" + name;
    FileTools::MakeDirectory(outputDir);
    plotter.setOutputDir(outputDir);
    
    for (auto & scenario : j["Scenarios"])
    {
        BOSS_ASSERT(scenario.count("State") && scenario["State"].is_string(), "Scenario has no 'state' string");
        BOSS_ASSERT(scenario.count("BuildOrder") && scenario["BuildOrder"].is_string(), "Scenario has no 'buildOrder' string");
    
        std::cout << "    Plotting Build Order: " << scenario["BuildOrder"] << "\n";
        plotter.addPlot(scenario["BuildOrder"], BOSSConfig::Instance().GetState(scenario["State"]), BOSSConfig::Instance().GetBuildOrder(scenario["BuildOrder"]));
    }

    plotter.doPlots();

    std::cout << "    " << name << " completed" << std::endl;
}
