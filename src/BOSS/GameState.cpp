#include "GameState.h"
#include <numeric>
#include <iomanip>

using namespace BOSS;

const int ResourceScale = 1000;
const int MPWPF = 45;
const int GPWPF = 70;
const int WorkersPerRefinery = 3;

GameState::GameState()
    : m_minerals        (0)
    , m_gas             (0)
    , m_race            (Races::None)
    , m_currentSupply   (0)
    , m_maxSupply       (0)
    , m_currentFrame    (0)
    , m_previousFrame   (0)
    , m_mineralWorkers  (0)
    , m_gasWorkers      (0)
    , m_buildingWorkers (0)
    , m_numRefineries   (0)
    , m_numDepots       (0)
    , m_previousAction  (ActionTypes::None)
{

}

void GameState::getLegalActions(std::vector<ActionType> & legalActions) const
{
    legalActions.clear();
    for (auto & type : ActionTypes::GetAllActionTypes())
    {
        if (isLegal(type)) { legalActions.push_back(type); }
    }
}

bool GameState::isLegal(const ActionType action) const
{
    if (action.getRace() != m_race) { return false; }

    static const ActionType larva("Larva");
    if (action == larva) { return false; }

    const size_t refineriesInProgress = getNumInProgress(ActionTypes::GetRefinery(m_race));
    const size_t mineralWorkers = m_mineralWorkers + m_buildingWorkers + getNumInProgress(ActionTypes::GetWorker(m_race));
    const size_t numRefineries  = m_numRefineries + refineriesInProgress;
    const size_t numDepots      = m_numDepots + getNumInProgress(ActionTypes::GetResourceDepot(m_race));

    // check that the build limit is not surpassed
    if (action.buildLimit() && action.buildLimit() <= getNumTotal(action)) { return false; }

    // check to see if we will ever have enough resources to build this thing
    if (whenResourcesReady(action) == -1) { return false; }
        	
    // if we won't have enough supply eventually to build this item, it's not legal
    // assumes there will never be a situation where one unit builds another, and both have supply costs > 0, unless it is a morph
    if ((m_currentSupply + action.supplyCost() - action.whatBuilds().supplyCost()) > (m_maxSupply + getSupplyInProgress())) { return false; }

    // rules for buildings which are built by workers
    if (action.isBuilding() && !action.isMorphed() && !action.isAddon() && (mineralWorkers == 0)) { return false; }

    // if we have no mineral income we'll never have a mineral unit
    if ((m_minerals < action.mineralPrice()) && (mineralWorkers == 0)) { return false; }

    // don't build more refineries than resource depots
    if (action.isRefinery() && (numRefineries >= numDepots)) { return false; }

    // check that there are sufficient workers for a refinery to be made, such that three can be assigned to the refinery when it finishes
    if (action.isRefinery() && findReservableWorkers(action.buildTime(), action.isMorphed()).size() != WorkersPerRefinery) { return false; }

    // we don't need to go over the maximum supply limit with supply providers
    if (action.isSupplyProvider() && (m_maxSupply + getSupplyInProgress() > 400)) { return false; }

    // if we don't have a builder for the type, we can't build it
    if (!haveBuilder(action) && (action.whatBuilds() != larva)) { return false; }

    // if we don't have the prerequisites, we can't build it either
    if (!havePrerequisites(action)) { return false; }

    return true;
}

void GameState::doAction(const ActionType type)
{
    BOSS_ASSERT(isLegal(type), "Trying to do illegal action: %s", type.getName().c_str());

    int previousFrame = m_currentFrame;

    // figure out when this action can be done and fast forward to it
    const int timeWhenReady = whenCanBuild(type);
    fastForward(timeWhenReady);

    // subtract the resource cost
    m_minerals      -= scaleResource(type.mineralPrice());
    m_gas           -= scaleResource(type.gasPrice());
    m_currentSupply += type.supplyCost();

    // Adjusts the supply to match the unit being morphed, rather than the combination of that unit and its builder's supply
    if (type.isMorphed())
    {
        m_currentSupply -= type.whatBuilds().supplyCost();
    }

    // if it's a Terran building that's not an addon, the worker removed from minerals
    if (type.getRace() == Races::Terran && type.isBuilding() && !type.isAddon())
    {
        m_mineralWorkers--;
        m_buildingWorkers++;
    }

    // if it's a Zerg unit that requires a Drone, remove a mineral worker
    if (type.getRace() == Races::Zerg && type.whatBuilds().isWorker())
    {
        m_mineralWorkers--;     // the worker is no longer on minerals
    }

    // get a builder for this type and start building it
    addUnit(type, getBuilderID(type));
}

