#include "CombatSearch_Bucket.h"

using namespace BOSS;

CombatSearch_Bucket::CombatSearch_Bucket(const CombatSearchParameters p)
    : m_bucket(p.getFrameTimeLimit(), 200)
{
    m_params = p;
   
    BOSS_ASSERT(m_params.getInitialState().getRace() != Races::None, "Combat search initial state is invalid");
}
void CombatSearch_Bucket::recurse(const GameState & state, size_t depth)
{
    if (timeLimitReached())
    {
        throw BOSS_COMBATSEARCH_TIMEOUT;
    }

    updateResults(state);
    m_bucket.update(state, m_buildOrder);

    if (isTerminalNode(state, depth))
    {
        return;
    }

    if (m_bucket.isDominated(state))
    {
        //return;
    }

    ActionSet legalActions;
    generateLegalActions(state, legalActions, m_params);
    
    for (size_t a(0); a < legalActions.size(); ++a)
    {
        GameState child(state);
        child.doAction(legalActions[a]);
        m_buildOrder.add(legalActions[a]);
        
        recurse(child,depth+1);

        m_buildOrder.pop_back();
    }
}

void CombatSearch_Bucket::printResults()
{
    m_bucket.print();
}

#include "BuildOrderPlotter.h"
void CombatSearch_Bucket::writeResultsFile(const std::string & dir, const std::string & filename)
{
    BuildOrderPlotter::WriteGnuPlot(dir + "/" + filename + "_BucketResults", m_bucket.getBucketResultsString(), " with steps");

    // write the final build order data
    BuildOrderPlotter plot;
    plot.setOutputDir(dir);
    plot.addPlot("Bucket", m_params.getInitialState(), m_bucket.getBucket(m_bucket.numBuckets() - 1).buildOrder);
    plot.doPlots();
}