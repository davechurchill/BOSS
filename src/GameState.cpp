#include "GameState.h"
#include <numeric>
#include <iomanip>

using namespace BOSS;

const float MPWPF = 0.045f;
const float GPWPF = 0.070f;
const int WorkersPerRefinery = 3;

GameState::GameState()
    : m_minerals        (0.0f)
    , m_gas             (0.0f)
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

bool GameState::isLegal(const ActionType & action) const
{
    if (action.getRace() != m_race) { return false; }

    const size_t mineralWorkers = m_mineralWorkers + m_buildingWorkers;
    const size_t refineriesInProgress = getNumInProgress(ActionTypes::GetRefinery(m_race));
    const size_t numRefineries  = m_numRefineries + refineriesInProgress;
    const size_t numDepots      = m_numDepots + getNumInProgress(ActionTypes::GetResourceDepot(m_race));

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

    // TODO: can only build one of a tech type
    // TODO: check to see if an addon can ever be built
    if (!haveBuilder(action)) { return false; }

    if (!havePrerequisites(action)) { return false; }

    return true;
}

void GameState::doAction(const ActionType & type)
{
    int previousFrame = m_currentFrame;

    // figure out when this action can be done and fast forward to it
    int timeWhenReady = whenCanBuild(type);
    fastForward(timeWhenReady);

    // subtract the resource cost
    m_minerals  -= type.mineralPrice();
    m_gas       -= type.gasPrice();

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
    addInstance(type, getBuilderID(type));
}

void GameState::fastForward(const int & toFrame)
{
    if (toFrame == m_currentFrame) { return; }

    BOSS_ASSERT(toFrame > m_currentFrame, "Must ff to the future");

    m_previousFrame             = m_currentFrame;
    int previousFrame           = m_currentFrame;
    int lastActionFinishTime    = m_currentFrame;
    
    // iterate backward over actions in progress since they're sorted
    // that way for ease of deleting the finished ones
    while (!m_instancesBeingBuilt.empty())
    {
        Instance & instance = getInstance(m_instancesBeingBuilt.back());
        ActionType type = instance.getType();

        // if the current action in progress will finish after the ff time, we can stop
        int actionCompletionTime = previousFrame + instance.getTimeUntilBuilt();
        if (actionCompletionTime > toFrame) { break; }

        // add the resources we gathered during this time period
        int timeElapsed         = actionCompletionTime - lastActionFinishTime;
        m_minerals              += timeElapsed * MPWPF * m_mineralWorkers;
        m_gas                   += timeElapsed * GPWPF * m_gasWorkers;
        lastActionFinishTime    = actionCompletionTime;
        
        // if it's a Terran building that's not an addon, the worker returns to minerals
        if (type.getRace() == Races::Terran && type.isBuilding() && !type.isAddon())
        {
            m_mineralWorkers++;
        }

        // complete the action and remove it from the list
        completeInstance(instance);
        m_instancesBeingBuilt.pop_back();
    }

    // update resources from the last action finished to the ff frame
    int timeElapsed = toFrame - lastActionFinishTime;
    m_minerals      += timeElapsed * MPWPF * m_mineralWorkers;
    m_gas           += timeElapsed * GPWPF * m_gasWorkers;

    // update all the intances to the ff time
    for (auto & instance : m_instances)
    {
        instance.fastForward(toFrame - previousFrame);
    }

    m_currentFrame = toFrame;
}

void GameState::completeInstance(Instance & instance)
{
    instance.complete();
    m_maxSupply += instance.getType().supplyProvided();

    // if it's a worker, assign it to the correct job
    if (instance.getType().isWorker())
    {
        m_mineralWorkers++;
        int needGasWorkers = std::max(0, (WorkersPerRefinery*m_numRefineries - m_gasWorkers));
        BOSS_ASSERT(needGasWorkers < m_mineralWorkers, "Shouldn't need more gas workers than we have mineral workers");
        m_mineralWorkers -= needGasWorkers;
        m_gasWorkers += needGasWorkers;
    }
    else if (instance.getType().isRefinery())
    {
        m_numRefineries++;
        BOSS_ASSERT(m_numRefineries <= m_numDepots, "Shouldn't have more refineries than depots");
        int needGasWorkers = std::max(0, (WorkersPerRefinery*m_numRefineries - m_gasWorkers));
        BOSS_ASSERT(needGasWorkers < m_mineralWorkers, "Shouldn't need more gas workers than we have mineral workers");
        m_mineralWorkers -= needGasWorkers;
        m_gasWorkers += needGasWorkers;
    }
    else if (instance.getType().isDepot())
    {
        m_numDepots++;
    }
}

