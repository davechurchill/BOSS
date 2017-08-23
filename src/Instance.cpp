#include "Instance.h"

using namespace BOSS;

Instance::Instance(const ActionType & type, const size_t & id, int builderID)
    : m_job             (InstanceJobs::None)
    , m_id              (id)
    , m_type            (type)
    , m_addon           (ActionTypes::None)
    , m_buildType       (ActionTypes::None)
    , m_buildID         (0)
    , m_timeUntilBuilt  (builderID != -1 ? type.buildTime() : 0)
    , m_timeUntilFree   (builderID != -1 ? type.buildTime() : 0)
    , m_numLarva        (0)
    , m_builderID       (builderID)
{
    
}

void Instance::startBuilding(const Instance & instance)
{
    // if it's not a probe, this instance won't be free until the build time is done
    if (!m_type.isWorker() || !m_type.getRace() == Races::Protoss)
    {
        m_timeUntilFree = instance.getType().buildTime();
    }
    
    m_buildType = instance.getType();
    m_buildID = instance.getID();

    if (instance.getType().isMorphed())
    {
        m_type = instance.getType();
    }
}

void Instance::complete()
{
    m_timeUntilFree = 0;
    m_timeUntilBuilt = 0;
}

void Instance::fastForward(const int & frames)
{
    // if we are completing the thing that this instance is building
    if ((m_buildType != ActionTypes::None) && frames >= m_timeUntilFree)
    {
        if (m_buildType.isAddon())
        {
            m_addon = m_buildType;
        }

        m_buildType = ActionTypes::None;
        m_buildID = 0;
        m_job = m_type.isWorker() ? InstanceJobs::Minerals : InstanceJobs::None;
    }

    m_timeUntilFree = std::max(0, m_timeUntilFree - frames);
    m_timeUntilBuilt = std::max(0, m_timeUntilBuilt - frames);
}

// returns when this instance can build a given type, -1 if it can't
int Instance::whenCanBuild(const ActionType & type) const
{
    // check to see if this type can build the given type
    // TODO: check equivalent types (hatchery gspire etc)
    if (type.whatBuilds() != m_type) { return -1; }

    // if it requires an addon and we won't ever have one, we can't build it
    if (type.whatBuildsAddon() != ActionTypes::None && type.whatBuildsAddon() != m_addon && type.whatBuildsAddon() != m_buildType)
    {
        return -1;
    }

    // if this is a worker and it's harvesting gas, it can't build
    if (m_type.isWorker() && (m_job == InstanceJobs::Gas))
    {
        return -1;
    }

    return m_timeUntilFree;
}

const int & Instance::getTimeUntilBuilt() const
{
    return m_timeUntilBuilt;
}

const int & Instance::getTimeUntilFree() const
{
    return m_timeUntilFree;
}

const ActionType & Instance::getType() const
{
    return m_type;
}

const ActionType & Instance::getAddon() const
{
    return m_addon;
}

const ActionType & Instance::getBuildType() const
{
    return m_buildType;
}

const size_t & Instance::getID() const
{
    return m_id;
}

void Instance::setBuilderID(const int & id)
{
    m_builderID = id;
}