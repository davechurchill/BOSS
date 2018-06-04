#include "CombatSearchExperiment.h"
#include "CombatSearch.h"
#include "CombatSearch_Bucket.h"
#include "CombatSearch_Integral.h"
#include "CombatSearch_BestResponse.h"
#include "FileTools.h"

using namespace BOSS;

CombatSearchExperiment::CombatSearchExperiment()
    : m_race(Races::None)
{

}

CombatSearchExperiment::CombatSearchExperiment(const std::string & name, const json & val)
    : m_race(Races::None)
    , m_name(name)
{
    BOSS_ASSERT(val.count("SearchTypes") && val["SearchTypes"].is_array(), "CombatSearchExperiment must have a 'SearchTypes' array");
    for (size_t i(0); i < val["SearchTypes"].size(); ++i)
    {
        BOSS_ASSERT(val["SearchTypes"][i].is_string(), "searchTypes element is not a string");
        
        m_searchTypes.push_back(val["SearchTypes"][i]);
    }

    BOSS_ASSERT(val.count("Race") && val["Race"].is_string(), "CombatSearchExperiment must have a 'Race' string");
    m_race = Races::GetRaceID(val["Race"]);

    BOSS_ASSERT(val.count("OutputDir") && val["OutputDir"].is_string(), "CombatSearchExperiment must have an 'OutputDir' string");
    m_outputDir = val["OutputDir"].get<std::string>();

    BOSS_ASSERT(val.count("State") && val["State"].is_string(), "CombatSearchExperiment must have a 'State' string");
    m_params.setInitialState(BOSSConfig::Instance().GetState(val["State"]));

    BOSS_ASSERT(val.count("FrameTimeLimit") && val["FrameTimeLimit"].is_number_integer(), "CombatSearchExperiment must have a 'FrameTimeLimit' int");
    m_params.setFrameTimeLimit(val["FrameTimeLimit"]);

    BOSS_ASSERT(val.count("SearchTimeLimitMS") && val["SearchTimeLimitMS"].is_number_integer(), "CombatSearchExperiment must have a 'SearchTimeLimitMS' int");
    m_params.setSearchTimeLimit(val["SearchTimeLimitMS"]);

    if (val.count("MaxActions"))
    {
        const json & maxActions = val["MaxActions"];
        BOSS_ASSERT(maxActions.is_array(), "MaxActions is not an array");

        for (size_t i(0); i < maxActions.size(); ++i)
        {
            BOSS_ASSERT(maxActions[i].is_array(), "MaxActions element must be array of size 2");

            BOSS_ASSERT(maxActions[i].size() == 2 && maxActions[i][0u].is_string() && maxActions[i][1u].is_number_integer(), "MaxActions element must be [\"Action\", Count]");

            const std::string & typeName = maxActions[i][0u];

            BOSS_ASSERT(ActionTypes::TypeExists(typeName), "Action Type doesn't exist: %s", typeName.c_str());

            m_params.setMaxActions(ActionTypes::GetActionType(typeName), maxActions[i][1]);
        }
    }

    if (val.count("RelevantActions"))
    {
        const json & relevantActions = val["RelevantActions"];
        BOSS_ASSERT(relevantActions.is_array(), "RelevantActions is not an array");

        ActionSet relevantActionSet;

        for (size_t i(0); i < relevantActions.size(); ++i)
        {
            BOSS_ASSERT(relevantActions[i].is_string(), "RelvantActions element must be action type string");
            BOSS_ASSERT(ActionTypes::TypeExists(relevantActions[i]), "Action Type doesn't exist: %s", relevantActions[i]);
            
            relevantActionSet.add(ActionTypes::GetActionType(relevantActions[i]));
        }

        m_params.setRelevantActions(relevantActionSet);
    }

    if (val.count("AlwaysMakeWorkers"))
    {
        BOSS_ASSERT(val["AlwaysMakeWorkers"].is_boolean(), "AlwaysMakeWorkers should be a bool");

        m_params.setAlwaysMakeWorkers(val["AlwaysMakeWorkers"]);
    }

    if (val.count("OpeningBuildOrder"))
    {
        BOSS_ASSERT(val["OpeningBuildOrder"].is_string(), "OpeningBuildOrder should be a string");
        m_params.setOpeningBuildOrder(BOSSConfig::Instance().GetBuildOrder(val["OpeningBuildOrder"]));
    }

    if (val.count("BestResponseParams"))
    {
        const json & brVal = val["BestResponseParams"];

        BOSS_ASSERT(brVal.is_object(), "BestResponseParams not an object");
        BOSS_ASSERT(brVal.count("EnemyState"), "bestResponseParams must have 'enemyState' string");
        BOSS_ASSERT(brVal.count("EnemyBuildOrder"), "bestResponseParams must have 'enemyBuildOrder' string");

        BOSS_ASSERT(brVal.count("EnemyState") && brVal["EnemyState"].is_string(), "bestResponseParams must have a 'EnemyState' string");
        m_params.setEnemyInitialState(BOSSConfig::Instance().GetState(brVal["EnemyState"]));

        BOSS_ASSERT(brVal.count("EnemyBuildOrder") && brVal["EnemyBuildOrder"].is_string(), "BestResponseParams must have a 'EnemyBuildOrder' string");
        m_params.setEnemyBuildOrder(BOSSConfig::Instance().GetBuildOrder(brVal["EnemyBuildOrder"]));
    }
}

void CombatSearchExperiment::run()
{
    m_outputDir = "results/" + Assert::CurrentDateTime();

    FileTools::MakeDirectory(m_outputDir);

    static std::string stars = "************************************************";
    for (size_t i(0); i < m_searchTypes.size(); ++i)
    {
        std::shared_ptr<CombatSearch> combatSearch;
        std::string resultsFile = m_name;

        std::cout << "\n" << stars << "\n* Running Experiment: " << m_name << " [" << m_searchTypes[i] << "]\n" << stars << "\n";

        if (m_searchTypes[i] == "Integral")
        {
            combatSearch = std::shared_ptr<CombatSearch>(new CombatSearch_Integral(m_params));
            resultsFile += "_Integral"; 
        }
        else if (m_searchTypes[i] == "Bucket")
        {
            combatSearch = std::shared_ptr<CombatSearch>(new CombatSearch_Bucket(m_params));
            resultsFile += "_Bucket"; 
        }
        else if (m_searchTypes[i] == "BestResponse")
        {
            combatSearch = std::shared_ptr<CombatSearch>(new CombatSearch_BestResponse(m_params));
            resultsFile += "_BestResponse"; 
        }
        else
        {
            BOSS_ASSERT(false, "CombatSearch type not found: %s", m_searchTypes[i].c_str());
        }

        combatSearch->search();
        combatSearch->printResults();
        combatSearch->writeResultsFile(m_outputDir, resultsFile);
        const CombatSearchResults & results = combatSearch->getResults();
        std::cout << "\nSearched " << results.nodesExpanded << " nodes in " << results.timeElapsed << "ms @ " << (1000.0*results.nodesExpanded/results.timeElapsed) << " nodes/sec\n\n";
    }
}