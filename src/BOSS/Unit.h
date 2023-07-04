#pragma once

#include "Common.h"
#include "ActionType.h"

namespace BOSS
{

namespace UnitJobs
{
    enum { None, Minerals, Gas, Build };
}

class Unit
{
    size_t      m_id                = 0;                    // index in GameState::m_Units
    size_t      m_builderID         = 0;                    // id of the unit that built this Unit
    ActionType  m_type              = ActionTypes::None;    // type of this Unit
    ActionType  m_addon             = ActionTypes::None;    // type of completed addon this Unit has
    size_t      m_addonID           = 0;                    // id of the addon unit for this unit
    ActionType  m_buildType         = ActionTypes::None;    // type of the Unit currently being built by this Unit
    size_t      m_buildID           = 0;                    // id of the Unit currently being built by this Unit
    int         m_job               = UnitJobs::None;       // current job this Unit has (UnitJobs::XXX)
    int         m_timeUntilBuilt    = 0;                    // time remaining until this Unit is completed
    int         m_timeUntilFree     = 0;                    // time remaining until this Unit can build again
    int         m_numLarva          = 0;                    // number of larva this building currently has (Hatch only)
    int         m_timeUntilLarva    = 0;
    std::vector<int> m_larvaToAdd;

public:

    Unit(const ActionType type, const size_t id, int builderID);

    int getTimeUntilFree() const;
    int getTimeUntilBuilt() const;
    ActionType getType() const;
    ActionType getAddon() const;
    ActionType getBuildType() const;
    size_t getID() const;
    size_t getBuildID() const;
    size_t getBuilderID() const;
    size_t getAddonID() const;
    bool hasAddon() const;
    int timeUntilLarva() const;
    int numLarva() const;
    std::vector<int>& larvaToAdd();


    int whenCanBuild(const ActionType & type) const;

    void complete();
    void setBuilderID(const int id);
    void startBuilding(const Unit & Unit);
    void startMorphing(const ActionType& type);
    void fastForward(const int frames);
    void useLarva();
    void addLarva();

    bool operator == (const Unit& rhs) const = default;
};

}