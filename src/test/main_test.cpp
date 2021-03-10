#include "BOSS.h"

using namespace BOSS;

#include "catch2/catch_amalgamated.hpp"

TEST_CASE("Init BOSS")
{
    BOSS::Init("config/BWData.json");
    REQUIRE(true);
}

void DoLegalCheck(GameState& state, std::map<std::string, bool>& legal, const std::string & build, const std::vector<std::string> & nowLegal)
{
    for (auto& s : nowLegal) { legal[s] = true; }

    if (build.size() > 0) 
    { 
        REQUIRE(state.isLegal(build)); 
        state.doAction(build); 
    }

    for (auto& kv : legal)
    {
        if (state.isLegal(ActionType(kv.first)) == kv.second)
        {
            REQUIRE(true);
        }
        else
        {
            std::cout << build << " Legal Fail: " << kv.first << std::endl;
            REQUIRE(false);
        }
        
    }
}

TEST_CASE("Blank State")
{
    GameState blankState;

    REQUIRE(blankState.getUnits().empty());
    REQUIRE(blankState.getCurrentSupply() == 0);
    REQUIRE(blankState.getMinerals() == 0);
    REQUIRE(blankState.getGas() == 0);
}

TEST_CASE("Protoss Start State")
{
    BOSS::GameState startState;
    startState.addUnit(ActionType("Nexus"));
    startState.addUnit(ActionType("Probe"));
    startState.addUnit(ActionType("Probe"));
    startState.addUnit(ActionType("Probe"));
    startState.addUnit(ActionType("Probe"));
    startState.setMinerals(50);

    REQUIRE(startState.getNumCompleted(ActionType("Probe")) == 4);
    REQUIRE(startState.getNumCompleted(ActionType("Nexus")) == 1);
    REQUIRE(startState.getNumMineralWorkers() == 4);
    REQUIRE(startState.getNumGasWorkers() == 0);
    REQUIRE(startState.getCurrentSupply() == 8);
    
    REQUIRE(startState.whenCanBuild(ActionType("Probe")) == startState.getCurrentFrame());
    REQUIRE(startState.whenCanBuild(ActionType("Nexus")) > startState.getCurrentFrame());
}

TEST_CASE("Protoss Tech Tree") 
{
    BOSS::Init("config/BWData.json");

    BOSS::GameState state;
    state.addUnit(ActionType("Nexus"));
    state.addUnit(ActionType("Probe"));
    state.addUnit(ActionType("Probe"));
    state.addUnit(ActionType("Probe"));
    state.addUnit(ActionType("Probe"));
    state.setMinerals(50);

    std::map<std::string, bool> legal;
    for (auto& a : ActionTypes::GetAllActionTypes())
    {
        legal[a.getName()] = false;
    }

    // test the base legal units
    DoLegalCheck(state, legal, "", { "Probe", "Nexus", "Pylon", "Assimilator" });

    // build some more workers and gas
    state.doAction(ActionType("Probe"));
    state.doAction(ActionType("Probe"));
    state.doAction(ActionType("Probe"));
    state.doAction(ActionType("Probe"));
    state.doAction(ActionType("Assimilator"));

    // we can't have more assimilators than nexus
    legal["Assimilator"] = false;

    // build stuff in order in the tech tree, testing legality as we go
    DoLegalCheck(state, legal, "Pylon", { "Forge", "Gateway" });
    DoLegalCheck(state, legal, "Forge", { "PhotonCannon" });
    DoLegalCheck(state, legal, "Gateway", { "Zealot", "CyberneticsCore", "ShieldBattery" });
    DoLegalCheck(state, legal, "CyberneticsCore", { "CitadelofAdun", "RoboticsFacility", "Stargate", "Dragoon" });
    DoLegalCheck(state, legal, "RoboticsFacility", { "RoboticsSupportBay", "Observatory", "Shuttle" });
    DoLegalCheck(state, legal, "RoboticsSupportBay", { "Reaver" });
    DoLegalCheck(state, legal, "Observatory", { "Observer" });
    DoLegalCheck(state, legal, "CitadelofAdun", { "TemplarArchives" });
    DoLegalCheck(state, legal, "TemplarArchives", { "HighTemplar", "DarkTemplar" });
    DoLegalCheck(state, legal, "Stargate", { "Corsair", "Scout", "FleetBeacon", "ArbiterTribunal" });
    DoLegalCheck(state, legal, "FleetBeacon", { "Carrier" });
    DoLegalCheck(state, legal, "ArbiterTribunal", { "Arbiter" });
    DoLegalCheck(state, legal, "Nexus", { "Assimilator" });
}

