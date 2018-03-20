#include "Unit.h"

using namespace BOSS;

Unit::Unit(const ActionType & type, const size_t & id, int builderID)
    : m_job             (UnitJobs::None)
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

void Unit::startBuilding(const Unit & Unit)
{
    // if it's not a probe, this Unit won't be free until the build time is done
    if (!m_type.isWorker() || !m_type.getRace() == Races::Protoss)
    {
        m_timeUntilFree = Unit.getType().buildTime();
    }
    
    m_buildType = Unit.getType();
    m_buildID = Unit.getID();

    if (Unit.getType().isMorphed())
    {
        m_type = Unit.getType();
    }
}

void Unit::complete()
{
    m_timeUntilFree = 0;
    m_timeUntilBuilt = 0;
}

void Unit::fastForward(const int & frames)
{
    // if we are completing the thing that this Unit is building
    if ((m_buildType != ActionTypes::None) && frames >= m_timeUntilFree)
    {
        if (m_buildType.isAddon())
        {
            m_addon = m_buildType;
        }

        m_buildType = ActionTypes::None;
        m_buildID = 0;
        m_job = m_type.isWorker() ? UnitJobs::Minerals : UnitJobs::None;
    }

    m_timeUntilFree = std::max(0, m_timeUntilFree - frames);
    m_timeUntilBuilt = std::max(0, m_timeUntilBuilt - frames);
}

// returns when this Unit can build a given type, -1 if it can't
int Unit::whenCanBuild(const ActionType & type) const
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
    if (m_type.isWorker() && (m_job == UnitJobs::Gas))
    {
        return -1;
    }

    return m_timeUntilFree;
}

const int & Unit::getTimeUntilBuilt() const
{
    return m_timeUntilBuilt;
}

const int & Unit::getTimeUntilFree() const
{
    return m_timeUntilFree;
}

const ActionType & Unit::getType() const
{
    return m_type;
}

const ActionType & Unit::getAddon() const
{
    return m_addon;
}

const ActionType & Unit::getBuildType() const
{
    return m_buildType;
}

const size_t & Unit::getID() const
{
    return m_id;
}

void Unit::setBuilderID(const int & id)
{
    m_builderID = id;
}