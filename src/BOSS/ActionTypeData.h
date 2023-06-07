#pragma once

#include "Common.h"
#include "ActionType.h"

namespace BOSS
{
    
struct ActionTypeData
{
    std::string                 name            = "None";
    std::string                 raceName        = "None";
    size_t                      id              = 0;
    int                         race            = Races::None;            
    int                         mineralCost     = 0;      
    int                         gasCost         = 0;       
    int                         supplyCost      = 0;   
    int                         energyCost      = 0;
    int                         supplyProvided  = 0;   
    int                         buildTime       = 0;
    int                         numProduced     = 1;
    int                         startingEnergy  = 0;
    int                         maxEnergy       = 0;
    int                         buildLimit = -1;
    bool                        isUnit          = false;
    bool                        isUpgrade       = false;
    bool                        isAbility       = false;
    bool                        isBuilding      = false;
    bool                        isWorker        = false;
    bool                        isRefinery      = false;
    bool                        isSupplyProvider= false;
    bool                        isDepot         = false;
    bool                        isAddon         = false;
    bool                        isHatchery      = false;
    bool                        isMorphed       = false;
    bool                        isTech          = false;
    ActionType                  whatBuilds      = 0;
    size_t                      whatBuildsCount = 1;
    ActionType                  whatBuildsAddon = 0;
    std::string                 whatBuildsStr;
    std::string                 whatBuildsStatus;
    std::string                 whatBuildsAddonStr;
    std::vector<std::string>    equivalentStrings;
    std::vector<std::string>    requiredStrings;
    std::vector<ActionType>     equivalent;
    std::vector<ActionType>     required;

    static void Init(const std::string & filename);
    static const ActionTypeData & GetActionTypeData(const ActionID action);
    static const ActionTypeData & GetActionTypeData(const std::string & name);
    static const std::vector<ActionTypeData> & GetAllActionTypeData();
};

}
