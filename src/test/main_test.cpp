#define CATCH_CONFIG_MAIN

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
        ActionType act(build);
        state.doAction(act); 
        if (act.buildLimit() == state.getNumTotal(build))
        {
            legal[build] = false;
        }
    }

    for (auto& kv : legal)
    {
        if (state.isLegal(ActionType(kv.first)) == kv.second)
        {
            REQUIRE(true);
        }
        else
        {
            std::cout << build << " Legal Fail: " << kv.first << " Should be " << (kv.second ? "legal" : "illegal") << ", is " << (kv.second ? "illegal" : "legal") << std::endl;
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
    DoLegalCheck(state, legal, "TemplarArchives", { "HighTemplar", "DarkTemplar", "PsionicStorm", "Hallucination", "MindControl", "Maelstrom"});
    DoLegalCheck(state, legal, "Stargate", { "Corsair", "Scout", "FleetBeacon", "ArbiterTribunal" });
    DoLegalCheck(state, legal, "FleetBeacon", { "Carrier", "DisruptionWeb"});
    DoLegalCheck(state, legal, "ArbiterTribunal", { "Arbiter", "Recall", "StasisField" });
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
    DoLegalCheck(state, legal, "Marine", { });
    DoLegalCheck(state, legal, "Bunker", { });
    DoLegalCheck(state, legal, "Academy", { "Medic", "Firebat", "ComsatStation", "StimPacks", "Restoration", "OpticalFlare"});
    DoLegalCheck(state, legal, "Medic", { });
    DoLegalCheck(state, legal, "SupplyDepot", { });
    DoLegalCheck(state, legal, "Firebat", { });
    DoLegalCheck(state, legal, "Factory", { "Vulture", "MachineShop", "Starport", "Armory" });
    DoLegalCheck(state, legal, "Factory", { });
    DoLegalCheck(state, legal, "MachineShop", { "SiegeTank", "SpiderMines", "TankSiegeMode"});
    DoLegalCheck(state, legal, "SiegeTank", { });
    DoLegalCheck(state, legal, "Armory", { "Goliath" });
    DoLegalCheck(state, legal, "Goliath", { });
    DoLegalCheck(state, legal, "Starport", { "Wraith", "ControlTower", "ScienceFacility" }); 
    DoLegalCheck(state, legal, "Wraith", { });
    DoLegalCheck(state, legal, "SupplyDepot", { });
    DoLegalCheck(state, legal, "Starport", { });
    DoLegalCheck(state, legal, "ControlTower", { "Valkyrie",  "Dropship", "CloakingField"});
    DoLegalCheck(state, legal, "Valkyrie", { });
    DoLegalCheck(state, legal, "Dropship", { });
    DoLegalCheck(state, legal, "ScienceFacility", { "PhysicsLab", "CovertOps", "ScienceVessel", "EMPShockwave", "Irradiate" });
    DoLegalCheck(state, legal, "ScienceFacility", { });
    DoLegalCheck(state, legal, "PhysicsLab", { "Battlecruiser", "YamatoGun" });
    DoLegalCheck(state, legal, "SupplyDepot", { });
    DoLegalCheck(state, legal, "Battlecruiser", { });
    legal["CovertOps"] = false;
    legal["PhysicsLab"] = false;
    DoLegalCheck(state, legal, "CovertOps", { "Ghost", "NuclearSilo", "Lockdown", "PersonnelCloaking" });
    DoLegalCheck(state, legal, "SupplyDepot", { });
    DoLegalCheck(state, legal, "CommandCenter", {"Refinery"});
    DoLegalCheck(state, legal, "NuclearSilo", { "NuclearMissile" });
    DoLegalCheck(state, legal, "ScienceFacility", { "CovertOps", "PhysicsLab" });
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

TEST_CASE("Zerg Tech Tree")
{
    BOSS::Init("config/BWData.json");

    BOSS::GameState state;
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Overlord"));
    state.addUnit(ActionType("Hatchery"));
    state.setMinerals(50);

    std::map<std::string, bool> legal;
    for (auto& a : ActionTypes::GetAllActionTypes())
    {
        legal[a.getName()] = false;
    }

    // test the base legal units
    DoLegalCheck(state, legal, "", { "Drone", "Overlord", "SpawningPool", "Extractor", "Hatchery", "CreepColony", "EvolutionChamber" });

    // build some more workers and gas so everything will eventually be legal
    // zerg needs a ton of drones so just make them all in the beginning
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Overlord"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Extractor"));
    state.doAction(ActionType("Hatchery"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Overlord"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Hatchery"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Overlord"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Overlord"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));

    DoLegalCheck(state, legal, "", { "Burrowing" });
    // build stuff in order in the tech tree, testing legality as we go
    DoLegalCheck(state, legal, "CreepColony", {});
    DoLegalCheck(state, legal, "CreepColony", {});
    DoLegalCheck(state, legal, "CreepColony", {});
    DoLegalCheck(state, legal, "SpawningPool", { "Zergling", "SunkenColony", "HydraliskDen", "Lair" });
    DoLegalCheck(state, legal, "EvolutionChamber", { "SporeColony" });
    DoLegalCheck(state, legal, "Zergling", { });
    DoLegalCheck(state, legal, "SporeColony", { });
    DoLegalCheck(state, legal, "SunkenColony", { });
    DoLegalCheck(state, legal, "HydraliskDen", { "Hydralisk" });
    DoLegalCheck(state, legal, "Hydralisk", {});
    DoLegalCheck(state, legal, "Lair", { "QueensNest", "Spire", "LurkerAspect"});
    DoLegalCheck(state, legal, "LurkerAspect", { "Lurker" });
    DoLegalCheck(state, legal, "Spire", { "Mutalisk", "Scourge" });
    DoLegalCheck(state, legal, "QueensNest", { "Queen", "Hive", "Ensnare", "SpawnBroodlings" });
    DoLegalCheck(state, legal, "Queen", { });
    DoLegalCheck(state, legal, "Lair", { }); // 2nd lair so hive won't be illegal next
    DoLegalCheck(state, legal, "Hive", { "DefilerMound", "NydusCanal", "UltraliskCavern", "GreaterSpire" });
    DoLegalCheck(state, legal, "DefilerMound", { "Defiler", "Consume", "Plague"});
    DoLegalCheck(state, legal, "Defiler", { });
    DoLegalCheck(state, legal, "Overlord", { });
    DoLegalCheck(state, legal, "UltraliskCavern", { "Ultralisk" });
    DoLegalCheck(state, legal, "Ultralisk", { });
    DoLegalCheck(state, legal, "Overlord", { });
    DoLegalCheck(state, legal, "Mutalisk", { });
    DoLegalCheck(state, legal, "Mutalisk", { });
    DoLegalCheck(state, legal, "Mutalisk", { });
    DoLegalCheck(state, legal, "Spire", { }); // so gspire won't be illegal
    DoLegalCheck(state, legal, "GreaterSpire", { "Devourer", "Guardian" });
    DoLegalCheck(state, legal, "Overlord", { });
    DoLegalCheck(state, legal, "Devourer", { });
    DoLegalCheck(state, legal, "Guardian", { });
}