void GameState::addInstance(const ActionType & type, int builderID)
{
    BOSS_ASSERT(m_race == Races::None || type.getRace() == m_race, "Adding an instance of a different race");

    m_race = type.getRace();
    Instance instance(type, m_instances.size(), builderID);

    m_instances.push_back(instance);
    m_currentSupply += instance.getType().supplyCost();
    
    // if we have a valid builder for this object, add it to the instances being built
    if (builderID != -1)
    {
        getInstance(builderID).startBuilding(m_instances.back());

        // add the instance ID being built and sort the list of all actions in progress in reverse finish time order
        m_instancesBeingBuilt.push_back(instance.getID());
        std::sort(m_instancesBeingBuilt.begin(), m_instancesBeingBuilt.end(), 
            [this](const size_t & id1, const size_t & id2)
            { return this->getInstance(id1).getTimeUntilBuilt() > this->getInstance(id2).getTimeUntilBuilt(); });
    }
    // otherwise just complete the instance now and skip the instance in progress step
    else
    {
        completeInstance(m_instances.back());
    }
}

int GameState::whenCanBuild(const ActionType & action) const
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

int GameState::whenResourcesReady(const ActionType & action) const
{
    if (m_minerals >= action.mineralPrice() && m_gas >= action.gasPrice())
    {
        return getCurrentFrame();
    }

    if (action.isDepot())
    {
        int a = 6;
    }

    int previousFrame           = m_currentFrame;
    int currentMineralWorkers   = m_mineralWorkers;
    int currentGasWorkers       = m_gasWorkers;
    int lastActionFinishFrame   = m_currentFrame;
    int addedTime               = 0;
    float addedMinerals         = 0;
    float addedGas              = 0;
    float mineralDifference     = action.mineralPrice() - m_minerals;
    float gasDifference         = action.gasPrice() - m_gas;

    // loop through each action in progress, adding the minerals we would gather from each interval
    for (size_t i(0); i < m_instancesBeingBuilt.size(); ++i)
    {
        const Instance & instance = getInstance(m_instancesBeingBuilt[m_instancesBeingBuilt.size() - 1 - i]);
        int actionCompletionTime = previousFrame + instance.getTimeUntilBuilt();

        // the time elapsed and the current minerals per frame
        int elapsed = actionCompletionTime - lastActionFinishFrame;

        // the amount of minerals that would be added this time step
        float tempAddMinerals = elapsed * currentMineralWorkers * MPWPF;
        float tempAddGas      = elapsed * currentGasWorkers * GPWPF;

        // if this amount isn't enough, update the amount added for this interval
        if (addedMinerals + tempAddMinerals < mineralDifference || addedGas + tempAddGas < gasDifference)
        {
            addedMinerals += tempAddMinerals;
            addedGas += tempAddGas;
            addedTime += elapsed;
        }
        else { break; }

        // finishing a building as terran gives you a mineral worker back
        if (instance.getType().isBuilding() && !instance.getType().isAddon() && (instance.getType().getRace() == Races::Terran))
        {
            currentMineralWorkers++;
        }
        // finishing a worker gives us another mineral worker
        else if (instance.getType().isWorker())
        {
            currentMineralWorkers++;
        }
        // finishing a refinery adjusts the worker count
        else if (instance.getType().isRefinery())
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

        int mineralTimeNeeded = (int)std::ceil((mineralDifference - addedMinerals) / (currentMineralWorkers * MPWPF));
        int gasTimeNeeded     = (int)std::ceil((gasDifference - addedGas) / (currentGasWorkers * GPWPF));
        addedTime             += std::max(mineralTimeNeeded, gasTimeNeeded);
    }
    
    return addedTime + m_currentFrame;
}

int GameState::whenBuilderReady(const ActionType & action) const
{
    int builderID = getBuilderID(action);

    BOSS_ASSERT(builderID != -1, "Didn't find when builder ready for %s", action.getName().c_str());

    return m_currentFrame + getInstance(builderID).getTimeUntilFree();
}

int GameState::whenSupplyReady(const ActionType & action) const
{
    int supplyNeeded = action.supplyCost() + m_currentSupply - m_maxSupply;
    if (supplyNeeded <= 0) { return m_currentFrame; }

    // search the actions in progress in reverse for the first supply provider
    for (size_t i(0); i < m_instancesBeingBuilt.size(); ++i)
    {
        const Instance & instance = getInstance(m_instancesBeingBuilt[m_instancesBeingBuilt.size() - 1 - i]);   
        if (instance.getType().supplyProvided() > supplyNeeded)
        {
            return m_currentFrame + instance.getTimeUntilBuilt();
        }
    }

    BOSS_ASSERT(false, "Didn't find any supply in progress to build %s", action.getName().c_str());
    return m_currentFrame;
}