TEST_CASE("Terran Start State")
{
    BOSS::GameState startState;
    startState.addUnit(ActionType("CommandCenter"));
    startState.addUnit(ActionType("SCV"));
    startState.addUnit(ActionType("SCV"));
    startState.addUnit(ActionType("SCV"));
    startState.addUnit(ActionType("SCV"));
    startState.setMinerals(50);

    REQUIRE(startState.getNumCompleted(ActionType("SCV")) == 4);
    REQUIRE(startState.getNumCompleted(ActionType("CommandCenter")) == 1);
    REQUIRE(startState.getNumMineralWorkers() == 4);
    REQUIRE(startState.getNumGasWorkers() == 0);
    REQUIRE(startState.getCurrentSupply() == 8);

    REQUIRE(startState.whenCanBuild(ActionType("SCV")) == startState.getCurrentFrame());
    REQUIRE(startState.whenCanBuild(ActionType("CommandCenter")) > startState.getCurrentFrame());
}

TEST_CASE("Terran Tech Tree")
{
    BOSS::Init("config/BWData.json");

    BOSS::GameState state;
    state.addUnit(ActionType("CommandCenter"));
    state.addUnit(ActionType("SCV"));
    state.addUnit(ActionType("SCV"));
    state.addUnit(ActionType("SCV"));
    state.addUnit(ActionType("SCV"));
    state.setMinerals(50);

    std::map<std::string, bool> legal;
    for (auto& a : ActionTypes::GetAllActionTypes())
    {
        legal[a.getName()] = false;
    }

    // test the base legal units
    DoLegalCheck(state, legal, "", { "SCV", "CommandCenter", "Refinery", "SupplyDepot", "Barracks", "EngineeringBay"});

    // build some more workers and gas so everything will eventually be legal
    state.doAction(ActionType("SCV"));
    state.doAction(ActionType("SCV"));
    state.doAction(ActionType("SCV"));
    state.doAction(ActionType("SCV"));
    state.doAction(ActionType("SCV"));
    state.doAction(ActionType("SCV"));
    state.doAction(ActionType("Refinery"));
    state.doAction(ActionType("SupplyDepot"));

    // we can't have more assimilators than nexus
    legal["Refinery"] = false;

    // build stuff in order in the tech tree, testing legality as we go
    DoLegalCheck(state, legal, "EngineeringBay", { "MissileTurret" });
    DoLegalCheck(state, legal, "Barracks", { "Marine", "Bunker", "Academy", "Factory" });
    DoLegalCheck(state, legal, "Bunker", { });
    DoLegalCheck(state, legal, "Academy", { "Medic", "Firebat", "ComsatStation" });
    DoLegalCheck(state, legal, "Factory", { "Vulture", "MachineShop", "Starport", "Armory" });
    DoLegalCheck(state, legal, "Factory", { });
    DoLegalCheck(state, legal, "MachineShop", { "SiegeTank" });
    DoLegalCheck(state, legal, "Armory", { "Goliath" });
    DoLegalCheck(state, legal, "Starport", { "Wraith", "ControlTower", "ScienceFacility" }); 
    DoLegalCheck(state, legal, "Starport", { });
    DoLegalCheck(state, legal, "ControlTower", { "Valkyrie",  "Dropship" });
    DoLegalCheck(state, legal, "ScienceFacility", {  "PhysicsLab", "CovertOps", "ScienceVessel" });
    DoLegalCheck(state, legal, "ScienceFacility", {  });
    DoLegalCheck(state, legal, "PhysicsLab", { "Battlecruiser" });
    DoLegalCheck(state, legal, "CovertOps", { "Ghost", "NuclearSilo" });
    DoLegalCheck(state, legal, "ScienceFacility", {  });
    DoLegalCheck(state, legal, "SupplyDepot", { });
    DoLegalCheck(state, legal, "Battlecruiser", { });
    DoLegalCheck(state, legal, "Ghost", { });
}

TEST_CASE("Terran Build With All Workers")
{
    BOSS::GameState state;
    state.addUnit(ActionType("CommandCenter"));
    state.addUnit(ActionType("SCV"));
    state.addUnit(ActionType("SCV"));
    state.addUnit(ActionType("SCV"));
    state.addUnit(ActionType("SCV"));
    state.setMinerals(5000);

    BuildOrder bo;
    bo.add(ActionType("SupplyDepot"), 10);

    BuildOrder bo2;
    bo2.add(ActionType("Barracks"), 6);

    
    BuildOrderPlotter::QuickPlot(state, { bo, bo2 });
}