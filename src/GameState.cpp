#include "GameState.h"
#include <numeric>
#include <iomanip>

using namespace BOSS;

//const double MPWPF = 0.045f;
//const double GPWPF = 0.070f;
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

    const size_t mineralWorkers = m_mineralWorkers + m_buildingWorkers;
    const size_t refineriesInProgress = getNumInProgress(ActionTypes::GetRefinery(m_race));
    const size_t numRefineries  = m_numRefineries + refineriesInProgress;
    const size_t numDepots      = m_numDepots + getNumInProgress(ActionTypes::GetResourceDepot(m_race));

    if (action.getName() == "SiegeTankTankMode")
    {
        int a = 6;
    }

    // 
    if (mineralWorkers == 0) { return false; }
    	
    // if it's a unit and we are out of supply and aren't making an overlord, it's not legal
	if (!action.isMorphed() && !action.isSupplyProvider() && ((m_currentSupply + action.supplyCost()) > (m_maxSupply + getSupplyInProgress()))) { return false; }

    // TODO: require an extra for refineries byt not buildings
    // rules for buildings which are built by workers
    if (action.isBuilding() && !action.isMorphed() && !action.isAddon() && (mineralWorkers == 0)) { return false; }

    // if we have no gas income we can't make a gas unit
    if ((m_gas < action.gasPrice()) && (m_gasWorkers == 0)) { return false; }

    // if we have no mineral income we'll never have a minerla unit
    if ((m_minerals < action.mineralPrice()) && (mineralWorkers == 0)) { return false; }

    // don't build more refineries than resource depots
    if (action.isRefinery() && (numRefineries >= numDepots)) { return false; }

    // we don't need to go over the maximum supply limit with supply providers
    if (action.isSupplyProvider() && (m_maxSupply + getSupplyInProgress() > 400)) { return false; }

    // if it's an addon, we need to check if we have a building without an addon to build it
    if (action.isAddon())
    {

    }

    // TODO: can only build one of a tech type
    // TODO: check to see if an addon can ever be built
    if (!haveBuilder(action)) { return false; }

    if (!havePrerequisites(action)) { return false; }

    return true;
}

void GameState::doAction(const ActionType type)
{
    int previousFrame = m_currentFrame;

    // figure out when this action can be done and fast forward to it
    const int timeWhenReady = whenCanBuild(type);
    fastForward(timeWhenReady);

    // subtract the resource cost
    m_minerals  -= scaleResource(type.mineralPrice());
    m_gas       -= scaleResource(type.gasPrice());

    // if it's a Terran building that's not an addon, the worker removed from minerals
    if (type.getRace() == Races::Terran && type.isBuilding() && !type.isAddon())
    {
        m_mineralWorkers--;
        m_buildingWorkers++;
    }

    // if it's a Zerg unit that requires a Drone, remove a mineral worker
    if (type.getRace() == Races::Zerg && type.whatBuilds().isWorker())
    {
        m_mineralWorkers--;
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
        }

        // complete the action and remove it from the list
        completeUnit(unit);
        m_unitsBeingBuilt.pop_back();
    }

    // update resources from the last action finished to the ff frame
    int timeElapsed = toFrame - lastActionFinishTime;
    m_minerals      += timeElapsed * MPWPF * m_mineralWorkers;
    m_gas           += timeElapsed * GPWPF * m_gasWorkers;

    // update all the intances to the ff time
    for (auto & unit : m_units)
    {
        unit.fastForward(toFrame - previousFrame);
    }

    m_currentFrame = toFrame;
}

void GameState::completeUnit(Unit & unit)
{
	unit.complete();
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
        m_numRefineries++;
        BOSS_ASSERT(m_numRefineries <= m_numDepots, "Shouldn't have more refineries than depots");
        int needGasWorkers = std::max(0, (WorkersPerRefinery*m_numRefineries - m_gasWorkers));
        BOSS_ASSERT(needGasWorkers < m_mineralWorkers, "Shouldn't need more gas workers than we have mineral workers");
        m_mineralWorkers -= needGasWorkers;
        m_gasWorkers += needGasWorkers;
    }
    else if (unit.getType().isDepot())
    {
        m_numDepots++;
    }
}