void GameState::fastForward(const int toFrame)
{
    if (toFrame == m_currentFrame) { return; }

    BOSS_ASSERT(toFrame > m_currentFrame, "Must ff to the future");

    m_previousFrame             = m_currentFrame;
    int previousFrame           = m_currentFrame;
    int lastActionFinishTime    = m_currentFrame;
    
    // iterate backward over actions in progress since they're sorted
    // that way for ease of deleting the finished ones
    while (!m_unitsBeingBuilt.empty())
    {
        Unit & unit = getUnit(m_unitsBeingBuilt.back());
        ActionType type = unit.getType();

        // if the current action in progress will finish after the ff time, we can stop
        const int actionCompletionTime = previousFrame + unit.getTimeUntilBuilt();
        if (actionCompletionTime > toFrame) { break; }

        // add the resources we gathered during this time period
        const int timeElapsed	= actionCompletionTime - lastActionFinishTime;
        m_minerals              += timeElapsed * MPWPF * m_mineralWorkers;
        m_gas                   += timeElapsed * GPWPF * m_gasWorkers;
        lastActionFinishTime    = actionCompletionTime;
        
        // if it's a Terran building that's not an addon, the worker returns to minerals
        if (type.getRace() == Races::Terran && type.isBuilding() && !type.isAddon())
        {
            m_mineralWorkers++;
            m_buildingWorkers--;
        }

        // register the action and remove it from the list
        completeUnit(unit);
        m_unitsBeingBuilt.pop_back();
    }

    // update resources from the last action finished to the ff frame
    int timeElapsed = toFrame - lastActionFinishTime;
    m_minerals      += timeElapsed * MPWPF * m_mineralWorkers;
    m_gas           += timeElapsed * GPWPF * m_gasWorkers;

    // update all the intances to the ff time
    std::map<int, std::vector<size_t>> larvae;
    for (size_t i = 0; i < m_units.size(); i++)
    {
        m_units[i].fastForward(toFrame - previousFrame);
        for (auto l : m_units[i].larvaToAdd())
        {
            larvae[l].push_back(i);
        }
        m_units[i].larvaToAdd().clear();
    }

    // Add larva in order
    static const ActionType larva("Larva");
    for (auto iter = larvae.rbegin(); iter != larvae.rend(); ++iter)
    {
        for (int i : iter->second)
        {
            addUnit(larva, m_units[i].getID());
            if (iter->first + previousFrame != toFrame)
            {
                auto& newLarva = m_units.back();
                newLarva.complete();
            }
        }
    }

    m_currentFrame = toFrame;
}

void GameState::completeUnit(Unit & unit)
{
    m_maxSupply += unit.getType().supplyProvided();

    // if it's a worker, assign it to the correct job
    if (unit.getType().isWorker())
    {
        m_mineralWorkers++;
        int needGasWorkers = std::max(0, (WorkersPerRefinery*m_numRefineries - m_gasWorkers));
        BOSS_ASSERT(needGasWorkers < m_mineralWorkers, "Shouldn't need more gas workers than we have mineral workers");
        m_mineralWorkers -= needGasWorkers;
        m_gasWorkers += needGasWorkers;
    }
    else if (unit.getType().isRefinery())
    {
        int count = 0;
        for (auto& u : m_units)
        {
            if (u.getType().isWorker() && u.reservedFor() == unit.getID())
            {
                u.setJob(UnitJobs::Gas);
                u.reserve(-1);
                count++;
            }
        }
        m_numRefineries++;
        m_gasWorkers += WorkersPerRefinery;
        m_mineralWorkers -= WorkersPerRefinery;
        BOSS_ASSERT(count == WorkersPerRefinery, "Incorrect number of reserved workers found");
    }
    else if (unit.getType().isDepot())
    {
        m_numDepots++;
    }

}

