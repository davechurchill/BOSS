#pragma once

#include <stdio.h>
#include <math.h>
#include <fstream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <limits>
#include <map>
#include <set>
#include <algorithm>
#include "json/json.hpp"

using json = nlohmann::json;

#include "BOSSAssert.h"

namespace BOSS
{
    typedef size_t  ActionID;
    typedef size_t  RaceID;

	// constants declared in data file
	class CONSTANTS
	{
	public:
		static double MPWPF;				// minerals per worker per frame
		static double GPWPF;				// gas per worker per frame
		static double ERPF;				// energy regen per frame
		static double HRPF;				// health regen per frame
		static double SRPF;				// shield regen per frame
		static int WorkersPerRefinery;
	};

    namespace Races
    {
        enum {Protoss, Terran, Zerg, NUM_RACES, None};

        RaceID GetRaceID(const std::string & race);
        std::string GetRaceName(RaceID race);
    }
}