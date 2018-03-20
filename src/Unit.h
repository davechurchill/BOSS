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
    size_t      m_id;               // index in GameState::m_Units
    size_t      m_builderID;        // id of the unit that built this Unit
    ActionType  m_type;             // type of this Unit
    ActionType  m_addon;            // type of completed addon this Unit has
    ActionType  m_buildType;        // type of the Unit currently being built by this Unit
    size_t      m_buildID;          // id of the Unit currently being built by this Unit
    int         m_job;              // current job this Unit has (UnitJobs::XXX)
    int         m_timeUntilBuilt;   // time remaining until this Unit is completed
    int         m_timeUntilFree;    // time remaining until this Unit can build again
    int         m_numLarva;         // number of larva this building currently has (Hatch only)

public:

    Unit(const ActionType & type, const size_t & id, int builderID);

    const int & getTimeUntilFree() const;
    const int & getTimeUntilBuilt() const;
    const ActionType & getType() const;
    const ActionType & getAddon() const;
    const ActionType & getBuildType() const;
    const size_t & getID() const;

    int whenCanBuild(const ActionType & type) const;

    void complete();
    void setBuilderID(const int & id);
    void startBuilding(const Unit & Unit);
    void fastForward(const int & frames);
};

}