#include "ActionType.h"
#include "ActionTypeData.h"

using namespace BOSS;

ActionType::ActionType()
    : m_id(0)
{

}

ActionType::ActionType(const ActionType & type)
    : m_id(type.m_id)
{

}

ActionType::ActionType(const ActionID & actionID)
    : m_id(actionID)
{

}

ActionType & ActionType::operator = (const ActionType & rhs)
{
    if (this != &rhs)
    {
        new (this) ActionType(rhs);
    }

    return *this;
}   

ActionID    ActionType::getID()     const { return m_id; }
RaceID      ActionType::getRace()   const { return ActionTypeData::GetActionTypeData(m_id).race; }
const std::string & ActionType::getName()   const { return ActionTypeData::GetActionTypeData(m_id).name; }
	
int  ActionType::buildTime()         const { return ActionTypeData::GetActionTypeData(m_id).buildTime; }
int  ActionType::mineralPrice()      const { return ActionTypeData::GetActionTypeData(m_id).mineralCost; }
int  ActionType::gasPrice()          const { return ActionTypeData::GetActionTypeData(m_id).gasCost; }
int  ActionType::supplyCost()        const { return ActionTypeData::GetActionTypeData(m_id).supplyCost; }
int  ActionType::supplyProvided()    const { return ActionTypeData::GetActionTypeData(m_id).supplyProvided; }
int  ActionType::numProduced()       const { return 1; }
bool ActionType::isAddon()           const { return ActionTypeData::GetActionTypeData(m_id).isAddon; }
bool ActionType::isRefinery()        const { return ActionTypeData::GetActionTypeData(m_id).isRefinery; }
bool ActionType::isWorker()          const { return ActionTypeData::GetActionTypeData(m_id).isWorker; }
bool ActionType::isBuilding()        const { return ActionTypeData::GetActionTypeData(m_id).isBuilding; }
bool ActionType::isDepot()           const { return ActionTypeData::GetActionTypeData(m_id).isDepot; }
bool ActionType::isSupplyProvider()  const { return ActionTypeData::GetActionTypeData(m_id).isSupplyProvider; }
bool ActionType::isUnit()            const { return ActionTypeData::GetActionTypeData(m_id).isUnit; }
bool ActionType::isUpgrade()         const { return ActionTypeData::GetActionTypeData(m_id).isUpgrade; }
bool ActionType::isAbility()         const { return ActionTypeData::GetActionTypeData(m_id).isAbility; }
bool ActionType::isMorphed()         const { return false; }

ActionType ActionType::whatBuilds() const
{
    return ActionTypeData::GetActionTypeData(m_id).whatBuilds;
}

ActionType ActionType::whatBuildsAddon() const
{
    return ActionTypeData::GetActionTypeData(m_id).whatBuildsAddon;
}

const std::vector<ActionType> & ActionType::required() const
{
    return ActionTypeData::GetActionTypeData(m_id).required;
}

const std::vector<ActionType> & ActionType::equivalent() const
{
    return ActionTypeData::GetActionTypeData(m_id).equivalent;
}

const bool ActionType::operator == (const ActionType & rhs)     const { return m_id == rhs.m_id; }
const bool ActionType::operator != (const ActionType & rhs)     const { return m_id != rhs.m_id; }
const bool ActionType::operator <  (const ActionType & rhs)     const { return m_id < rhs.m_id; }

namespace BOSS
{
namespace ActionTypes
{
    std::vector<ActionType>  allActionTypes;
    std::map<std::string, ActionType> nameMap;
    std::vector<ActionType>  workerActionTypes;
    std::vector<ActionType>  refineryActionTypes;
    std::vector<ActionType>  supplyProviderActionTypes;
    std::vector<ActionType>  resourceDepotActionTypes;

    void Init()
    {
        for (size_t i(0); i < ActionTypeData::GetAllActionTypeData().size(); ++i)
        {
            allActionTypes.push_back(ActionType(i));
            nameMap[allActionTypes[i].getName()] = allActionTypes[i];
        }

        workerActionTypes.push_back(ActionTypes::GetActionType("Probe"));
        refineryActionTypes.push_back(ActionTypes::GetActionType("Assimilator"));
        supplyProviderActionTypes.push_back(ActionTypes::GetActionType("Pylon"));
        resourceDepotActionTypes.push_back(ActionTypes::GetActionType("Nexus"));

        workerActionTypes.push_back(ActionTypes::GetActionType("SCV"));
        refineryActionTypes.push_back(ActionTypes::GetActionType("Refinery"));
        supplyProviderActionTypes.push_back(ActionTypes::GetActionType("SupplyDepot"));
        resourceDepotActionTypes.push_back(ActionTypes::GetActionType("CommandCenter"));

        workerActionTypes.push_back(ActionTypes::GetActionType("Drone"));
        refineryActionTypes.push_back(ActionTypes::GetActionType("Extractor"));
        supplyProviderActionTypes.push_back(ActionTypes::GetActionType("Overlord"));
        resourceDepotActionTypes.push_back(ActionTypes::GetActionType("Hatchery"));
    }

    const ActionType & GetWorker(const RaceID raceID)
    {
        return workerActionTypes[raceID];
    }

    const ActionType & GetSupplyProvider(const RaceID raceID)
    {
        return supplyProviderActionTypes[raceID];
    }

    const ActionType & GetRefinery(const RaceID raceID)
    {
        return refineryActionTypes[raceID];
    }

    const ActionType & GetResourceDepot(const RaceID raceID)
    {
        return resourceDepotActionTypes[raceID];
    }
    
    const ActionType & GetActionType(const std::string & name)
    {
        BOSS_ASSERT(TypeExists(name), "ActionType name not found: %s", name.c_str());

        return nameMap[name];
    }

    const bool TypeExists(const std::string & name) 
    {
        return nameMap.find(name) != nameMap.end();
    }

    const std::vector<ActionType> & GetAllActionTypes()
    {
        return allActionTypes;
    }

    ActionType None(0);

}
}