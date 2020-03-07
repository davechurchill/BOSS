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
    int					m_race              = Races::None;
    double				m_minerals          = 0;
    double				m_gas               = 0;
    int					m_currentSupply     = 0;
    int					m_maxSupply         = 0;
    int					m_currentFrame      = 0;
    int					m_previousFrame     = 0;
    int					m_mineralWorkers    = 0;
    int					m_gasWorkers        = 0;
    int					m_buildingWorkers   = 0;
    int					m_numRefineries     = 0;
    int					m_numDepots         = 0;
    ActionType			m_previousAction    = ActionTypes::None;
    
    Unit &  getUnit(const size_t & id);
    int     getBuilderID(const ActionType type)             const;
    bool    haveBuilder(const ActionType type)              const;
    bool    havePrerequisites(const ActionType type)        const;
    int     whenSupplyReady(const ActionType action)        const;
    int     whenPrerequisitesReady(const ActionType action) const;
    int     whenResourcesReady(const ActionType action)     const;
    int     whenBuilderReady(const ActionType action)       const;
    void    completeUnit(Unit & Unit);

public:

    GameState();
    
    const Unit &  getUnit(const size_t id)              const;
    double  getMinerals()                               const;
    double  getGas()                                    const;
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
    void    setMinerals(const double minerals);
    void    setGas(const double gas);

    std::string     toString() const;
};
}
