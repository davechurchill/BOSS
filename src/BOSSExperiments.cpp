#include "BOSSExperiments.h"

#include "CombatSearchExperiment.h"
#include "BuildOrderPlotter.h"

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

void Experiments::RunBuildOrderPlot(const std::string & name, const json & j)
{
    std::cout << "Build Order Plot Experiment - " << name << std::endl;

    BOSS_ASSERT(j.count("Scenarios") && j["Scenarios"].is_array(), "Experiment has no Scenarios array");
    BOSS_ASSERT(j.count("OutputDir") && j["OutputDir"].is_string(), "Experiment has no OutputFile string");
    
    BuildOrderPlotter plotter;
    plotter.setOutputDir(j["OutputDir"].get<std::string>());
    
    for (auto & scenario : j["Scenarios"])
    {
        BOSS_ASSERT(scenario.count("State") && scenario["State"].is_string(), "Scenario has no 'state' string");
        BOSS_ASSERT(scenario.count("BuildOrder") && scenario["BuildOrder"].is_string(), "Scenario has no 'buildOrder' string");
    
        plotter.addPlot(scenario["BuildOrder"], BOSSConfig::Instance().GetState(scenario["State"]), BOSSConfig::Instance().GetBuildOrder(scenario["BuildOrder"]));
    }

    plotter.doPlots();

    std::cout << "    " << name << " completed" << std::endl;
}