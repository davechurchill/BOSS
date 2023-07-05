#include "BuildOrderSearchResults.h"

using namespace BOSS;

BuildOrderSearchResults::BuildOrderSearchResults()
    : solved(false)
    , timedOut(false)
    , solutionFound(false)
    , nodesExpanded(0)
    , timeElapsed(0)
{
}

void BuildOrderSearchResults::printResults(bool pbo) const
{
    printf("%14llu%12.2lf       ",nodesExpanded,timeElapsed);

    if (pbo)
    {
        printBuildOrder();
    }

    printf("\n");
}

void BuildOrderSearchResults::printBuildOrder() const
{
    for (size_t i(0); i<buildOrder.size(); ++i)
    {
        printf("%d ",buildOrder[i].getID());
    }
}