std::vector<int> BOSS::GameState::findReservableWorkers(const int buildTime, const bool morphed) const
{
    std::vector<int> workers;
    bool foundExtra = !morphed;
    for (auto& unit : m_units)
    {
        if (unit.getType().isWorker() && unit.getJob() != UnitJobs::Gas && unit.reservedFor() == -1)
        {
            if (unit.getTimeUntilFree() > buildTime) { continue; }
            if (workers.size() == WorkersPerRefinery)
            {
                foundExtra = true;
                break;
            }
            workers.push_back(unit.getID());
        }
    }
    if (!foundExtra) { workers.clear(); }
    return workers;
}

// add a unit of the given type to the state
// if builderID is -1 (default) the unit is added as completed, otherwise it begins construction with the builder
void GameState::addUnit(const ActionType type, int builderID)
{
    BOSS_ASSERT(m_race == Races::None || type.getRace() == m_race, "Adding an Unit of a different race");

    m_race = type.getRace();

    int unitBeingBuiltID = 0;

    static const ActionType larva("Larva");
    if (type == larva)
    {
        Unit unit(type, m_units.size(), builderID);
        m_units.push_back(unit);
        unitBeingBuiltID = unit.getID();
        getUnit(builderID).addLarva();
    }
    // if there's no builder, complete the unit now and skip the unit in progress step
    else if (builderID == -1)
    {
        Unit unit(type, m_units.size(), builderID); // unit is completed in constructor
        m_units.push_back(unit);
        unitBeingBuiltID = unit.getID();
        completeUnit(unit);

        m_currentSupply += type.supplyCost();

        // if it's a hatchery, add 3 larva
        static const ActionType hatchery("Hatchery");
        static const ActionType larva("Larva");
        if (unit.getType() == hatchery)
        {
            addUnit(larva, unit.getID());
            addUnit(larva, unit.getID());
            addUnit(larva, unit.getID());
        }
    }
    // if we have a valid builder for this object, add it to the Units being built
    else 
    {

        // if the type is morphed we don't need a new unit 
        if (type.isMorphed())
        {
            Unit& builder = getUnit(builderID);

            // if the builder is a larva, we need to subtract from its hatchery's count
            if (builder.getType() == larva)
            {
                getUnit(builder.getBuilderID()).useLarva();;
            }

            builder.startMorphing(type);
            unitBeingBuiltID = builder.getID();
        }
        // if it's non-morphed, then we need a new unit
        else
        {
            Unit unit(type, m_units.size(), builderID);
            m_units.push_back(unit);
            getUnit(builderID).startBuilding(m_units.back());
            unitBeingBuiltID = unit.getID();
        }

        // add the unit ID being built and sort the list
        m_unitsBeingBuilt.push_back(unitBeingBuiltID);

        // we know the list is already sorted when we add this unit, so we just swap it from the end until it's in the right place
        for (size_t i = m_unitsBeingBuilt.size() - 1; i > 0; i--)
        {
            if (getUnit(m_unitsBeingBuilt[i]).getTimeUntilBuilt() > getUnit(m_unitsBeingBuilt[i - 1]).getTimeUntilBuilt())
            {
                std::swap(m_unitsBeingBuilt[i], m_unitsBeingBuilt[i - 1]);
            }
            else { break; }
        }
    }
    
    // Reserve workers for refinery
    if (type.isRefinery())
    {
        auto workers = findReservableWorkers(type.buildTime(), false);
        BOSS_ASSERT(workers.size() == 3, "Incorrect number of workers to reserve");
        for (int i : workers)
        {
            m_units[i].reserve(unitBeingBuiltID);
        }
    }
}

int GameState::whenCanBuild(const ActionType action) const
{
    // figure out when prerequisites will be ready
    int maxTime         = m_currentFrame;
    int prereqTime      = whenPrerequisitesReady(action);
    int resourceTime    = whenResourcesReady(action);
    int supplyTime      = whenSupplyReady(action);
    int builderTime     = whenBuilderReady(action);

    BOSS_ASSERT(resourceTime != -1, "Resources will never be ready");

    // figure out the max of all these times
    maxTime = std::max(resourceTime,    maxTime);
    maxTime = std::max(prereqTime,      maxTime);
    maxTime = std::max(supplyTime,      maxTime);
    maxTime = std::max(builderTime,     maxTime);

    // return the time
    return maxTime;
}

