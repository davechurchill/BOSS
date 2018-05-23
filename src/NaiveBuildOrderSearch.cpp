#include "NaiveBuildOrderSearch.h"
#include "ActionSet.h"

using namespace BOSS;

NaiveBuildOrderSearch::NaiveBuildOrderSearch(const GameState & state, const BuildOrderSearchGoal & goal)
    : m_state(state)
    , m_goal(goal)
    , m_naiveSolved(false)
{

}

bool NaiveBuildOrderSearch::checkUnsolvable()
{
    const ActionType & worker = ActionTypes::GetWorker(m_state.getRace());
    const ActionType & supply = ActionTypes::GetSupplyProvider(m_state.getRace());
    const ActionType & depot = ActionTypes::GetResourceDepot(m_state.getRace());

    size_t mineralWorkers = m_state.getNumMineralWorkers();
    size_t numDepot = m_state.getNumTotal(depot);

    if (mineralWorkers == 0 || numDepot == 0)
    {
        return true;
    }

    if (!m_state.isLegal(worker) && !m_state.isLegal(supply))
    {
        return true;
    }

    return false;
}

const BuildOrder & NaiveBuildOrderSearch::solve()
{
    if (m_naiveSolved)
    {
        return m_buildOrder;
    }

    if (checkUnsolvable())
    {
        bool temp = checkUnsolvable();
        m_buildOrder = BuildOrder();
        return m_buildOrder;
    }

    ActionSet wanted;
    int minWorkers = 0;

    const ActionType & worker = ActionTypes::GetWorker(m_state.getRace());

    // add everything from the goal to the needed set
    for (const ActionType & actionType : ActionTypes::GetAllActionTypes())
    {
        size_t numCompleted = m_state.getNumTotal(actionType);
            
        if (m_goal.getGoal(actionType) > numCompleted)
        {
            wanted.add(actionType);
        }
    }

    if (wanted.size() == 0)
    {
        return m_buildOrder;
    }

    // Calculate which prerequisite units we need to build to achieve the units we want from the goal
    ActionSet requiredToBuild;
    Tools::CalculatePrerequisitesRequiredToBuild(m_state, wanted, requiredToBuild);

    // Add the required units to a preliminary build order
    BuildOrder buildOrder;
    for (size_t a(0); a < requiredToBuild.size(); ++a)
    {
        if (requiredToBuild.contains(a))
        {
            buildOrder.add(ActionType(a));
        }
    }

    // Add some workers to the build order if we don't have many, this usually gives a lower upper bound
    int requiredWorkers = minWorkers - m_state.getNumCompleted(ActionTypes::GetWorker(m_state.getRace()));
    buildOrder.add(worker, requiredWorkers);

    // Add the goal units to the end of the build order 
    for (const ActionType & actionType : ActionTypes::GetAllActionTypes())
    {
        int need = (int)m_goal.getGoal(actionType);
        int have = (int)m_state.getNumTotal(actionType);
        int numNeeded = need - have - buildOrder.getTypeCount(actionType);
         
        buildOrder.add(actionType, numNeeded);
    }

    // if we are zerg, make sure we have enough morphers for morphed units
    if (m_state.getRace() == Races::Zerg)
    {
        // do this whole thing twice so that Hive->Lair->Hatchery is satisfied
        for (size_t t=0; t<2; ++t)
        {
            std::vector<size_t> neededMorphers(ActionTypes::GetAllActionTypes().size(), 0);
            for (size_t i(0); i < ActionTypes::GetAllActionTypes().size(); ++i)
            {
                const ActionType & type(i);

                if (type.isMorphed())
                {
                    const ActionType & morpher = type.whatBuilds();

                    int willMorph = buildOrder.getTypeCount(type);
                    int haveMorpher = m_state.getNumTotal(morpher);
                    int boMoprher = buildOrder.getTypeCount(morpher);

                    int need = willMorph - haveMorpher - boMoprher;

                    if (need > 0)
                    {
                        neededMorphers[morpher.getID()] += need;
                    }
                }
            }
            
            // add the morphers to the build order
            for (size_t i(0); i<neededMorphers.size(); ++i)
            {
                buildOrder.add(ActionType(i), neededMorphers[i]);
            }
        }

        // special case: hydra/lurker both in goal, need to add hydras, same with creep/sunken and muta/guardian
        // ignore other spire / hatchery since they recursively serve all purposes
        static const ActionType & Hydralisk     = ActionTypes::GetActionType("Hydralisk");
        static const ActionType & Lurker        = ActionTypes::GetActionType("Lurker");
        static const ActionType & Creep         = ActionTypes::GetActionType("CreepColony");
        static const ActionType & Sunken        = ActionTypes::GetActionType("SunkenColony");
        static const ActionType & Spore         = ActionTypes::GetActionType("SporeColony");
        static const ActionType & Mutalisk      = ActionTypes::GetActionType("Mutalisk");
        static const ActionType & Guardian      = ActionTypes::GetActionType("Guardian");
        static const ActionType & Devourer      = ActionTypes::GetActionType("Devourer");

        if (m_goal.getGoal(Hydralisk) > 0)
        {
            int currentHydras = m_state.getNumTotal(Hydralisk) + buildOrder.getTypeCount(Hydralisk) - buildOrder.getTypeCount(Lurker);
            int additionalHydras = m_goal.getGoal(Hydralisk) - currentHydras;
            buildOrder.add(Hydralisk, additionalHydras);
        }

        if (m_goal.getGoal(Guardian) > 0 && m_goal.getGoal(Devourer) > 0)
        {
            int currentMutas = m_state.getNumTotal(Mutalisk) + buildOrder.getTypeCount(Mutalisk);
            int additionalMutas = buildOrder.getTypeCount(Guardian) + buildOrder.getTypeCount(Devourer) - currentMutas;
            buildOrder.add(Mutalisk, additionalMutas);
        }

        if (m_goal.getGoal(Mutalisk) > 0)
        {
            int currentMutas = m_state.getNumTotal(Mutalisk) + buildOrder.getTypeCount(Mutalisk) - buildOrder.getTypeCount(Guardian) - buildOrder.getTypeCount(Devourer);
            int additionalMutas = m_goal.getGoal(Mutalisk) - currentMutas;
            buildOrder.add(Mutalisk, additionalMutas);
        }

        if (m_goal.getGoal(Sunken) > 0 && m_goal.getGoal(Spore) > 0)
        {
            int currentCreep = m_state.getNumTotal(Creep) + buildOrder.getTypeCount(Creep);
            int additionalCreep = buildOrder.getTypeCount(Spore) + buildOrder.getTypeCount(Sunken) - currentCreep;
            buildOrder.add(Creep, additionalCreep);
        }

        if (m_goal.getGoal(Creep) > 0)
        {
            int currentCreep = m_state.getNumTotal(Creep) + buildOrder.getTypeCount(Creep) - buildOrder.getTypeCount(Spore) - buildOrder.getTypeCount(Sunken);
            int additionalCreep = m_goal.getGoal(Creep) - currentCreep;
            buildOrder.add(Creep, additionalCreep);
        }
    }

    // figure out how many workers are needed for the build order to be legal      
    size_t workersNeeded = m_goal.getGoal(worker);

    // we need enough workers to fill all the refineries that will be built
    size_t gasWorkersNeeded = 3*m_state.getNumTotal(ActionTypes::GetRefinery(m_state.getRace())) + 3*buildOrder.getTypeCount(ActionTypes::GetRefinery(m_state.getRace()));

    workersNeeded = std::max(workersNeeded, gasWorkersNeeded);

    // special case for zerg: buildings consume drones
    if (m_state.getRace() == Races::Zerg)
    {
        for (const ActionType & type : ActionTypes::GetAllActionTypes())
        {
            if (type.whatBuilds().isWorker() && !type.isMorphed())
            {
                workersNeeded += buildOrder.getTypeCount(type);
            }
        }
    }

    int workersToAdd = workersNeeded - m_state.getNumTotal(worker) - buildOrder.getTypeCount(worker);
    workersToAdd = std::max(0, workersToAdd);
    
    buildOrder.add(worker, workersToAdd);


    // Check to see if we have enough buildings for the required addons
    if (m_state.getRace() == Races::Terran)
    {
        // Terran buildings that can make addons
        static const ActionType CommandCenter   = ActionTypes::GetActionType("CommandCenter");
        static const ActionType Factory         = ActionTypes::GetActionType("Factory");
        static const ActionType Starport        = ActionTypes::GetActionType("Starport");
        static const ActionType ScienceFacility = ActionTypes::GetActionType("ScienceFacility");

        // Terran building addons
        static const ActionType ComsatStation   = ActionTypes::GetActionType("ComsatStation");
        static const ActionType NuclearSilo     = ActionTypes::GetActionType("NuclearSilo");
        static const ActionType MachineShop     = ActionTypes::GetActionType("MachineShop");
        static const ActionType ControlTower    = ActionTypes::GetActionType("ControlTower");
        static const ActionType PhysicsLab      = ActionTypes::GetActionType("PhysicsLab");
        static const ActionType CovertOps       = ActionTypes::GetActionType("CovertOps");

        int numCommandCenters   = m_state.getNumTotal(CommandCenter)   + buildOrder.getTypeCount(CommandCenter);
        int numFactories        = m_state.getNumTotal(Factory)         + buildOrder.getTypeCount(Factory);
        int numStarports        = m_state.getNumTotal(Starport)        + buildOrder.getTypeCount(Starport);
        int numSci              = m_state.getNumTotal(ScienceFacility) + buildOrder.getTypeCount(ScienceFacility);

        int commandCenterAddons = buildOrder.getTypeCount(ComsatStation) + buildOrder.getTypeCount(NuclearSilo);
        int factoryAddons       = buildOrder.getTypeCount(MachineShop);
        int starportAddons      = buildOrder.getTypeCount(ControlTower);
        int sciAddons           = buildOrder.getTypeCount(PhysicsLab) + buildOrder.getTypeCount(CovertOps);
        
        // add the necessary buildings to make the addons
        buildOrder.add(CommandCenter, commandCenterAddons - numCommandCenters);
        buildOrder.add(Factory, factoryAddons - numFactories);
        buildOrder.add(Starport, starportAddons - numStarports);
        buildOrder.add(ScienceFacility, sciAddons - numSci);
    }

    // Bubble sort the build order so that prerequites always come before what requires them
    buildOrder.sortByPrerequisites();

    // Insert supply buildings so that build order is legal w.r.t. supply counts
    int maxSupply = m_state.getMaxSupply() + m_state.getSupplyInProgress();
    int currentSupply = m_state.getCurrentSupply();

    const ActionType & supplyProvider = ActionTypes::GetSupplyProvider(m_state.getRace());

    BuildOrder finalBuildOrder;
    for (size_t a(0); a < buildOrder.size(); ++a)
    {
        const ActionType & nextAction = buildOrder[a];
        int maxSupply = m_state.getMaxSupply();
        int currentSupply = m_state.getCurrentSupply();
        int supplyInProgress = m_state.getSupplyInProgress();

		// insert 1 or more supply providers if needed
        // TODO: don't go over 200 supply
		while (!nextAction.isMorphed() && !nextAction.isSupplyProvider() && (nextAction.supplyCost() > (maxSupply + supplyInProgress - currentSupply)))
		{
			BOSS_ASSERT(m_state.isLegal(supplyProvider), "Should be able to build more supply here. Max: %d", maxSupply);
			finalBuildOrder.add(supplyProvider);
			m_state.doAction(supplyProvider);

			maxSupply = m_state.getMaxSupply();
			currentSupply = m_state.getCurrentSupply();
			supplyInProgress = m_state.getSupplyInProgress();
		}

		BOSS_ASSERT(m_state.isLegal(nextAction), "Should be able to build the next action now");
		finalBuildOrder.add(nextAction);
		m_state.doAction(nextAction);
	}

    m_buildOrder = finalBuildOrder;
    m_naiveSolved = true;

    return m_buildOrder;
}

