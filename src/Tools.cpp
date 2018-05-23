#include "Tools.h"
#include "BuildOrderSearchGoal.h"
#include "NaiveBuildOrderSearch.h"
#include "ActionSet.h"

using namespace BOSS;

#include "JSONTools.h"

BuildOrder Tools::GetOptimizedNaiveBuildOrderOld(const GameState & state, const BuildOrderSearchGoal & goal)
{
    BuildOrder bestBuildOrder = GetNaiveBuildOrderAddWorkersOld(state, goal, 4);
    int minCompletionTime = Tools::GetBuildOrderCompletionTime(state, bestBuildOrder);
    size_t bestNumWorkers = bestBuildOrder.getTypeCount(ActionTypes::GetWorker(state.getRace()));

    for (size_t numWorkers(8); numWorkers < 27; ++numWorkers)
    {
        BuildOrder buildOrder = Tools::GetNaiveBuildOrderAddWorkersOld(state, goal, numWorkers);
        int completionTime = Tools::GetBuildOrderCompletionTime(state, buildOrder);
        size_t workers = buildOrder.getTypeCount(ActionTypes::GetWorker(state.getRace()));
        
        if (completionTime <= minCompletionTime + ((int)(workers-bestNumWorkers)*24))
        {
            minCompletionTime = completionTime;
            bestBuildOrder = buildOrder;
        }
    }

    
    int bestCompletionTime = Tools::GetBuildOrderCompletionTime(state, bestBuildOrder);
    BuildOrder testBuildOrder;

    //std::cout << "Found a better build order that takes " << bestCompletionTime << " frames\n";
    while (true)
    {
        const static ActionType gateway = ActionTypes::GetActionType("Protoss_Gateway");
        InsertActionIntoBuildOrder(testBuildOrder, bestBuildOrder, state, gateway);

        int completionTime = Tools::GetBuildOrderCompletionTime(state, testBuildOrder);

        if (completionTime < bestCompletionTime)
        {
            //std::cout << "Found a better build order that takes " << completionTime << " frames\n";
            bestCompletionTime = completionTime;
            bestBuildOrder = testBuildOrder;
        }
        else
        {
            break;
        }
    }


    return bestBuildOrder;
}