// returns the game frame that we will have the resources available to construction given action type
// this function assumes the action is legal (must be checked beforehand)
int GameState::whenResourcesReady(const ActionType action) const
{
    const int mineralPrice = scaleResource(action.mineralPrice());
    const int gasPrice = scaleResource(action.gasPrice());
    if (m_minerals >= mineralPrice && m_gas >= gasPrice)
    {
        return getCurrentFrame();
    }

    int previousFrame           = m_currentFrame;
    int currentMineralWorkers   = m_mineralWorkers;
    int currentGasWorkers       = m_gasWorkers;
    int lastActionFinishFrame   = m_currentFrame;
    int addedTime               = 0;
    int addedMinerals           = 0;
    int addedGas                = 0;
    int mineralDifference       = mineralPrice - m_minerals;
    int gasDifference           = gasPrice - m_gas;

    // loop through each action in progress, adding the minerals we would gather from each interval
    for (size_t i(0); i < m_unitsBeingBuilt.size(); ++i)
    {
        const Unit & unit = getUnit(m_unitsBeingBuilt[m_unitsBeingBuilt.size() - 1 - i]);
        int actionCompletionTime = previousFrame + unit.getTimeUntilBuilt();

        // the time elapsed and the current minerals per frame
        int elapsed = actionCompletionTime - lastActionFinishFrame;

        // the amount of minerals that would be added this time step
        int tempAddMinerals = elapsed * currentMineralWorkers * MPWPF;
        int tempAddGas      = elapsed * currentGasWorkers * GPWPF;

        // if this amount isn't enough, update the amount added for this interval
        if (addedMinerals + tempAddMinerals < mineralDifference || addedGas + tempAddGas < gasDifference)
        {
            addedMinerals += tempAddMinerals;
            addedGas += tempAddGas;
            addedTime += elapsed;
        }
        else { break; }

        // finishing a building as terran gives you a mineral worker back
        if (unit.getType().isBuilding() && !unit.getType().isAddon() && (unit.getType().getRace() == Races::Terran))
        {
            currentMineralWorkers++;
        }
        // finishing a worker gives us another mineral worker
        if (unit.getType().isWorker())
        {
            currentMineralWorkers++;
        }
        // finishing a refinery adjusts the worker count
        else if (unit.getType().isRefinery())
        {
            currentMineralWorkers -= WorkersPerRefinery; 
            currentGasWorkers += WorkersPerRefinery;
        }

        // update the last action
        lastActionFinishFrame = actionCompletionTime;
    }

    BOSS_ASSERT(currentMineralWorkers >= 0, "Can't have negative workers");
    BOSS_ASSERT(currentGasWorkers >= 0, "Can't have negative workers");

    int mineralsNeeded = mineralDifference - addedMinerals;
    int gasNeeded = gasDifference - addedGas;

    if ((mineralsNeeded > 0 && currentMineralWorkers == 0) || (gasNeeded > 0 && currentGasWorkers == 0))
    {
        return -1;
    }

    // if we still haven't added enough minerals, add more time
    if (mineralsNeeded > 0 || gasNeeded > 0)
    {
        int mineralIncome       = currentMineralWorkers * MPWPF;
        int gasIncome           = currentGasWorkers * GPWPF;
        int mineralTimeNeeded   = mineralIncome == 0 ? 0 : (mineralsNeeded / mineralIncome);
        int gasTimeNeeded       = gasIncome     == 0 ? 0 : (gasNeeded / gasIncome);

        // since this is integer division, check to see if we need one more step to go above required resources
        if (mineralTimeNeeded * mineralIncome < mineralsNeeded) { mineralTimeNeeded += 1; }
        if (gasTimeNeeded     * gasIncome     < gasNeeded)      { gasTimeNeeded += 1; }

        addedTime += std::max(mineralTimeNeeded, gasTimeNeeded);
    }
    
    return addedTime + m_currentFrame;
}

