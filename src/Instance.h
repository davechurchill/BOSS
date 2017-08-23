#pragma once

#include "Common.h"
#include "ActionType.h"

namespace BOSS
{

namespace InstanceJobs
{
    enum { None, Minerals, Gas, Build };
}

class Instance
{
    size_t      m_id;               // index in GameState::m_instances
    size_t      m_builderID;        // id of the unit that built this instance
    ActionType  m_type;             // type of this instance
    ActionType  m_addon;            // type of completed addon this instance has
    ActionType  m_buildType;        // type of the instance currently being built by this instance
    size_t      m_buildID;          // id of the instance currently being built by this instance
    int         m_job;              // current job this instance has (InstanceJobs::XXX)
    int         m_timeUntilBuilt;   // time remaining until this instance is completed
    int         m_timeUntilFree;    // time remaining until this instance can build again
    int         m_numLarva;         // number of larva this building currently has (Hatch only)

public:

    Instance(const ActionType & type, const size_t & id, int builderID);

    const int & getTimeUntilFree() const;
    const int & getTimeUntilBuilt() const;
    const ActionType & getType() const;
    const ActionType & getAddon() const;
    const ActionType & getBuildType() const;
    const size_t & getID() const;

    int whenCanBuild(const ActionType & type) const;

    void complete();
    void setBuilderID(const int & id);
    void startBuilding(const Instance & instance);
    void fastForward(const int & frames);
};

}