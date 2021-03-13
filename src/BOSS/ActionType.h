#pragma once

#include "Common.h"
#include "ActionSet.h"

namespace BOSS
{

class ActionType
{
    const ActionID m_id = 0;

public:

    ActionType();
    ActionType(const ActionID actionID);
    ActionType(const std::string & str);

    ActionType & operator = (const ActionType & rhs);
    const std::string & getName()       const;

    ActionID getID()        const;
    RaceID   getRace()      const;

    int  buildTime()        const;
    int  mineralPrice()     const;
    int  gasPrice()         const;
    int  supplyCost()       const;
    int  supplyProvided()   const;
    int  numProduced()      const;
    bool isUnit()           const;
    bool isUpgrade()        const;
    bool isAbility()        const;
    bool isBuilding()       const;
    bool isWorker()         const;
    bool isRefinery()       const;
    bool isDepot()          const;
    bool isSupplyProvider() const;
    bool isAddon()          const;
    bool isMorphed()        const;
    bool isHatchery()       const;

    ActionType whatBuilds() const;
    ActionType whatBuildsAddon() const;
    const std::vector<ActionType> & required() const;
    const std::vector<ActionType> & equivalent() const;
    const ActionSet & getPrerequisiteActionCount() const;
    const ActionSet & getRecursivePrerequisiteActionCount() const;
    bool operator == (const ActionType rhs)     const;
    bool operator != (const ActionType rhs)     const;
    bool operator <  (const ActionType rhs)     const;
};

namespace ActionTypes
{
    void Init();
    const std::vector<ActionType> & GetAllActionTypes();
    ActionType GetWorker(const RaceID raceID);
    ActionType GetSupplyProvider(const RaceID raceID);
    ActionType GetRefinery(const RaceID raceID);
    ActionType GetResourceDepot(const RaceID raceID);
    ActionType GetActionType(const std::string & name);
    bool       TypeExists(const std::string & name);

    ActionSet CalculatePrerequisites(const ActionType & action);
    void CalculateRecursivePrerequisites(ActionSet & count, const ActionType & action);

    extern ActionType None;
    }
}