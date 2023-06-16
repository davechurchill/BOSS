#pragma once

#include "Common.h"
#include "ActionType.h"
#include "Unit.h"

namespace BOSS
{
class GameState
{
    std::vector<Unit>	m_units;
    std::vector<size_t>	m_unitsBeingBuilt;  // indices of m_units which are not completed, sorted descending by finish time
    int m_race              = Races::None;
    int m_minerals          = 0;
    int m_gas               = 0;
    int m_currentSupply     = 0;
    int m_maxSupply         = 0;
    int m_currentFrame      = 0;
    int m_previousFrame     = 0;
    int m_mineralWorkers    = 0;
    int m_gasWorkers        = 0;
    int m_buildingWorkers   = 0;
    int m_numRefineries     = 0;
    int m_numDepots         = 0;
    ActionType m_previousAction    = ActionTypes::None;
    
    
    Unit &  getUnit(const size_t & id);
    int     getBuilderID(const ActionType type)             const;
    bool    haveBuilder(const ActionType type)              const;
    bool    havePrerequisites(const ActionType type)        const;
    int     whenSupplyReady(const ActionType action)        const;
    int     whenPrerequisitesReady(const ActionType action) const;
    int     whenResourcesReady(const ActionType action)     const;
    int     whenBuilderReady(const ActionType action)       const;
    int     scaleResource(int baseResourceValue)            const;
    void    registerUnit(Unit & Unit);

public:

    GameState();
    
    const Unit &  getUnit(const size_t id)              const;
    int     getMinerals()                               const;
    int     getGas()                                    const;
    int     getCurrentSupply()                          const;
    int     getMaxSupply()                              const;
    int     getCurrentFrame()                           const;
    bool    canBuildNow(const ActionType action)        const;
    int     whenCanBuild(const ActionType action)       const;
    int     getSupplyInProgress()                       const;
    int     getLastActionFinishTime()                   const;
    int     getNextFinishTime(const ActionType type)    const;

    size_t  getNumMineralWorkers()                      const;
    size_t  getNumGasWorkers()                          const;
    size_t  getNumInProgress(const ActionType action)   const;
    size_t  getNumCompleted(const ActionType action)    const;
    size_t  getNumTotal(const ActionType action)        const;
    bool    isLegal(const ActionType type)              const;
    bool    haveType(const ActionType action)           const;
    int     getRace()                                   const;
    void    getLegalActions(std::vector<ActionType> & legalActions) const;

    void    doAction(const ActionType type);
    void    fastForward(const int frames);
    void    addUnit(const ActionType unit, int builderID = -1);
    void    setMinerals(const int minerals);
    void    setGas(const int gas);

    bool    operator == (const GameState & rhs)         const; // compares everything except previous frame

    std::string toString() const;

    std::string toStringAllUnits() const;
    std::string toStringResources() const;
    std::string toStringInProgress() const;
    std::string toStringCompleted() const;
    std::string toStringLegalActions() const;

    const std::vector<Unit>& getUnits() const;
};
}
