#include "BOSS.h"
#include "BOSSExperiments.h"

using namespace BOSS;

int main(int argc, char *argv[])
{
    // Initialize all the BOSS internal data
    BOSS::Init("config/BWData.json");

    // Read in the config parameters that will be used for experiments
    BOSS::BOSSConfig::Instance().ParseConfig("config/BOSS_Config.txt");

    BOSS::Experiments::RunExperiments("config/BOSS_Config.txt");

    return 0;
}