int GameState::whenBuilderReady(const ActionType action) const
{
    static const ActionType larva("Larva");
    static const ActionType hatchery("Hatchery");
    
    // if what builds this is a larva, we have to check when the next one will appear
    if (action.whatBuilds() == larva)
    {
        int minLarvaTime = 1000000;
        for (auto& unit : m_units)
        {
            if (!unit.getType().isHatchery()) { continue; }
            if (unit.numLarva() > 0) { return m_currentFrame; }
            // if it's an actual hatchery, check its build time
            int time = unit.timeUntilLarva();
            if (unit.getType() == hatchery) { time += unit.getTimeUntilFree(); }
            if (time < minLarvaTime)
            {
                minLarvaTime = time;
            }
        }
        return m_currentFrame + minLarvaTime;
        BOSS_ASSERT(minLarvaTime != 1000000, "Error in getting next larva time");
    }
    // otherwise, check to see when a builder will be ready
    else
    {
        int builderID = getBuilderID(action);
        BOSS_ASSERT(builderID != -1, "Didn't find when builder ready for %s", action.getName().c_str());
        return m_currentFrame + getUnit(builderID).getTimeUntilFree();
    }

}

int GameState::whenSupplyReady(const ActionType action) const
{
    int supplyNeeded = action.supplyCost() + m_currentSupply - m_maxSupply;
    if (supplyNeeded <= 0) { return m_currentFrame; }

    // search the actions in progress in reverse for the first supply provider
    for (size_t i(0); i < m_unitsBeingBuilt.size(); ++i)
    {
        const Unit & unit = getUnit(m_unitsBeingBuilt[m_unitsBeingBuilt.size() - 1 - i]);   
        if (unit.getType().supplyProvided() >= supplyNeeded)
        {
            return m_currentFrame + unit.getTimeUntilBuilt();
        }
    }

    BOSS_ASSERT(false, "Didn't find any supply in progress to build %s", action.getName().c_str());
    return m_currentFrame;
}

int GameState::whenPrerequisitesReady(const ActionType action) const
{
    // if this action requires no prerequisites, then they are ready right now
    if (action.required().empty())
    {
        return m_currentFrame;
    }

    // if it has prerequisites, we need to find the max-min time that any of the prereqs are free
    int whenPrereqReady = 0;
    for (auto & req : action.required())
    {

        // find the minimum time that this particular prereq will be ready
        int minReady = std::numeric_limits<int>::max();
        for (auto & unit : m_units)
        {
            if (!unit.getType().isEquivalentTo(req)) { continue; }
            bool morphEvolution = unit.getType().isMorphed() && unit.getType().whatBuilds().isEquivalentTo(req);
            int timeTillReqFilled = morphEvolution ? 0 : unit.getTimeUntilBuilt();
            minReady = std::min(minReady, timeTillReqFilled);
            if (timeTillReqFilled == 0) { break; }
        }
        // we can only build the type after the LAST of the prereqs are ready
        whenPrereqReady = std::max(whenPrereqReady, minReady);
        BOSS_ASSERT(whenPrereqReady != std::numeric_limits<int>::max(), "Did not find a prerequisite required to build %s", action.getName().c_str());
    }
      
    return m_currentFrame + whenPrereqReady;
}

int GameState::getBuilderID(const ActionType action) const
{
    int minWhenReady = std::numeric_limits<int>::max();
    int builderID = -1;

    // look over all our units and get when the next builder type is free
    for (auto & unit : m_units)
    {
        int whenReady = unit.whenCanBuild(action);
        
        // shortcut return if we found something that can build now
        if (whenReady == 0)
        {
            return unit.getID();
        }

        // terrain building cannot make addon while it is building

        // if the Unit can build the unit, set the new min
        if (whenReady != -1 && whenReady < minWhenReady)
        {
            minWhenReady = whenReady;
            builderID = unit.getID();
        }
    }

    BOSS_ASSERT(builderID != -1, "Didn't find a builder for %s", action.getName().c_str());

    return builderID;
}

bool GameState::haveBuilder(const ActionType type) const
{
    return type.whatBuildsCount() <= std::count_if(m_units.begin(), m_units.end(),
        [&type, this](const Unit& u)
        {
            int time = u.whenCanBuild(type);
            // if this is a worker and it's reserved for future gas production, it can only build something that would finish before the refinery its waiting for
            if (u.getType().isWorker() && time != -1 && u.reservedFor() != -1)
            {
                return ( time + type.buildTime() < m_units[u.reservedFor()].getTimeUntilBuilt() && !type.isMorphed()); 
            }
            return time != -1; 
        });;
}