TEST_CASE("Zerg Use All Drones")
{
    BOSS::Init("config/BWData.json");

    BOSS::GameState state;
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Overlord"));
    state.addUnit(ActionType("Hatchery"));
    state.setMinerals(50);

    state.doAction(ActionType("SpawningPool"));
    state.doAction(ActionType("SpawningPool"));
    state.doAction(ActionType("SpawningPool"));
    state.doAction(ActionType("SpawningPool"));

    REQUIRE(!state.isLegal(ActionType("SpawningPool")));
}

TEST_CASE("Zerg 3 Drones Gas Only")
{
    /*BOSS::Init("config/BWData.json");

    BOSS::GameState state;
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Overlord"));
    state.addUnit(ActionType("Hatchery"));
    state.setMinerals(50);

    state.doAction(ActionType("SpawningPool"));
    state.doAction(ActionType("Extractor"));
    state.fastForward(state.getCurrentFrame() + 500);

    REQUIRE(!state.isLegal(ActionType("SpawningPool")));*/
}

void SupplySanityCheck(const GameState& state)
{
    int supplyUsedSum = 0;
    int supplyTotalSum = 0;

    for (auto& u : state.getUnits())
    {
        supplyUsedSum += u.getType().supplyCost();
        
        if (u.getTimeUntilFree() == 0)
        {
            supplyTotalSum += u.getType().supplyProvided();
        }
    }

    if (supplyTotalSum != state.getMaxSupply())

    REQUIRE(supplyUsedSum == state.getCurrentSupply());
    REQUIRE(supplyTotalSum == state.getMaxSupply());
}

