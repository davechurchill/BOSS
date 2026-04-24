#pragma once

#include "BOSS.h"
#include "JSONTools.h"
#include "BuildOrderSearchGoal.h"
#include "GameState.h"
#include "ActionType.h"
#include "DFBB_BuildOrderSearchResults.h"
#include "ActionOrdering.hpp"
#include "BuildOrderPlotter.h"

namespace BOSS
{

class DFBBSearchExperiment
{
    struct RunData
    {
        std::string                  label;
        DFBB_BuildOrderSearchResults results;
    };

    std::string             m_name;
    std::string             m_outputDir;
    std::string             m_htmlFile;
    GameState               m_initialState;
    BuildOrderSearchGoal    m_goal;
    int                     m_searchTimeLimitMS;
    bool                    m_printNewBest;
    bool                    m_smartSearch;
    std::vector<ActionOrderingType> m_orderings;
    bool                    m_useRepetitions;
    bool                    m_useIncreasingRepetitions;
    bool                    m_useLandmarkLowerBound;
    bool                    m_useResourceLowerBound;
    bool                    m_useAlwaysMakeWorkers;
    bool                    m_useSupplyBounding;
    double                  m_supplyBoundingThreshold;
    std::vector<ActionType> m_relevantActions;

    void printResults(const DFBB_BuildOrderSearchResults & results) const;
    void writeHTMLFile(const std::vector<RunData> & runs) const;

public:

    DFBBSearchExperiment();
    DFBBSearchExperiment(const std::string & name, const json & val);

    void run();
};
}
