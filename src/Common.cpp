#pragma once

#include "Common.h"

using namespace BOSS;

// defining static variables
double CONSTANTS::MPWPF;
double CONSTANTS::GPWPF;
double CONSTANTS::ERPF;
double CONSTANTS::HRPF;
double CONSTANTS::SRPF;
int CONSTANTS::WorkersPerRefinery;

RaceID Races::GetRaceID(const std::string & race)
{
    if      (race == "Protoss") { return Races::Protoss; }
    else if (race == "Terran")  { return Races::Terran; }
    else if (race == "Zerg")    { return Races::Zerg; }
    else                        { return Races::None; }
}

std::string Races::GetRaceName(RaceID race)
{
    if      (race == Races::Protoss)    { return "Protoss"; }
    else if (race == Races::Terran)     { return "Terran"; }
    else if (race == Races::Zerg)       { return "Zerg"; }
    else                                { return "None"; }
}