TEST_CASE("Zerg Supply Sanity Test")
{
    BOSS::Init("config/BWData.json");

    BOSS::GameState state;
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Overlord"));
    state.addUnit(ActionType("Hatchery"));
    state.setMinerals(50);

    SupplySanityCheck(state);

    std::vector<std::string> bos = { "Drone", "Drone", "Drone", "Drone", "Drone", "Overlord", "Drone", "Drone", "Drone", "Hatchery", "SpawningPool", "Drone", "Extractor", "Drone", "Drone", "Drone", "Drone", "Drone", "Drone", "HydraliskDen", "Drone", "Overlord", "Drone", "Drone",  "Overlord", "Drone", "Hydralisk", "Hydralisk", "Hydralisk", "Hydralisk", "Hydralisk", "Hydralisk", "Hydralisk", "Hydralisk", "Hydralisk", "Hydralisk", "Hydralisk", "Hydralisk", "Hatchery", "Extractor" };

    for (size_t i = 0; i < bos.size(); i++)
    {
        ActionType type(bos[i]);
        state.doAction(type);
        SupplySanityCheck(state);
    }
}

TEST_CASE("Zerg Supply Legal Test")
{
    BOSS::Init("config/BWData.json");

    BOSS::GameState state;
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Drone"));
    state.addUnit(ActionType("Overlord"));
    state.addUnit(ActionType("Hatchery"));
    state.setMinerals(50);

    GameState state2(state);

    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));
    state.doAction(ActionType("Drone"));

    state2.doAction(ActionType("Drone"));
    state2.doAction(ActionType("Drone"));
    state2.doAction(ActionType("Drone"));
    state2.doAction(ActionType("SpawningPool"));
    state2.doAction(ActionType("Drone"));
    state2.doAction(ActionType("Drone"));
    state2.doAction(ActionType("Drone"));
    
    REQUIRE(!state.isLegal(ActionType("Drone")));
    REQUIRE(!state2.isLegal(ActionType("Drone")));
    REQUIRE(!state2.isLegal(ActionType("Zergling")));
}

TEST_CASE("Morph Supply Test")
{
    // Checks that supply is not being double counted when a unit morphs into another unit that uses supply
    BOSS::GameState state;
    state.addUnit(ActionType("Overlord"));
    state.addUnit(ActionType("HydraliskDen"));
    state.addUnit(ActionType("LurkerAspect"));
    state.addUnit(ActionType("Larva"));
    int supply = state.getCurrentSupply();
    state.setGas(125);
    state.setMinerals(125);
    state.doAction(ActionType("Hydralisk"));
    REQUIRE(state.getCurrentSupply() == supply + 2);
    state.doAction(ActionType("Lurker"));
    REQUIRE(state.getCurrentSupply() == supply + 4);
}

void SimulateEachFrameTo(GameState& state, const ActionType& type)
{
    while (!state.canBuildNow(type))
    {
        state.fastForward(state.getCurrentFrame() + 1);
    }
}

void FastForwardTo(GameState& state, const ActionType& type)
{
    state.fastForward(state.whenCanBuild(type));
}