BuildOrder Tools::GetNaiveBuildOrderAddWorkersOld(const GameState & state, const BuildOrderSearchGoal & goal, size_t maxWorkers)
{
    ActionSet wanted;
    int minWorkers = 8;

    const ActionType & worker = ActionTypes::GetWorker(state.getRace());
    std::vector<size_t> buildOrderActionTypeCount(ActionTypes::GetAllActionTypes().size(), 0);

    // add everything from the goal to the needed set
    for (size_t a(0); a < ActionTypes::GetAllActionTypes().size(); ++a)
    {
        const ActionType & actionType(a);
        size_t numCompleted = state.getNumTotal(actionType);
            
        if (goal.getGoal(actionType) > numCompleted)
        {
            wanted.add(actionType);
        }
    }

    if (wanted.size() == 0)
    {
        return BuildOrder();
    }

    // Calculate which prerequisite units we need to build to achieve the units we want from the goal
    ActionSet requiredToBuild;
    CalculatePrerequisitesRequiredToBuild(state, wanted, requiredToBuild);

    // Add the required units to a preliminary build order
    BuildOrder buildOrder;
    for (size_t a(0); a < requiredToBuild.size(); ++a)
    {
        const ActionType & type = requiredToBuild[a];
        buildOrder.add(type);
        buildOrderActionTypeCount[type.getID()]++;
    }

    // Add some workers to the build order if we don't have many, this usually gives a lower upper bound
    int requiredWorkers = minWorkers - state.getNumCompleted(ActionTypes::GetWorker(state.getRace()));
    while (requiredWorkers-- > 0)
    {
        buildOrder.add(worker);
        buildOrderActionTypeCount[worker.getID()]++;
    }

    // Add the goal units to the end of the build order 
    for (size_t a(0); a < ActionTypes::GetAllActionTypes().size(); ++a)
    {
        const ActionType & actionType(a);
        int need = (int)goal.getGoal(actionType);
        int have = (int)state.getNumTotal(actionType);
        int numNeeded = need - have - buildOrderActionTypeCount[actionType.getID()]; 
            
        for (int i(0); i < numNeeded; ++i)
        {
            buildOrder.add(actionType);
        }
    }

    
    static const ActionType commandCenter = ActionTypes::GetActionType("CommandCenter");
    static const ActionType factory = ActionTypes::GetActionType("Factory");
    static const ActionType starport = ActionTypes::GetActionType("Starport");
    static const ActionType scienceFacility = ActionTypes::GetActionType("ScienceFacility");


    // Check to see if we have enough buildings for the required addons
    if (state.getRace() == Races::Terran)
    {
        int commandCenterAddons = 0;
        int factoryAddons = 0;
        int starportAddons = 0;
        int sciAddons = 0;

        int numCommandCenters = state.getNumTotal(commandCenter);
        int numFactories = state.getNumTotal(factory);
        int numStarports = state.getNumTotal(starport);
        int numSci = state.getNumTotal(scienceFacility);
        
        for (size_t a(0); a < buildOrder.size(); ++a)
        {
            const ActionType & actionType = buildOrder[a];

            if (actionType.isAddon())
            {
                if (actionType.whatBuilds() == commandCenter)
                {
                    ++commandCenterAddons;
                }
                else if (actionType.whatBuilds() == factory)
                {
                    ++factoryAddons;
                }
                else if (actionType.whatBuilds() == starport)
                {
                    ++starportAddons;
                }
                else if (actionType.whatBuilds() == scienceFacility)
                {
                    ++sciAddons;
                }
                else
                {
                    BOSS_ASSERT(false, "Addon has no builder: %s %s", actionType.getName().c_str(), actionType.whatBuilds().getName().c_str());
                }
            }

            if (actionType == commandCenter)
            {
                ++numCommandCenters;
            }
            else if (actionType == factory)
            {
                ++numFactories;
            }
            else if (actionType == starport)
            {
                ++numStarports;
            }
            else if (actionType == scienceFacility)
            {
                ++numSci;
            }
        }

        // add the necessary buildings to make the addons
        for (int n(0); n < commandCenterAddons - numCommandCenters; ++n)
        {
            buildOrder.add(commandCenter);
        }

        for (int n(0); n < factoryAddons - numFactories; ++n)
        {
            buildOrder.add(factory);
        }

        for (int n(0); n < starportAddons - numStarports; ++n)
        {
            buildOrder.add(starport);
        }
        for (int n(0); n < sciAddons - numSci; ++n)
        {
            buildOrder.add(scienceFacility);
        }

    }

    // Bubble sort the build order so that prerequites always come before what requires them
    for (size_t i(0); i < buildOrder.size()-1; ++i)
    {
        for (size_t j(i+1); j < buildOrder.size(); ++j)
        {
            const ActionSet & recursivePre = buildOrder[i].getRecursivePrerequisiteActionCount();

            if (recursivePre.contains(buildOrder[j]))
            {
                std::swap(buildOrder[i], buildOrder[j]);
            }
        }
    }

    // finish the build order with workers and supply
    BuildOrder finalBuildOrder;
    GameState currentState(state);
    size_t i = 0;
    while (i < buildOrder.size())
    {
        const ActionType & worker           = ActionTypes::GetWorker(currentState.getRace());
        const ActionType & supplyProvider   = ActionTypes::GetSupplyProvider(currentState.getRace());
        const ActionType & nextAction       = buildOrder[i];
        size_t maxSupply             = currentState.getMaxSupply() + currentState.getSupplyInProgress();
        size_t numWorkers            = currentState.getNumTotal(worker);
        size_t currentSupply         = currentState.getCurrentSupply();

        if (numWorkers < 8)
        {
            finalBuildOrder.add(worker);
            currentState.doAction(worker);
            continue;
        }

        // insert a supply provider if we are behind
        int surplusSupply = maxSupply - currentSupply;
		if (surplusSupply < nextAction.supplyCost() + 2)
		{
			try
			{
				BOSS_ASSERT(currentState.isLegal(supplyProvider), "supplyProvider should be legal");
				finalBuildOrder.add(supplyProvider);
				currentState.doAction(supplyProvider);
				continue;
			}
			catch (BOSSException e)
			{
				break;
			}

		}

       
        int whenWorkerReady      = currentState.whenCanBuild(worker);
        int whennextActionReady  = currentState.whenCanBuild(nextAction);

        if ((numWorkers < maxWorkers) && (whenWorkerReady < whennextActionReady))
        {
			// check to see if we should insert a worker
			try
			{
				BOSS_ASSERT(currentState.isLegal(worker), "Worker should be legal");
				finalBuildOrder.add(worker);
				currentState.doAction(worker);
			}
			catch (BOSSException)
			{
			}
			continue;
		}
		else
		{
			ActionType testNextAction = buildOrder[i];
			BOSS_ASSERT(currentState.isLegal(nextAction), "nextAction should be legal");
			finalBuildOrder.add(nextAction);
			currentState.doAction(nextAction);
			++i;
		}
    }

    return finalBuildOrder;
}

