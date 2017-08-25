#include "Eval.h"

namespace BOSS
{
namespace Eval
{
    double ArmyCompletedResourceSum(const GameState & state)
    {
        float sum(0);
	    
	    for (auto & type : ActionTypes::GetAllActionTypes())
	    {
	        if (!type.isBuilding() && !type.isWorker() && !type.isSupplyProvider())
	        {
                sum += state.getNumCompleted(type)*type.mineralPrice();
	            sum += 2*state.getNumCompleted(type)*type.gasPrice();
	        }
	    }
	    
	    return sum;
    }

    double ArmyTotalResourceSum(const GameState & state)
    {
        float sum(0);
	    
	    for (auto & type : ActionTypes::GetAllActionTypes())
	    {
	        if (!type.isBuilding() && !type.isWorker() && !type.isSupplyProvider())
	        {
                sum += state.getNumTotal(type)*type.mineralPrice();
	            sum += 2*state.getNumTotal(type)*type.gasPrice();
	        }
	    }
	    
	    return sum;
    }

    bool BuildOrderBetter(const BuildOrder & buildOrder, const BuildOrder & compareTo)
    {
        size_t numWorkers = 0;
        size_t numWorkersOther = 0;

        for (size_t a(0); a<buildOrder.size(); ++a)
        {
            if (buildOrder[a].isWorker())
            {
                numWorkers++;
            }
        }

        for (size_t a(0); a<compareTo.size(); ++a)
        {
            if (compareTo[a].isWorker())
            {
                numWorkersOther++;
            }
        }

        if (numWorkers == numWorkersOther)
        {
            return buildOrder.size() < compareTo.size();
        }
        else
        {
            return numWorkers > numWorkersOther;
        }
    }

    bool StateDominates(const GameState & state, const GameState & other)
    {
        // we can't define domination for different races
        if (state.getRace() != other.getRace())
        {
            return false;
        }

        // if we have less resources than the other state we are not dominating it
        if ((state.getMinerals() < other.getMinerals()) || (state.getGas() < other.getGas()))
        {
            return false;
        }

        // if we have less of any unit than the other state we are not dominating it
        for (auto & action : ActionTypes::GetAllActionTypes())
        {
            if (state.getNumTotal(action) < other.getNumTotal(action))
            {
                return false;
            }

            if (state.getNumCompleted(action) < other.getNumCompleted(action))
            {
                return false;
            }
        }

        return true;
    }
}
}

