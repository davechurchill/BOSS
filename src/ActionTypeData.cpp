#include "ActionTypeData.h"
#include "rapidjson/document.h"
#include "JSONTools.h"

using namespace BOSS;

std::vector<ActionTypeData>     AllActionTypeData;
std::map<std::string, size_t>   ActionTypeNameMap;

const ActionTypeData & ActionTypeData::GetActionTypeData(const std::string & name)
{
    BOSS_ASSERT(ActionTypeNameMap.find(name) != ActionTypeNameMap.end(), "Action name not found: %s", name.c_str());

    return AllActionTypeData[ActionTypeNameMap.at(name)];
}

const ActionTypeData & ActionTypeData::GetActionTypeData(const ActionID & action)
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

    std::string config = JSONTools::ReadFile(filename);

    // read the JSON file and report an error if the file couldn't be read
    if (config.length() == 0)
    {
        std::cerr << "Error: Config File Not Found or is Empty\n";
        std::cerr << "Config Filename: " << filename << "\n";
        std::cerr << "The bot will not run without its configuration file\n";
        std::cerr << "Please check that the file exists and is not empty. Incomplete paths are relative to the BOSS .exe file\n";
        return;
    }

    // parse the JSON file and report an error if the parsing failed
    rapidjson::Document doc;
    bool parsingFailed = doc.Parse(config.c_str()).HasParseError();
    if (parsingFailed)
    {
        std::cerr << "Error: Config File Found, but could not be parsed\n";
        std::cerr << "Config Filename: " << filename << "\n";
        std::cerr << "The bot will not run without its configuration file\n";
        std::cerr << "Please check that the file exists, is not empty, and is valid JSON. Incomplete paths are relative to the BOSS .exe file\n";
        return;
    }

    // read all of the action types in the file
    if (doc.HasMember("Types") && doc["Types"].IsArray())
    {
        const rapidjson::Value & actions = doc["Types"];
        for (size_t a(0); a < actions.Size(); ++a)
        {
            ActionTypeData data;

            data.id = a;
            JSONTools::ReadString("name",           actions[a], data.name);
            JSONTools::ReadString("race",           actions[a], data.raceName);
            data.race = Races::GetRaceID(data.raceName);
            JSONTools::ReadInt("mineralCost",       actions[a], data.mineralCost);
            JSONTools::ReadInt("gasCost",           actions[a], data.gasCost);
            JSONTools::ReadInt("supplyCost",        actions[a], data.supplyCost);
            JSONTools::ReadInt("energyCost",        actions[a], data.energyCost);
            JSONTools::ReadInt("supplyProvided",    actions[a], data.supplyProvided);
            JSONTools::ReadInt("buildTime",         actions[a], data.buildTime);
            JSONTools::ReadInt("numProduced",       actions[a], data.numProduced);
            JSONTools::ReadInt("startingEnergy",    actions[a], data.startingEnergy);
            JSONTools::ReadInt("maxEnergy",         actions[a], data.maxEnergy);
            JSONTools::ReadBool("isUnit",           actions[a], data.isUnit);
            JSONTools::ReadBool("isUpgrade",        actions[a], data.isUpgrade);
            JSONTools::ReadBool("isAbility",        actions[a], data.isAbility);
            JSONTools::ReadBool("isBuilding",       actions[a], data.isBuilding);
            JSONTools::ReadBool("isWorker",         actions[a], data.isWorker);
            JSONTools::ReadBool("isRefinery",       actions[a], data.isRefinery);
            JSONTools::ReadBool("isSupplyProvider", actions[a], data.isSupplyProvider);
            JSONTools::ReadBool("isResourceDepot",  actions[a], data.isDepot);
            JSONTools::ReadBool("isAddon",          actions[a], data.isAddon);

            BOSS_ASSERT(actions[a].HasMember("whatBuilds"), "no 'whatBuilds' member");
            auto & whatBuilds = actions[a]["whatBuilds"];
            data.whatBuildsStr = whatBuilds[0].GetString();
            data.whatBuildsCount = whatBuilds[1].GetInt();
            data.whatBuildsStatus = whatBuilds[2].GetString();
            if (whatBuilds.Size() == 4) { data.whatBuildsAddonStr = whatBuilds[3].GetString(); }

            BOSS_ASSERT(actions[a].HasMember("required"), "no 'required' member");
            for (size_t i(0); i < actions[a]["required"].Size(); ++i)
            {
                data.requiredStrings.push_back(actions[a]["required"][i].GetString());
            }

            BOSS_ASSERT(actions[a].HasMember("equivalent"), "no 'equivalent' member");
            for (size_t i(0); i < actions[a]["equivalent"].Size(); ++i)
            {
                data.equivalentStrings.push_back(actions[a]["equivalent"][i].GetString());
            }

            // the name map stores the index that will hold this data, which is the current size
            ActionTypeNameMap[data.name] = AllActionTypeData.size();

            // then we add the data to the vector
            AllActionTypeData.push_back(data);
             
            std::cout << AllActionTypeData.back().name << " " << AllActionTypeData.back().mineralCost << "\n";
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
            data.whatBuildsAddon = ActionType(AllActionTypeData[ActionTypeNameMap.at(data.whatBuildsAddonStr)].id);
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