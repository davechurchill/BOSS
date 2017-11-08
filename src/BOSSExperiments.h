#pragma once

#include "Common.h"
#include "JSONTools.h"

namespace BOSS
{

namespace Experiments
{
    void RunExperiments(const std::string & experimentFilename);

    //void RunCombatExperiment(const std::string & name, const rapidjson::Value & val);
    void RunBuildOrderPlot(const std::string & name, const json & j);
}

}