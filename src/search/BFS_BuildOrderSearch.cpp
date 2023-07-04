#include "BFS_BuildOrderSearch.h"
#include <queue>

void BFS_BuildOrderSearch::search()
{
    m_searchTimer.start();
    struct Node
    {
        BOSS::GameState state;
        int depth;
        std::vector<ActionType> buildOrder;
    };
    std::queue<Node> openList;
    openList.push(Node{ m_initialState, 0, {} });

    std::vector<ActionType> legal;
    auto dependancies = findLooseDependancies();

    while (true) {

        // timeout
        if (m_searchTimeLimit && m_searchTimer.getElapsedTimeInMilliSec() > m_searchTimeLimit)
        {
            m_results.timedOut = true;
            break;
        }

        Node current = openList.front();
        openList.pop();

        // Completion condition
        if (m_goal.isAchievedBy(current.state))
        {
            // Add actions to build order
            for (auto& act : current.buildOrder)
            {
                m_results.buildOrder.add(act);
            }
            break;
        }

        // expand node
        current.state.getLegalActions(legal);
        for (auto& act : legal)
        {
            // Domain specific pruning; all other actions cannot improve build order
            if (dependancies.contains(act) || act.isWorker() || act.isSupplyProvider() || act.isHatchery())
            {
                Node child = current;
                child.depth++;
                child.state.doAction(act);
                child.buildOrder.push_back(act);
                openList.push(child);
            }
        }
        m_results.nodesExpanded++;


    }
    m_results.solved = !m_results.timedOut;
    m_results.timeElapsed = m_searchTimer.getElapsedTimeInMilliSec();
}