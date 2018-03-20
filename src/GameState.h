#pragma once

#include "Common.h"
#include "ActionType.h"
#include "Unit.h"

namespace BOSS
{

class GameState 
{
    std::vector<Unit>	m_units;
    std::vector<size_t>	m_unitsBeingBuilt;
    int					m_race;
    double				m_minerals;
    double				m_gas;
    int					m_currentSupply;
    int					m_maxSupply;
    int					m_currentFrame;
    int					m_previousFrame;
    int					m_mineralWorkers;
    int					m_gasWorkers;
    int					m_buildingWorkers;
    int					m_numRefineries;
    int					m_numDepots;
    ActionType			m_previousAction;

    int			getBuilderID(const ActionType & type) const;
    bool		haveBuilder(const ActionType & type) const;
    bool		havePrerequisites(const ActionType & type) const;

    int			whenSupplyReady(const ActionType & action)          const;
    int			whenPrerequisitesReady(const ActionType & action)   const;
    int			whenResourcesReady(const ActionType & action)       const;
    int			whenBuilderReady(const ActionType & action)         const;
    int			getSupplyInProgress() const;

    Unit &		getUnit(const size_t & id);
    void		completeUnit(Unit & Unit);

public: 

    GameState();

    const double &	getMinerals() const;
    const double &	getGas() const;
    const int &		getCurrentSupply() const;
    const int &		getMaxSupply() const;
    const int &		getCurrentFrame() const;
    const Unit &	getUnit(const size_t & id) const;
	bool			canBuildNow(const ActionType & action) const;
    int				whenCanBuild(const ActionType & action) const;
    
    size_t			getNumInProgress(const ActionType & action) const;
    size_t			getNumCompleted(const ActionType & action) const;
    size_t			getNumTotal(const ActionType & action) const;
    void			getLegalActions(std::vector<ActionType> & legalActions) const;
    bool			isLegal(const ActionType & type) const;
    bool			haveType(const ActionType & action) const;
    int				getRace() const;

    void			doAction(const ActionType & type);
    void			fastForward(const int & frames);
    void			addUnit(const ActionType & Unit, int builderID = -1);
    void			setMinerals(const double & minerals);
    void			setGas(const double & gas);

    std::string		toString() const;
};
}
