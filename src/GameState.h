#pragma once

#include "Common.h"
#include "ActionType.h"
#include "Instance.h"

namespace BOSS
{

class GameState 
{
    std::vector<Instance>           m_instances;
    std::vector<size_t>             m_instancesBeingBuilt;
    int                             m_race;
    float                           m_minerals;
    float                           m_gas;
    int                             m_currentSupply;
    int                             m_maxSupply;
    int                             m_currentFrame;
    int                             m_previousFrame;
    int                             m_mineralWorkers;
    int                             m_gasWorkers;
    int                             m_buildingWorkers;
    int                             m_numRefineries;
    int                             m_numDepots;
    ActionType                      m_previousAction;

    int getBuilderID(const ActionType & type) const;
    bool haveBuilder(const ActionType & type) const;
    bool havePrerequisites(const ActionType & type) const;

    int whenSupplyReady(const ActionType & action)          const;
    int whenPrerequisitesReady(const ActionType & action)   const;
    int whenResourcesReady(const ActionType & action)       const;
    int whenBuilderReady(const ActionType & action)         const;
    int getSupplyInProgress() const;

    Instance & getInstance(const size_t & id);
    void completeInstance(Instance & instance);

public: 

    GameState();

    const float & getMinerals() const;
    const float & getGas() const;
    const int & getCurrentSupply() const;
    const int & getMaxSupply() const;
    const int & getCurrentFrame() const;
    const Instance & getInstance(const size_t & id) const;
    int whenCanBuild(const ActionType & action) const;
    
    size_t getNumInProgress(const ActionType & action) const;
    size_t getNumCompleted(const ActionType & action) const;
    size_t getNumTotal(const ActionType & action) const;
    void getLegalActions(std::vector<ActionType> & legalActions) const;
    bool isLegal(const ActionType & type) const;
    bool haveType(const ActionType & action) const;
    int getRace() const;

    void doAction(const ActionType & type);
    void fastForward(const int & frames);
    void addInstance(const ActionType & instance, int builderID = -1);
    void setMinerals(const float & minerals);
    void setGas(const float & gas);

    std::string toString() const;
};
}
