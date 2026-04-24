#pragma once

#include "BOSS.h"
#include "BuildOrderSearchGoal.h"
#include "DFBB_BuildOrderSearchResults.h"
#include "GameState.h"
#include "JSONTools.h"

namespace BOSS
{

class MonteCarloTreeSearchExperiment
{
    struct RunData
    {
        std::string                  label;
        DFBB_BuildOrderSearchResults results;
    };

    std::string             m_name;
    std::string             m_outputFile;
    std::string             m_htmlFile;
    GameState               m_initialState;
    BuildOrderSearchGoal    m_goal;
    int                     m_searchTimeLimitMS;
    size_t                  m_iterations;
    double                  m_explorationConstant;
    bool                    m_printNewBest;
    bool                    m_useRepetitions;
    bool                    m_useIncreasingRepetitions;
    bool                    m_useLandmarkLowerBound;
    bool                    m_useResourceLowerBound;
    bool                    m_useAlwaysMakeWorkers;
    bool                    m_useSupplyBounding;
    double                  m_supplyBoundingThreshold;

    void printResults(const DFBB_BuildOrderSearchResults & results) const;
    void writeResultsFile(const DFBB_BuildOrderSearchResults & results) const;
    void writeHTMLFile(const std::vector<RunData> & runs) const;

public:
    MonteCarloTreeSearchExperiment();
    MonteCarloTreeSearchExperiment(const std::string & name, const json & val);

    void run();
};

}