bool GameState::havePrerequisites(const ActionType type) const
{
    return std::all_of(type.required().begin(), type.required().end(), 
           [this](const ActionType & req) 
        { 
            if (this->haveType(req))
            {
                return true;
            }
            for (auto& equi : req.equivalent())
            {
                if (this->haveType(equi))
                {
                    return true;
                }
            }
            return false;
        });
}

size_t GameState::getNumInProgress(const ActionType action) const
{
    return std::count_if(m_unitsBeingBuilt.begin(), m_unitsBeingBuilt.end(),
           [this, &action](const size_t & id) { return action == this->getUnit(id).getType(); } );
}

size_t GameState::getNumCompleted(const ActionType action) const
{
    return std::count_if(m_units.begin(), m_units.end(),
           [&action](const Unit & unit) { return action == unit.getType() && unit.getTimeUntilBuilt() == 0; } );
}

size_t GameState::getNumTotal(const ActionType action) const
{
    return std::count_if(m_units.begin(), m_units.end(), 
           [&action](const Unit & unit) { return action == unit.getType(); } );
}

bool GameState::haveType(const ActionType action) const
{
    return std::any_of(m_units.begin(), m_units.end(), 
           [&action](const Unit & i){ return i.getType() == action; });
}

int GameState::getSupplyInProgress() const
{
    return std::accumulate(m_unitsBeingBuilt.begin(), m_unitsBeingBuilt.end(), 0, 
           [this](size_t lhs, size_t rhs) { return lhs + this->getUnit(rhs).getType().supplyProvided(); });
}

int GameState::scaleResource(int baseResourceValue) const
{
    return baseResourceValue * ResourceScale;
}

const Unit & GameState::getUnit(const size_t id) const
{
    return m_units[id];
}

Unit & GameState::getUnit(const size_t & id)
{
    return m_units[id];
}

int GameState::getMinerals() const
{
    return (int)(m_minerals / ResourceScale);
}

int GameState::getGas() const
{
    return (int)(m_gas / ResourceScale);
}

int GameState::getCurrentSupply() const
{
    return m_currentSupply;
}

int GameState::getMaxSupply() const
{
    return m_maxSupply;
}

int GameState::getCurrentFrame() const
{
    return m_currentFrame;
}

void GameState::setMinerals(const int minerals)
{
    m_minerals = scaleResource(minerals);
}

void GameState::setGas(const int gas)
{
    m_gas = scaleResource(gas);
}

bool BOSS::GameState::operator==(const GameState& rhs) const
{
    return tie(m_units, m_unitsBeingBuilt, m_race, m_minerals, m_gas, m_currentSupply, m_maxSupply, m_currentFrame, m_mineralWorkers, m_gasWorkers, m_buildingWorkers, m_numRefineries, m_numDepots, m_previousAction) == tie(rhs.m_units, rhs.m_unitsBeingBuilt, rhs.m_race, rhs.m_minerals, rhs.m_gas, rhs.m_currentSupply, rhs.m_maxSupply, rhs.m_currentFrame, rhs.m_mineralWorkers, rhs.m_gasWorkers, rhs.m_buildingWorkers, rhs.m_numRefineries, rhs.m_numDepots, rhs.m_previousAction);
}

ActionType BOSS::GameState::getLastAction() const
{
    return m_previousAction;
}

int GameState::getRace() const
{
    return m_race;
}

bool GameState::canBuildNow(const ActionType action) const
{
	return whenCanBuild(action) == getCurrentFrame();
}

size_t GameState::getNumMineralWorkers() const
{
    return m_mineralWorkers;
}

size_t GameState::getNumGasWorkers() const
{
    return m_gasWorkers;
}

int GameState::getLastActionFinishTime() const
{
    return m_unitsBeingBuilt.empty() ? getCurrentFrame() : m_units[m_unitsBeingBuilt.front()].getTimeUntilBuilt();
}

int GameState::getNextFinishTime(const ActionType type) const
{
    auto it = std::find_if(m_unitsBeingBuilt.rbegin(), m_unitsBeingBuilt.rend(),
              [this, &type](const size_t & uid) { return this->getUnit(uid).getType() == type; });

    return it == m_unitsBeingBuilt.rend() ? getCurrentFrame() : m_units[*it].getTimeUntilFree();
}

