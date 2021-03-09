#include "Unit.h"

using namespace BOSS;

Unit::Unit(const ActionType type, const size_t id, int builderID)
    : m_id              (id)
    , m_type            (type)
    , m_timeUntilBuilt  (builderID != -1 ? type.buildTime() : 0)
    , m_timeUntilFree   (builderID != -1 ? type.buildTime() : 0)
    , m_builderID       (builderID)
{
    
}

void Unit::startBuilding(const Unit & unit)
{
    // if it's a protoss probe, nothing really happens here
    if (m_type.isWorker() && m_type.getRace() == Races::Protoss) { return; }

    // we won't be free until the build time is finished
    m_timeUntilFree = unit.getType().buildTime();
    
    // record the type that we're building
    m_buildType = unit.getType();

    // record the id of the unit we're building
    m_buildID = unit.getID();

    // if we morph the unit, then we change into it
    if (unit.getType().isMorphed())
    {
        m_type = unit.getType();
    }
}

void Unit::complete()
{
    //m_timeUntilFree = 0;
    //m_timeUntilBuilt = 0;
}

void Unit::fastForward(const int frames)
{
    // if we are completing the thing that this Unit is building
    if ((m_buildType != ActionTypes::None) && frames >= m_timeUntilFree)
    {
        // if we completed an addon, mark our addon as that type
        if (m_buildType.isAddon()) { m_addon = m_buildType; }

        // we are no longer building anything
        m_buildType = ActionTypes::None;
        m_buildID = 0;

        // go back to minerals if this is a worker
        m_job = m_type.isWorker() ? UnitJobs::Minerals : UnitJobs::None;
    }

    // subtract the amount of frames fast forwarded from our remaining times
    m_timeUntilFree = std::max(0, m_timeUntilFree - frames);
    m_timeUntilBuilt = std::max(0, m_timeUntilBuilt - frames);
}

// returns when this Unit can build a given type, -1 if it can't
int Unit::whenCanBuild(const ActionType & type) const
{
    if (m_type.getName() == "Factory" && type.getName() == "SiegeTankTankMode")
    {
        int a = 7;
    }

    // check to see if this type can build the given type
    // TODO: check equivalent types (hatchery gspire etc)
    if (type.whatBuilds() != m_type) { return -1; }

    // if we want to build an addon but we already have an addon then we can never build it
    if (type.isAddon() && hasAddon()) { return -1; }

    // if it requires an addon that we can't get, we can't build it
    if (type.whatBuildsAddon() != ActionTypes::None &&  // if it requires an addon
        type.whatBuildsAddon() != m_addon &&            // and we have an addon that isn't the addon
        type.whatBuildsAddon() != m_buildType)          // and we're not building the required addon
    {
        ActionType t = type.whatBuildsAddon();
        return -1;
    }

    // if this is a worker and it's harvesting gas, it can't build
    if (m_type.isWorker() && (m_job == UnitJobs::Gas)) { return -1; }

    return m_timeUntilFree;
}

const int Unit::getTimeUntilBuilt() const
{
    return m_timeUntilBuilt;
}

const int Unit::getTimeUntilFree() const
{
    return m_timeUntilFree;
}

const ActionType Unit::getType() const
{
    return m_type;
}

const ActionType Unit::getAddon() const
{
    return m_addon;
}

const ActionType Unit::getBuildType() const
{
    return m_buildType;
}

const size_t Unit::getID() const
{
    return m_id;
}

const size_t Unit::getAddonID() const
{
    return m_addonID;
}

void Unit::setBuilderID(const int id)
{
    m_builderID = id;
}

const bool Unit::hasAddon() const
{
    return m_addon != ActionTypes::None;
}

const size_t Unit::getBuildID() const
{
    return m_buildID;
}

const size_t Unit::getBuilderID() const
{
    return m_builderID;
}