TEST_CASE("Zerg FF vs Frames")
{
    std::vector<std::string> bos = { "Drone", "Drone", "Drone", "Drone", "Drone", "SpawningPool", "Drone", "Extractor", "Drone", "Overlord", "Lair", "Drone" };
    bos = { "Drone", "Drone", "Drone", "Drone", "Overlord", "Drone", "Drone", "Drone", "Drone", "Hatchery", "SpawningPool", "Drone", "Drone", "Hatchery", "Extractor", "Drone", "Drone", "Drone", "Overlord", "Lair", "Extractor", "Drone", "Drone", "Zergling", "Zergling", "Zergling", "Drone", "Drone", "Drone", "Drone", "Overlord", "Drone", "Overlord", "Drone", "Overlord", "Spire", "Overlord", "Drone", "Drone", "Drone", "Drone", "Drone", "Mutalisk", "Mutalisk", "Mutalisk", "Mutalisk", "Mutalisk", "Mutalisk", "Mutalisk", "Mutalisk", "Mutalisk", "Mutalisk", "Mutalisk", "Mutalisk" };

    BOSS::GameState state1;
    state1.addUnit(ActionType("Drone"));
    state1.addUnit(ActionType("Drone"));
    state1.addUnit(ActionType("Drone"));
    state1.addUnit(ActionType("Drone"));
    state1.addUnit(ActionType("Overlord"));
    state1.addUnit(ActionType("Hatchery"));
    state1.setMinerals(50);

    BOSS::GameState state2(state1);

    for (size_t i = 0; i < bos.size(); i++)
    {
        ActionType type(bos[i]);

        SimulateEachFrameTo(state1, type);
        state1.doAction(type);

        FastForwardTo(state2, type);
        state2.doAction(type);

        if (state1.getCurrentFrame() != state2.getCurrentFrame())
        {
            int a = 6;
        }

        REQUIRE(state1.getCurrentFrame() == state2.getCurrentFrame());
    }
}

TEST_CASE("Tech Test")
{
    BOSS::GameState state;
    state.addUnit(ActionType("Hatchery"));
    state.setMinerals(100);
    state.setGas(100);
    // Basic test
    auto act = ActionType("Burrowing");
    state.doAction(act);
    REQUIRE(state.haveType(act));

    // Test that techs cannot be built twice
    state.setMinerals(100);
    state.setGas(100);
    REQUIRE(!state.isLegal(act));
}

TEST_CASE("Equivalencies")
{
    // test isEquivalentTo() method
    ActionType hive("Hive");
    ActionType hatchery("Hatchery");
    REQUIRE(hive.isEquivalentTo(hatchery));
    REQUIRE(!hatchery.isEquivalentTo(hive));

    BOSS::GameState state1;
    state1.addUnit(hive);
    state1.setMinerals(100);
    state1.setGas(100);

    // test that equivalency works for non-morphing building
    auto act1 = ActionType("Burrowing");
    state1.doAction(act1);
    REQUIRE(state1.haveType(act1));

    state1.addUnit(ActionType("Drone"));

    // test that equivalency isn't allowed for morphing
    REQUIRE(!state1.isLegal(ActionType("Lair")));
    
    // test that equivalency works for requirements
    REQUIRE(state1.isLegal(ActionType("EvolutionChamber")));

    // Tests that requirements met by a unit continue to be met while it morphes into another unit that also meets those requirements
    BOSS::GameState state2;
    state2.addUnit(ActionType("Drone"));
    state2.addUnit(ActionType("Hatchery"));
    state2.addUnit(ActionType("SpawningPool"));
    state2.setGas(100);
    state2.setMinerals(225);
    state2.doAction(ActionType("Lair"));
    REQUIRE(state2.isLegal(ActionType("EvolutionChamber")));
    REQUIRE(state2.whenCanBuild(ActionType("EvolutionChamber"))==state2.getCurrentFrame());
    // Tests that the time till builder is available is correctly estimated for equivalents to builder
    state2.setGas(100);
    state2.setMinerals(100);
    REQUIRE(state2.whenCanBuild(ActionType("Burrowing")) == state2.getNextFinishTime(ActionType("Lair")));
}

TEST_CASE("Addons")
{
    BOSS::GameState state;
    ActionType command("CommandCenter");
    state.addUnit(command);
    state.setGas(200);
    state.setMinerals(200);
    state.addUnit(ActionType("ScienceFacility"));
    state.addUnit(ActionType("CovertOps"));
    ActionType silo("NuclearSilo");
    REQUIRE(state.isLegal(silo));
    state.doAction(silo);
    REQUIRE(!state.isLegal(silo));
}