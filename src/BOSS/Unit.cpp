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

void Unit::startMorphing(const ActionType & type)
{
    // we won't be free until the build time is finished
    m_timeUntilFree = type.buildTime();

    // technically this unit is now being built as well
    m_timeUntilBuilt = type.buildTime();

    // record the type that we're building
    m_buildType = type;

    // record the id of the unit we're building
    m_buildID = m_id;

    // if we morph the unit, then we change into it
    m_type = type;

    // we are building ourself
    m_builderID = m_id;

    static const ActionType hatchery("Hatchery");
    if (type == hatchery && m_builderID != -1)
    {
        m_timeUntilLarva = 1;
    }
}

void Unit::complete()
{
    m_timeUntilFree = 0;
    m_timeUntilBuilt = 0;
}

void Unit::fastForward(const int frames)
{
    m_larvaToAdd.clear();

    // if we are completing the thing that this Unit is building
    if ((m_buildType != ActionTypes::None) && frames >= m_timeUntilFree)
    {
        // if we completed an addon, mark our addon as that type
        if (m_buildType.isAddon()) { m_addon = m_buildType; }

        // we are no longer building anything
        m_buildType = ActionTypes::None;
        m_buildID = 0;

        // go back to minerals if this is a worker
        if (m_job != UnitJobs::Gas)
        {
            m_job = m_type.isWorker() ? UnitJobs::Minerals : UnitJobs::None;
        }
    }

    // compute the number of larva this building should have
    BOSS_ASSERT(m_numLarva < 3 || m_timeUntilLarva == 0, "Larva Error");
    static const ActionType hatchery("Hatchery");

    // we can only fast forward larva counters if we have a valid one
    bool ffLarva = m_timeUntilLarva > 0;

    // but don't fast forward larva counter of unbuilt hatcheries
    if (m_type == hatchery && m_timeUntilBuilt > frames) { ffLarva = false; }

    if (ffLarva)
    {
        BOSS_ASSERT(m_numLarva < 3, "Shouldn't have less than 3 larva and a non-zero timer");
        
        int ff = frames;
        if (m_type == hatchery && m_timeUntilBuilt <= frames)
        {
            ff -= m_timeUntilBuilt;
        }

        while (m_numLarva < 3 && ff > 0)
        {
            if (ff < m_timeUntilLarva)
            {
                m_timeUntilLarva -= ff;
                break;
            }
            else
            {
                ff -= m_timeUntilLarva;
                m_larvaToAdd.push_back(ff);
                m_timeUntilLarva = 13 * 24;
            }

            // don't add too many larva
            if (m_numLarva + m_larvaToAdd.size() >= 3) { break; }
        }
    }

    // subtract the amount of frames fast forwarded from our remaining times
    m_timeUntilFree = std::max(0, m_timeUntilFree - frames);
    m_timeUntilBuilt = std::max(0, m_timeUntilBuilt - frames);
}

// returns when this Unit can build a given type, -1 if it can't
int Unit::whenCanBuild(const ActionType & type) const
{
    // if morphing, the builder must be equal to, not simply equivalent to, the needed type
    if (type.whatBuilds() != m_type && type.isMorphed()) { return -1; }

    // check to see if this type is an equivalent that can build the given type
    if (!m_type.isEquivalentTo(type.whatBuilds())) { return -1; }

    // if we want to build an addon but we already have an addon then we can never build it
    if (type.isAddon() && (hasAddon() || getBuildType().isAddon())) { return -1; }

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

    // if this is reserved and the action requires morphing, cannot build
    if (m_reservedForID != -1 && type.isMorphed()) { return -1; }

    return m_timeUntilFree;
}

void Unit::useLarva()
{
    BOSS_ASSERT(m_numLarva > 0, "Can't use a larva when we have none");

    if (m_numLarva == 3)
    {
        m_timeUntilLarva = 13 * 24;
    }
        
    m_numLarva--;
}

int Unit::getTimeUntilBuilt() const
{
    return m_timeUntilBuilt;
}

int Unit::getTimeUntilFree() const
{
    return m_timeUntilFree;
}

ActionType Unit::getType() const
{
    return m_type;
}

ActionType Unit::getAddon() const
{
    return m_addon;
}

ActionType Unit::getBuildType() const
{
    return m_buildType;
}

size_t Unit::getID() const
{
    return m_id;
}

size_t Unit::getAddonID() const
{
    return m_addonID;
}

void Unit::setBuilderID(const int id)
{
    m_builderID = id;
}

bool Unit::hasAddon() const
{
    return m_addon != ActionTypes::None;
}

size_t Unit::getBuildID() const
{
    return m_buildID;
}

size_t Unit::getBuilderID() const
{
    return m_builderID;
}

int Unit::timeUntilLarva() const
{
    return m_timeUntilLarva;
}

int Unit::numLarva() const
{
    return m_numLarva;
}

int BOSS::Unit::reservedFor() const
{
    return m_reservedForID;
}

UnitJobs BOSS::Unit::getJob() const
{
    return m_job;
}

void Unit::addLarva()
{
    BOSS_ASSERT(m_numLarva < 3, "Can't have more than 3 larva");
    m_numLarva++;

    // make sure we reset the timer if we have 3 larva
    if (m_numLarva == 3) { m_timeUntilLarva = 0; }
}

void BOSS::Unit::reserve(const int refineryID)
{
    m_reservedForID = refineryID;
}

void BOSS::Unit::setJob(const UnitJobs job)
{
    m_job = job;
}

std::vector<int>& Unit::larvaToAdd() 
{
    return m_larvaToAdd;
}

