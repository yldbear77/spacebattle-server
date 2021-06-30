#include "../include/Spacecraft.h"

std::unordered_map<uint8_t, std::pair<std::string, uint8_t>> Spacecraft::craftInfo = {
	{Spacecraft::FIGHTER_JET, std::make_pair("Fighter Jet", 2)},
	{Spacecraft::DROPSHIP, std::make_pair("Dropship", 3)},
	{Spacecraft::STEALTH, std::make_pair("Stealth", 4)},
	{Spacecraft::MOTHER_SHIP, std::make_pair("Mothership", 6)},
	{Spacecraft::SPACE_STATION, std::make_pair("Space Station", 8)}
};