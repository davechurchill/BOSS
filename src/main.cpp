#include "BOSS.h"
#include "BOSSExperiments.h"

using namespace BOSS;

int main(int argc, char *argv[])
{
    // Initialize all the BOSS internal data
    BOSS::Init("BWData.json");

    // Read in the config parameters that will be used for experiments
    BOSS::BOSSConfig::Instance().ParseConfig("BOSS_Config.txt");

    BOSS::Experiments::RunExperiments("BOSS_Config.txt");

    return 0;
}