std::string GameState::toString() const
{
	std::stringstream ss;
    char buf[2048];
    ss << std::setfill('0') << std::setw(7);
	ss << "\n--------------------------------------\n";
    
	ss << "Current  Frame: " << m_currentFrame  << " (" << (m_currentFrame  / (60 * 24)) << "m " << ((m_currentFrame  / 24) % 60) << "s)\n";
    ss << "Previous Frame: " << m_previousFrame << " (" << (m_previousFrame / (60 * 24)) << "m " << ((m_previousFrame / 24) % 60) << "s)\n\n";

	ss << "Units Completed:\n";
    const std::vector<ActionType> & allActions = ActionTypes::GetAllActionTypes();
	for (auto & type : ActionTypes::GetAllActionTypes())
	{
        size_t numCompleted = getNumCompleted(type);
        if (numCompleted > 0) 
        {
			ss << "\t" << numCompleted << "\t" << type.getName() << "\n";
        }
    }

	ss << "\nUnits In Progress:\n";
    for (auto & id : m_unitsBeingBuilt) 
    {
        auto & unit = getUnit(id);
        sprintf(buf, "%5d %5d %s\n", unit.getID(), unit.getTimeUntilBuilt(), unit.getType().getName().c_str());
        ss << buf;
    }

    ss << "\nAll Units:\n";
    ss << "---------------------------------------------------------------\n";
    ss << "   ID  BID Constr  Free Type            BuildType           bID\n";
    ss << "---------------------------------------------------------------\n";
    for (auto & unit : m_units) 
    {
        sprintf(buf, "%5d %4d %6d %5d %-15s %-15s %5d\n", unit.getID(), unit.getBuilderID(), unit.getTimeUntilBuilt(), unit.getTimeUntilFree(), unit.getType().getName().c_str(), unit.getBuildType().getName().c_str(), unit.getBuildID());
        ss << buf;
    }
    
    ss << "\nLegal Actions:\n";
    std::vector<ActionType> legalActions;
    getLegalActions(legalActions);
    ss << "--------------------------------------\n";
    sprintf(buf, "%5s %5s %5s %5s\n", "total", "build", "res", "pre");
    ss << buf;
    ss << "--------------------------------------\n";
    for (auto & type : legalActions)
    {
        sprintf(buf, "%5d %5d %5d %5d %s\n", (whenCanBuild(type)-m_currentFrame), (whenBuilderReady(type)-m_currentFrame), (whenResourcesReady(type)-m_currentFrame), (whenPrerequisitesReady(type)-m_currentFrame), type.getName().c_str());
        ss << buf;
    }

	ss << "\nResources:\n";
    sprintf(buf, "%7d   Minerals %7d\n%7d   Gas  %7d\n%7d   Mineral Workers %7d\n%7d   Gas Workers %7d\n%3d/%3d  Supply\n", getMinerals(), m_minerals, getGas(), m_gas, getNumMineralWorkers(), getNumMineralWorkers()*MPWPF, getNumGasWorkers(), getNumGasWorkers()*GPWPF, m_currentSupply/2, m_maxSupply/2);
    ss << buf;

    ss << "--------------------------------------\n";
    ss << "Supply In Progress: " << getSupplyInProgress() << "\n";
    ss << "Next Probe Finish: " << getNextFinishTime(ActionTypes::GetActionType("Probe")) << "\n";
    //printPath();

    return ss.str();
}

std::string GameState::toStringAllUnits() const
{
    std::stringstream ss;
    char buf[2048];

    ss << "All Units:\n";
    ss << "---------------------------------------------------------------\n";
    ss << "   ID  BID Constr  Free Type            BuildType           bID\n";
    ss << "---------------------------------------------------------------\n";
    for (auto& unit : m_units)
    {
        sprintf(buf, "%5d %4d %6d %5d %-15s %-15s %5d %3d %5d\n", unit.getID(), unit.getBuilderID(), unit.getTimeUntilBuilt(), unit.getTimeUntilFree(), unit.getType().getName().c_str(), unit.getBuildType().getName().c_str(), unit.getBuildID(), unit.numLarva(), unit.timeUntilLarva());
        ss << buf;
    }

    return ss.str();
}

