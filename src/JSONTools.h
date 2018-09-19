#pragma once

#include "Common.h"
#include "BuildOrder.h"
#include "BuildOrderSearchGoal.h"

namespace BOSS
{
namespace JSONTools
{
    template <class T>
    void ReadInt(const char * key, const json & j, T & dest)
    {
        if (j.count(key))
        {
            BOSS_ASSERT(j[key].is_number_integer(), "%s should be an int", key);
            dest = (T)j[key];
        }
    }

	template <class T>
	void ReadFloat(const char * key, const json & j, T & dest) 
	{
		if (j.count(key))
		{
			BOSS_ASSERT(j[key].is_number_float(), "%s should be a float", key);
			dest = (T)j[key];
		}
	}
    
    void ReadBool(const char * key, const json & json, bool & dest);
    void ReadString(const char * key, const json & json, std::string & dest);

    std::string ReadFile(const std::string & filename);

    GameState GetGameState(const json & json);
    BuildOrder GetBuildOrder(const json & json);
    BuildOrderSearchGoal GetBuildOrderSearchGoal(const json & json);
    
    std::string GetBuildOrderString(const std::vector<ActionType> & buildOrder);
}
}
