#include "BOSS.h"
#include "BuildOrderPlotter.h"

using namespace BOSS;

extern "C" 
{
const char * BOSS_JS_Init(char * text)
{
    try
    {
        std::cout << "Initializing BOSS\n";
        BOSS::Init("bin/BWData.json");

        // Read in the config parameters that will be used for experiments
        BOSS::BOSSConfig::Instance().ParseParameters("bin/BOSS_Config.txt");
    }
    catch (std::exception e)
    {
        std::cout << e.what();
        return e.what();
    }

    std::cout << "BOSS Initialized with " << ActionTypes::GetAllActionTypes().size() << " action types\n";
    return "success";
}
}

std::string returnString;
extern "C" 
{
const char * BOSS_JS_GetBuildOrderPlot(char * text)
{
    try
    {
        BuildOrderPlotter plotter;
        json j = json::parse(text);

        for (auto & bo : j["BuildOrders"])
        {
            BOSS_ASSERT(bo.count("State") && bo["State"].is_object(), "Scenario has no 'state' object");
            BOSS_ASSERT(bo.count("BuildOrder") && bo["BuildOrder"].is_array(), "Scenario has no 'buildOrder' array");
        
            GameState state = JSONTools::GetGameState(bo["State"]);
            BuildOrder buildOrder = JSONTools::GetBuildOrder(bo["BuildOrder"]);

            plotter.addPlot(bo["Name"], state, buildOrder);
        }

        returnString = plotter.getPlotJSON(plotter.getPlots());
        return returnString.c_str();
    }
    catch (std::exception e)
    {
        return e.what();
    }
}
}