std::string GameState::toStringLegalActions() const
{
    std::stringstream ss;
    char buf[2048];

    ss << "\nLegal Actions:\n";
    std::vector<ActionType> legalActions;
    getLegalActions(legalActions);
    ss << "--------------------------------------\n";
    sprintf(buf, "%5s %5s %5s %5s\n", "total", "build", "res", "pre");
    ss << buf;
    ss << "--------------------------------------\n";
    for (auto& type : legalActions)
    {
        sprintf(buf, "%5d %5d %5d %5d %s\n", (whenCanBuild(type) - m_currentFrame), (whenBuilderReady(type) - m_currentFrame), (whenResourcesReady(type) - m_currentFrame), (whenPrerequisitesReady(type) - m_currentFrame), type.getName().c_str());
        ss << buf;
    }

    return ss.str();
}

std::string GameState::toStringResources() const
{
    std::stringstream ss;
    char buf[2048];

    ss << "\nCurrent  Frame: " << m_currentFrame << " (" << (m_currentFrame / (60 * 24)) << "m " << ((m_currentFrame / 24) % 60) << "s)\n";
    ss << "Previous Frame: " << m_previousFrame << " (" << (m_previousFrame / (60 * 24)) << "m " << ((m_previousFrame / 24) % 60) << "s)\n\n";

    ss << "Resources:\n";
    sprintf(buf, "%7d   Minerals  %7d\n%7d   Gas       %7d\n%7d   M Workers %7d\n%7d   G Workers %7d\n%3d/%3d  Supply\n", getMinerals(), m_minerals, getGas(), m_gas, getNumMineralWorkers(), getNumMineralWorkers() * MPWPF, getNumGasWorkers(), getNumGasWorkers() * GPWPF, m_currentSupply / 2, m_maxSupply / 2);
    ss << buf;

    ss << "--------------------------------------\n";
    ss << "Supply In Progress: " << getSupplyInProgress() << "\n";
    ss << "Next Probe Finish: " << getNextFinishTime(ActionTypes::GetActionType("Probe")) << "\n";
    
    return ss.str();
}

std::string GameState::toStringInProgress() const
{
    std::stringstream ss;
    char buf[2048];
    ss << std::setfill('0') << std::setw(7);

    ss << "Units In Progress:\n";
    for (auto& id : m_unitsBeingBuilt)
    {
        auto& unit = getUnit(id);
        sprintf(buf, "%5d %5d %s\n", unit.getID(), unit.getTimeUntilBuilt(), unit.getType().getName().c_str());
        ss << buf;
    }
    return ss.str();
}

std::string GameState::toStringCompleted() const
{
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(7);

    ss << "Units Completed:\n";
    const std::vector<ActionType>& allActions = ActionTypes::GetAllActionTypes();
    for (auto& type : ActionTypes::GetAllActionTypes())
    {
        size_t numCompleted = getNumCompleted(type);
        if (numCompleted > 0)
        {
            ss << "\t" << numCompleted << "\t" << type.getName() << "\n";
        }
    }
    return ss.str();
}

const std::vector<Unit>& GameState::getUnits() const
{
    return m_units;
}

const GameState BOSS::GameState::StarCraft1_ZergStart()
{
    GameState state;
    const static ActionType drone("Drone");
    const static ActionType hatch("Hatchery");
    const static ActionType over("Overlord");
    state.addUnit(drone);
    state.addUnit(drone);
    state.addUnit(drone);
    state.addUnit(drone);
    state.addUnit(hatch);
    state.addUnit(over);
    state.setMinerals(50);
    return state;
}

const GameState BOSS::GameState::StarCraft1_TerranStart()
{
    GameState state;
    const static ActionType scv("SCV");
    const static ActionType com("CommandCenter");
    state.addUnit(scv);
    state.addUnit(scv);
    state.addUnit(scv);
    state.addUnit(scv);
    state.addUnit(com);
    state.setMinerals(50);
    return state;
}

const GameState BOSS::GameState::StarCraft1_ProtossStart()
{
    GameState state;
    const static ActionType probe("Probe");
    const static ActionType nexus("Nexus");
    state.addUnit(probe);
    state.addUnit(probe);
    state.addUnit(probe);
    state.addUnit(probe);
    state.addUnit(nexus);
    state.setMinerals(50);
    return state;
}
