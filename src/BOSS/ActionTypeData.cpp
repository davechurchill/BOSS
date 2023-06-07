#include "ActionTypeData.h"
#include "JSONTools.h"

using namespace BOSS;

std::vector<ActionTypeData>     AllActionTypeData;
std::map<std::string, size_t>   ActionTypeNameMap;

const ActionTypeData & ActionTypeData::GetActionTypeData(const std::string & name)
{
    BOSS_ASSERT(ActionTypeNameMap.find(name) != ActionTypeNameMap.end(), "Action name not found: %s", name.c_str());

    return AllActionTypeData[ActionTypeNameMap.at(name)];
}

const ActionTypeData & ActionTypeData::GetActionTypeData(const ActionID action)
{
    BOSS_ASSERT(action < AllActionTypeData.size(), "ActionID overflow: %d", action);

    return AllActionTypeData[action];
}

const std::vector<ActionTypeData> & ActionTypeData::GetAllActionTypeData()
{
    return AllActionTypeData;
}

void ActionTypeData::Init(const std::string & filename)
{
    // add the None type for error returns
    AllActionTypeData.push_back(ActionTypeData());
    ActionTypeNameMap["None"] = 0;
    
    // parse the JSON file and report an error if the parsing failed
    std::ifstream file(filename);
    json j;

    try 
    {
        file >> j;
    }
    catch (json::parse_error e) 
    {
        std::cerr << e.what() << "\n";
        BOSS_ASSERT(false, "Can't Parse Data file: %s", filename.c_str());
    }

    // read all of the action types in the file
    if (j.count("Types") && j["Types"].is_array())
    {
        const json & actions = j["Types"];
        for (size_t a(0); a < actions.size(); ++a)
        {
            ActionTypeData data;

            data.id = a;
            JSONTools::ReadString("name",           actions[a], data.name);
            JSONTools::ReadString("race",           actions[a], data.raceName);
            data.race = (int)Races::GetRaceID(data.raceName);
            JSONTools::ReadInt("mineralCost",       actions[a], data.mineralCost);
            JSONTools::ReadInt("gasCost",           actions[a], data.gasCost);
            JSONTools::ReadInt("supplyCost",        actions[a], data.supplyCost);
            JSONTools::ReadInt("energyCost",        actions[a], data.energyCost);
            JSONTools::ReadInt("supplyProvided",    actions[a], data.supplyProvided);
            JSONTools::ReadInt("buildTime",         actions[a], data.buildTime);
            JSONTools::ReadInt("numProduced",       actions[a], data.numProduced);
            JSONTools::ReadInt("startingEnergy",    actions[a], data.startingEnergy);
            JSONTools::ReadInt("maxEnergy",         actions[a], data.maxEnergy);
            JSONTools::ReadInt("buildLimit",        actions[a], data.buildLimit);
            JSONTools::ReadBool("isUnit",           actions[a], data.isUnit);
            JSONTools::ReadBool("isUpgrade",        actions[a], data.isUpgrade);
            JSONTools::ReadBool("isAbility",        actions[a], data.isAbility);
            JSONTools::ReadBool("isBuilding",       actions[a], data.isBuilding);
            JSONTools::ReadBool("isWorker",         actions[a], data.isWorker);
            JSONTools::ReadBool("isRefinery",       actions[a], data.isRefinery);
            JSONTools::ReadBool("isSupplyProvider", actions[a], data.isSupplyProvider);
            JSONTools::ReadBool("isResourceDepot",  actions[a], data.isDepot);
            JSONTools::ReadBool("isAddon",          actions[a], data.isAddon);
            JSONTools::ReadBool("isTech",           actions[a], data.isTech);
            // muptiply here so we don't have to do it everywhere else
            // only matters for zergling and scourge
            data.supplyCost *= data.numProduced;

            if (data.name == "Hatchery" || data.name == "Lair" || data.name == "Hive")
            {
                data.isHatchery = true;
            }

            // Limit each tech to 1
            if (data.buildLimit == -1 && data.isTech)
            {
                data.buildLimit = 1;
            }

            BOSS_ASSERT(actions[a].count("whatBuilds"), "no 'whatBuilds' member");
            auto & whatBuilds = actions[a]["whatBuilds"];
            if (whatBuilds.size() > 0)
            {
                data.whatBuildsStr = whatBuilds[0].get<std::string>();
                data.whatBuildsCount = whatBuilds[1];
                data.whatBuildsStatus = whatBuilds[2].get<std::string>();
            }
            data.isMorphed = (data.whatBuildsStatus == "Morphed");
            if (whatBuilds.size() == 4) 
            { 
                data.whatBuildsAddonStr = whatBuilds[3].get<std::string>(); 
            }

            BOSS_ASSERT(actions[a].count("required"), "no 'required' member");
            for (auto & req : actions[a]["required"])
            {
                data.requiredStrings.push_back(req);
            }

            BOSS_ASSERT(actions[a].count("equivalent"), "no 'equivalent' member");
            for (auto & equiv : actions[a]["equivalent"])
            {
                data.equivalentStrings.push_back(equiv);
            }

            // the name map stores the index that will hold this data, which is the current size
            ActionTypeNameMap[data.name] = AllActionTypeData.size();

            // then we add the data to the vector
            AllActionTypeData.push_back(data);
             
            //std::cout << AllActionTypeData.back().name << " " << AllActionTypeData.back().mineralCost << "\n";
        }
    }

    // now we have to re-iterate over all established types to get the ids
    for (auto & data : AllActionTypeData)
    {
        if (data.whatBuildsStr.size() > 0)
        {
            // get the types of the thing that builds this type
            data.whatBuilds = ActionType(ActionTypeNameMap.at(data.whatBuildsStr));
        }

        if (data.whatBuildsAddonStr.size() > 0) 
        {
            // get the types of the addon required by the builder of this thing
            data.whatBuildsAddon = ActionType(ActionTypeNameMap.at(data.whatBuildsAddonStr));
        }

        // add the types of all the equivalent types
        for (size_t i(0); i < data.equivalentStrings.size(); ++i)
        {
            data.equivalent.push_back(ActionType(ActionTypeNameMap.at(data.equivalentStrings[i])));
        }

        // add the ids of all the prerequisites
        for (size_t i(0); i < data.requiredStrings.size(); ++i)
        {
            if (ActionTypeNameMap.find(data.requiredStrings[i]) != ActionTypeNameMap.end())
            {
                data.required.push_back(ActionType(ActionTypeNameMap.at(data.requiredStrings[i])));
            }
        }
    }
}

