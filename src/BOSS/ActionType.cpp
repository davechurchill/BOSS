#include "ActionType.h"
#include "ActionTypeData.h"
#include "ActionSet.h"

using namespace BOSS;

std::vector<ActionSet> allActionPrerequisites;
std::vector<ActionSet> allActionRecursivePrerequisites;

ActionType::ActionType()
{

}

ActionType::ActionType(const ActionID actionID)
    : m_id(actionID)
{

}

ActionType::ActionType(const std::string& str)
{
    *this = ActionTypes::GetActionType(str);
}

ActionType & ActionType::operator = (const ActionType & rhs)
{
    if (this != &rhs)
    {
        new (this) ActionType(rhs);
    }

    return *this;
}   

ActionID    ActionType::getID()      const { return m_id; }
RaceID      ActionType::getRace()    const { return ActionTypeData::GetActionTypeData(m_id).race; }
const std::string & ActionType::getName()   const { return ActionTypeData::GetActionTypeData(m_id).name; }
	
int  ActionType::buildTime()         const { return ActionTypeData::GetActionTypeData(m_id).buildTime; }
int  ActionType::mineralPrice()      const { return ActionTypeData::GetActionTypeData(m_id).mineralCost; }
int  ActionType::gasPrice()          const { return ActionTypeData::GetActionTypeData(m_id).gasCost; }
int  ActionType::supplyCost()        const { return ActionTypeData::GetActionTypeData(m_id).supplyCost; }
int  ActionType::supplyProvided()    const { return ActionTypeData::GetActionTypeData(m_id).supplyProvided; }
int  ActionType::numProduced()       const { return 1; }
int  ActionType::buildLimit()        const { return ActionTypeData::GetActionTypeData(m_id).buildLimit; }
bool ActionType::isAddon()           const { return ActionTypeData::GetActionTypeData(m_id).isAddon; }
bool ActionType::isRefinery()        const { return ActionTypeData::GetActionTypeData(m_id).isRefinery; }
bool ActionType::isWorker()          const { return ActionTypeData::GetActionTypeData(m_id).isWorker; }
bool ActionType::isBuilding()        const { return ActionTypeData::GetActionTypeData(m_id).isBuilding; }
bool ActionType::isDepot()           const { return ActionTypeData::GetActionTypeData(m_id).isDepot; }
bool ActionType::isSupplyProvider()  const { return ActionTypeData::GetActionTypeData(m_id).isSupplyProvider; }
bool ActionType::isUnit()            const { return ActionTypeData::GetActionTypeData(m_id).isUnit; }
bool ActionType::isUpgrade()         const { return ActionTypeData::GetActionTypeData(m_id).isUpgrade; }
bool ActionType::isAbility()         const { return ActionTypeData::GetActionTypeData(m_id).isAbility; }
bool ActionType::isMorphed()         const { return ActionTypeData::GetActionTypeData(m_id).isMorphed; }
bool ActionType::isHatchery()        const { return ActionTypeData::GetActionTypeData(m_id).isHatchery; }
bool ActionType::isTech()            const { return ActionTypeData::GetActionTypeData(m_id).isTech; }

ActionType ActionType::whatBuilds() const
{
    return ActionTypeData::GetActionTypeData(m_id).whatBuilds;
}

ActionType ActionType::whatBuildsAddon() const
{
    return ActionTypeData::GetActionTypeData(m_id).whatBuildsAddon;
}

int ActionType::whatBuildsCount() const
{
    return ActionTypeData::GetActionTypeData(m_id).whatBuildsCount;
}

const std::vector<ActionType> & ActionType::required() const
{
    return ActionTypeData::GetActionTypeData(m_id).required;
}

const std::vector<ActionType> & ActionType::equivalent() const
{
    return ActionTypeData::GetActionTypeData(m_id).equivalent;
}


const ActionSet & ActionType::getPrerequisiteActionCount() const
{
    return allActionPrerequisites[m_id];
}

const ActionSet & ActionType::getRecursivePrerequisiteActionCount() const
{
    return allActionRecursivePrerequisites[m_id];
}

bool ActionType::operator == (const ActionType rhs)     const { return m_id == rhs.m_id; }
bool ActionType::operator != (const ActionType rhs)     const { return m_id != rhs.m_id; }
bool ActionType::operator <  (const ActionType rhs)     const { return m_id < rhs.m_id; }

bool ActionType::isEquivalentTo(const ActionType other)   const 
{
    if (*this != other)
    {
        auto& equivalents = other.equivalent();
        auto it = std::find(equivalents.begin(), equivalents.end(), *this);
        if (it == equivalents.end()) { return false; }
    }
    return true;
};

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

        // calculate all action prerequisites
        for (size_t i(0); i < allActionTypes.size(); ++i)
        {
            allActionPrerequisites.push_back(CalculatePrerequisites(allActionTypes[i]));
        }

        // calculate all action recursive prerequisites
        for (size_t i(0); i < allActionTypes.size(); ++i)
        {
            ActionSet recursivePrerequisites;
            CalculateRecursivePrerequisites(recursivePrerequisites, allActionTypes[i]);
            allActionRecursivePrerequisites.push_back(recursivePrerequisites);
        }
    }

    ActionType GetWorker(const RaceID raceID)
    {
        return workerActionTypes[raceID];
    }

    ActionType GetSupplyProvider(const RaceID raceID)
    {
        return supplyProviderActionTypes[raceID];
    }

    ActionType GetRefinery(const RaceID raceID)
    {
        return refineryActionTypes[raceID];
    }

    ActionType GetResourceDepot(const RaceID raceID)
    {
        return resourceDepotActionTypes[raceID];
    }
    
    ActionType GetActionType(const std::string & name)
    {
        BOSS_ASSERT(TypeExists(name), "ActionType name not found: %s", name.c_str());

        return nameMap[name];
    }

    bool TypeExists(const std::string & name) 
    {
        return nameMap.find(name) != nameMap.end();
    }

    const std::vector<ActionType> & GetAllActionTypes()
    {
        return allActionTypes;
    }

    ActionType None(0);

    ActionSet CalculatePrerequisites(const ActionType & action)
    {
        ActionSet count;

        count.add(action.whatBuilds());
        auto addon = action.whatBuildsAddon();
        if (addon != None) { count.add(addon); }
        for (auto& req : action.required())
        {
            count.add(req);
        }
        return count;
    }

    void CalculateRecursivePrerequisites(ActionSet & allActions, const ActionType & action)
    {
        ActionSet pre = action.getPrerequisiteActionCount();

        if (action.gasPrice() > 0)
        {
            pre.add(ActionTypes::GetRefinery(action.getRace()));
        }

        for (size_t a(0); a < pre.size(); ++a)
        {
            const ActionType & actionType(a);
            
            if (pre.contains(actionType) && !allActions.contains(actionType))
            {
                allActions.add(actionType);
                CalculateRecursivePrerequisites(allActions, actionType);
            }
        }
    }

}
}