#include "BOSSExperiments.h"

//#include "CombatSearchExperiment.h"
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
                //RunCombatExperiment(name, val);
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

//void Experiments::RunCombatExperiment(const std::string & name, const rapidjson::Value & val)
//{
//    CombatSearchExperiment exp(name, val);
//    exp.run();
//}

void Experiments::RunBuildOrderPlot(const std::string & name, const json & j)
{
    BuildOrderPlotter plot(name, j);
    plot.doPlots();
}