// add a unit of the given type to the state
// if builderID is -1 (default) the unit is added as completed, otherwise it begins construction with the builder
void GameState::addUnit(const ActionType type, int builderID)
{
    BOSS_ASSERT(m_race == Races::None || type.getRace() == m_race, "Adding an Unit of a different race");

    m_race = type.getRace();
    Unit unit(type, m_units.size(), builderID);

    m_units.push_back(unit);
    m_currentSupply += unit.getType().supplyCost();
    
    // if we have a valid builder for this object, add it to the Units being built
    if (builderID != -1)
    {
        getUnit(builderID).startBuilding(m_units.back());

        // add the Unit ID being built and sort the list
        m_unitsBeingBuilt.push_back(unit.getID());

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
    // if there's no builder, complete the unit now and skip the unit in progress step
    else
    {
        completeUnit(m_units.back());
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
        else if (unit.getType().isWorker())
        {
            currentMineralWorkers++;
        }
        // finishing a refinery adjusts the worker count
        else if (unit.getType().isRefinery())
        {
            BOSS_ASSERT(currentMineralWorkers > WorkersPerRefinery, "Not enough mineral workers \n");
            currentMineralWorkers -= WorkersPerRefinery; 
            currentGasWorkers += WorkersPerRefinery;
        }

        // update the last action
        lastActionFinishFrame = actionCompletionTime;
    }

    // if we still haven't added enough minerals, add more time
    if (addedMinerals < mineralDifference || addedGas < gasDifference)
    {
        BOSS_ASSERT(currentMineralWorkers > 0, "Shouldn't have 0 mineral workers");

        int mineralIncome       = currentMineralWorkers * MPWPF;
        int gasIncome           = currentGasWorkers * GPWPF;
        int mineralsNeeded      = mineralDifference - addedMinerals;
        int gasNeeded           = gasDifference - addedGas;
        int mineralTimeNeeded   = mineralIncome == 0 ? 0 : (mineralsNeeded / mineralIncome);
        int gasTimeNeeded       = gasIncome     == 0 ? 0 : (gasDifference / gasIncome);

        // since this is integer division, check to see if we need one more step to go above required resources
        if (mineralTimeNeeded * mineralIncome < mineralsNeeded) { mineralTimeNeeded += 1; }
        if (gasTimeNeeded     * gasIncome     < gasNeeded)      { gasTimeNeeded += 1; }

        addedTime += std::max(mineralTimeNeeded, gasTimeNeeded);
    }
    
    return addedTime + m_currentFrame;
}

int GameState::whenBuilderReady(const ActionType action) const
{
    int builderID = getBuilderID(action);

    BOSS_ASSERT(builderID != -1, "Didn't find when builder ready for %s", action.getName().c_str());

    return m_currentFrame + getUnit(builderID).getTimeUntilFree();
}

int GameState::whenSupplyReady(const ActionType action) const
{
    int supplyNeeded = action.supplyCost() + m_currentSupply - m_maxSupply;
    if (supplyNeeded <= 0) { return m_currentFrame; }

    // search the actions in progress in reverse for the first supply provider
    for (size_t i(0); i < m_unitsBeingBuilt.size(); ++i)
    {
        const Unit & unit = getUnit(m_unitsBeingBuilt[m_unitsBeingBuilt.size() - 1 - i]);   
        if (unit.getType().supplyProvided() > supplyNeeded)
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
            if (unit.getType() != req) { continue; }
            minReady = std::min(minReady, unit.getTimeUntilFree());
            if (unit.getTimeUntilFree() == 0) { break; }
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
    return std::any_of(m_units.begin(), m_units.end(), 
           [&type](const Unit & u){ return u.whenCanBuild(type) != -1; });
}

bool GameState::havePrerequisites(const ActionType type) const
{
    return std::all_of(type.required().begin(), type.required().end(), 
           [this](const ActionType & req) { return this->haveType(req); });
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
        sprintf(buf, "%5d %4d %6d %5d %-15s %-15s %5d\n", unit.getID(), unit.getBuilderID(), unit.getTimeUntilBuilt(), unit.getTimeUntilFree(), unit.getType().getName().c_str(), unit.getBuildType().getName().c_str(), unit.getBuildID());
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

    ss << "\nUnits In Progress:\n";
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