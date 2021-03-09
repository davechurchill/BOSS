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

public:

    Unit(const ActionType type, const size_t id, int builderID);

    const int getTimeUntilFree() const;
    const int getTimeUntilBuilt() const;
    const ActionType getType() const;
    const ActionType getAddon() const;
    const ActionType getBuildType() const;
    const size_t getID() const;
    const size_t getBuildID() const;
    const size_t getBuilderID() const;
    const size_t getAddonID() const;
    const bool hasAddon() const;

    int whenCanBuild(const ActionType & type) const;

    void complete();
    void setBuilderID(const int id);
    void startBuilding(const Unit & Unit);
    void fastForward(const int frames);
};

}