int GameState::whenPrerequisitesReady(const ActionType & action) const
{
    return m_currentFrame;
}

bool GameState::haveBuilder(const ActionType & type) const
{
    return std::any_of(m_instances.begin(), m_instances.end(), 
                      [&type](const Instance & i){ return i.whenCanBuild(type) != -1; });
}

bool GameState::havePrerequisites(const ActionType & type) const
{
    return std::all_of(type.required().begin(), type.required().end(), 
                      [this](const ActionType & req) { return this->haveType(req); });
}

int GameState::getBuilderID(const ActionType & action) const
{
    int minWhenReady = std::numeric_limits<int>::max();
    int builderID = -1;

    // look over all our units and get when the next builder type is free
    for (auto & instance : m_instances)
    {
        int whenReady = instance.whenCanBuild(action);
        
        // shortcut return if we found something that can build now
        if (whenReady == 0)
        {
            return instance.getID();
        }

        // if the instance can build the unit, set the new min
        if (whenReady != -1 && whenReady < minWhenReady)
        {
            minWhenReady = m_currentFrame + whenReady;
            builderID = instance.getID();
        }
    }

    BOSS_ASSERT(builderID != -1, "Didn't find a builder for %s", action.getName().c_str());

    return builderID;
}

size_t GameState::getNumInProgress(const ActionType & action) const
{
    return std::count_if(m_instancesBeingBuilt.begin(), m_instancesBeingBuilt.end(),
                        [this, &action](const size_t & id) { return action == this->getInstance(id).getType(); } );
}

size_t GameState::getNumCompleted(const ActionType & action) const
{
    return std::count_if(m_instances.begin(), m_instances.end(),
                        [&action](const Instance & instance) { return action == instance.getType() && instance.getTimeUntilBuilt() == 0; } );
}

size_t GameState::getNumTotal(const ActionType & action) const
{
    return std::count_if(m_instances.begin(), m_instances.end(),
                        [&action](const Instance & instance) { return action == instance.getType(); } );
}

bool GameState::haveType(const ActionType & action) const
{
    return std::any_of(m_instances.begin(), m_instances.end(), 
                        [&action](const Instance & i){ return i.getType() == action; });
}

int GameState::getSupplyInProgress() const
{
    int sum = 0;
    for (auto & id : m_instancesBeingBuilt) { sum += getInstance(id).getType().supplyProvided(); }
    return sum;
}

const Instance & GameState::getInstance(const size_t & id) const
{
    return m_instances[id];
}

Instance & GameState::getInstance(const size_t & id)
{
    return m_instances[id];
}

const float & GameState::getMinerals() const
{
    return m_minerals;
}

const float & GameState::getGas() const
{
    return m_gas;
}

const int & GameState::getCurrentSupply() const
{
    return m_currentSupply;
}

const int & GameState::getMaxSupply() const
{
    return m_maxSupply;
}

const int & GameState::getCurrentFrame() const
{
    return m_currentFrame;
}

std::string GameState::toString() const
{
	std::stringstream ss;
    char buf[256];
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
    for (auto & id : m_instancesBeingBuilt) 
    {
        auto & instance = getInstance(id);
        sprintf(buf, "%6d %6d %s\n", instance.getID(), instance.getTimeUntilBuilt(), instance.getType().getName().c_str());
        ss << buf;
    }

    ss << "\nAll Instances:\n";
    for (auto & instance : m_instances) 
    {
        sprintf(buf, "%6d %6d %s\n", instance.getID(), instance.getTimeUntilFree(), instance.getType().getName().c_str());
        ss << buf;
    }
    
    ss << "\nLegal Actions:\n";
    std::vector<ActionType> legalActions;
    getLegalActions(legalActions);
    for (auto & type : legalActions)
    {
        sprintf(buf, "%6d %6d %6d %s\n", (whenCanBuild(type)-m_currentFrame), (whenBuilderReady(type)-m_currentFrame), (whenResourcesReady(type)-m_currentFrame), type.getName().c_str());
        ss << buf;
    }

	ss << "\nResources:\n";
    sprintf(buf, "%7d   Minerals\n%7d   Gas\n%7d   Mineral Workers\n%7d   Gas Workers\n%3d/%3d  Supply\n", (int)m_minerals, (int)m_gas, m_mineralWorkers, m_gasWorkers, m_currentSupply/2, m_maxSupply/2);
    ss << buf;

    ss << "--------------------------------------\n";
    //printPath();

    return ss.str();
}