void Tools::InsertActionIntoBuildOrder(BuildOrder & result, const BuildOrder & buildOrder, const GameState & initialState, const ActionType & action)
{
    int bestInsertIndex = -1;
    BuildOrder runningBuildOrder;
    GameState runningState(initialState);
    int minCompletionTime = Tools::GetBuildOrderCompletionTime(initialState, buildOrder);

    BuildOrder testBuildOrder = buildOrder;

    for (size_t insertIndex(0); insertIndex < buildOrder.size(); ++insertIndex)
    {
        // if we can test the action here, do it
        if (runningState.isLegal(action))
        {
            // figure out the build time of build order with action inserted here
            GameState tempState(runningState);
            tempState.doAction(action);
            for (size_t a(insertIndex); a < buildOrder.size(); ++a)
            {
                tempState.doAction(buildOrder[a]);
            }

            int completionTime = tempState.getLastActionFinishTime();

            if (completionTime < minCompletionTime)
            {
                minCompletionTime = completionTime;
                bestInsertIndex = insertIndex;
            }
        }

        BOSS_ASSERT(runningState.isLegal(buildOrder[insertIndex]), "We have made the next action illegal somehow");
        runningBuildOrder.add(buildOrder[insertIndex]);
        runningState.doAction(buildOrder[insertIndex]);
    }

    result.clear();
    for (size_t a(0); a<buildOrder.size(); ++a)
    {
        if (bestInsertIndex == a)
        {
            result.add(action);
        }

        result.add(buildOrder[a]);
    }
}

int Tools::GetUpperBound(const GameState & state, const BuildOrderSearchGoal & goal)
{
    NaiveBuildOrderSearch naiveSearch(state, goal);
    const BuildOrder & naiveBuildOrder = naiveSearch.solve();
    int upperBound = Tools::GetBuildOrderCompletionTime(state, naiveBuildOrder);

    return upperBound;
}

int Tools::GetLowerBound(const GameState & state, const BuildOrderSearchGoal & goal)
{
    ActionSet wanted;

    // add everything from the goal to the needed set
    for (size_t a(0); a < ActionTypes::GetAllActionTypes().size(); ++a)
    {
        const ActionType & actionType(a);
        size_t numCompleted = state.getNumTotal(actionType);
            
        if (goal.getGoal(actionType) > numCompleted)
        {
            wanted.add(actionType);
        }
    }

    int lowerBound = Tools::CalculatePrerequisitesLowerBound(state, wanted, 0);

    return lowerBound;
}

void Tools::CalculatePrerequisitesRequiredToBuild(const GameState & state, const ActionSet & needed, ActionSet & added)
{
    // if anything needed gas and we don't have a refinery, we need to add one
    ActionSet allNeeded(needed);
    const ActionType & refinery = ActionTypes::GetRefinery(state.getRace());
    if (!needed.contains(refinery) && (state.getNumCompleted(refinery) == 0) && !added.contains(refinery))
    {
        for (size_t n(0); n<needed.size(); ++n)
        {
            const ActionType & actionType = needed[n];
            if (actionType.gasPrice() > 0)
            {
                allNeeded.add(refinery);
                break;
            }
        }
    }

    for (size_t n(0); n<allNeeded.size(); ++n)
    {
        const ActionType & neededType = allNeeded[n];

        // if we already have the needed type completed we can skip it
        if (added.contains(neededType) || state.getNumCompleted(neededType) > 0)
        {
            
        }
        // if we have the needed type in progress we can add that time
        else if (state.getNumInProgress(neededType) > 0)
        {
            //added.add(neededType);
        }
        // otherwise we need to recurse on the needed type to build its prerequisites
        else
        {
            added.add(neededType);
            CalculatePrerequisitesRequiredToBuild(state, neededType.getPrerequisiteActionCount(), added);
        }
    }
}

// returns the amount of time necessary to complete the longest chain of sequential prerequisites
int Tools::CalculatePrerequisitesLowerBound(const GameState & state, const ActionSet & needed, int timeSoFar, int depth)
{
    int max = 0;
    for (size_t n(0); n<needed.size(); ++n)
    {
        const ActionType & neededType = needed[n];

        int thisActionTime = 0;

        // if we already have the needed type completed we can skip it
        if (state.getNumCompleted(neededType) > 0)
        {
            thisActionTime = timeSoFar;
        }
        // if we have the needed type in progress we can add that time
        else if (state.getNumInProgress(neededType) > 0)
        {
            thisActionTime = timeSoFar + state.getNextFinishTime(neededType) - state.getCurrentFrame();
        }
        // otherwise we need to recurse on the needed type to build its prerequisites
        else
        {
            /*for (int i=0; i<depth; ++i)
            {
                std::cout << "    ";
            }
            std::cout << neededType.getName() << " " << neededType.buildTime() << " " << timeSoFar << std::endl;*/
            thisActionTime = CalculatePrerequisitesLowerBound(state, neededType.getPrerequisiteActionCount(), timeSoFar + neededType.buildTime(), depth + 1);
        }

        if (thisActionTime > max)
        {
            max = thisActionTime;
        }
    }

    return max;
}

int Tools::GetBuildOrderCompletionTime(const GameState & state, const BuildOrder & buildOrder)
{
    GameState stateCopy(state);
    for (size_t i(0); i < buildOrder.size(); ++i)
    {
        if (!stateCopy.isLegal(buildOrder[i]))
        {
            std::cerr << "WARNING: Trying to get completion time of illegal build order" << std::endl;
        }

        stateCopy.doAction(buildOrder[i]);
    }

    return stateCopy.getLastActionFinishTime();
}