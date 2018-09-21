#include "BOSS.h"
#include "ActionType.h"
#include "ActionTypeData.h"

namespace BOSS
{
    bool isInit = false;

    void Init(const std::string & filename)
    {
        if (!isInit)
        {
			// parse the JSON file and report an error if the parsing failed
			std::ifstream file(filename);
			json j;
			file >> j;

			bool parsingFailed = false;
			if (parsingFailed)
			{
				std::cerr << "Error: Config File Found, but could not be parsed\n";
				std::cerr << "Config Filename: " << filename << "\n";
				std::cerr << "The bot will not run without its configuration file\n";
				std::cerr << "Please check that the file exists, is not empty, and is valid JSON. Incomplete paths are relative to the BOSS .exe file\n";
				return;
			}

			// initialize constants and action data from the file
			InitializeConstants(j);
            ActionTypeData::Init(j);
            ActionTypes::Init();
            isInit = true;
        }
    }

	void InitializeConstants(const json & j)
	{
		BOSS_ASSERT(j.count("WorkersPerRefinery"), "No 'WorkersPerRefinery' member");
		BOSS_ASSERT(j["WorkersPerRefinery"].is_number_integer(), "WorkersPerRefinery must be an integer");

		BOSS_ASSERT(j.count("MineralsPerWorkerPerFrame"), "No 'MineralsPerWorkerPerFrame' member");
		BOSS_ASSERT(j["MineralsPerWorkerPerFrame"].is_number_float(), "MineralsPerWorkerPerFrame must be a float");

		BOSS_ASSERT(j.count("GasPerWorkerPerFrame"), "No 'GasPerWorkerPerFrame' member");
		BOSS_ASSERT(j["GasPerWorkerPerFrame"].is_number_float(), "GasPerWorkerPerFrame must be a float");

		BOSS_ASSERT(j.count("EnergyRegenPerFrame"), "No 'EnergyRegenPerFrame' member");
		BOSS_ASSERT(j["EnergyRegenPerFrame"].is_number_float(), "EnergyRegenPerFrame must be a float");

		BOSS_ASSERT(j.count("HealthRegenPerFrame"), "No 'HealthRegenPerFrame' member");
		BOSS_ASSERT(j["HealthRegenPerFrame"].is_number_float(), "HealthRegenPerFrame must be a float");

		BOSS_ASSERT(j.count("ShieldRegenPerFrame"), "No 'ShieldRegenPerFrame' member");
		BOSS_ASSERT(j["ShieldRegenPerFrame"].is_number_float(), "ShieldRegenPerFrame must be a float");

		
		CONSTANTS::WorkersPerRefinery = j["WorkersPerRefinery"];
		CONSTANTS::MPWPF = j["MineralsPerWorkerPerFrame"];
		CONSTANTS::GPWPF = j["GasPerWorkerPerFrame"];
		CONSTANTS::ERPF = j["EnergyRegenPerFrame"];
		CONSTANTS::HRPF = j["HealthRegenPerFrame"];
		CONSTANTS::SRPF = j["ShieldRegenPerFrame"];
	}
}