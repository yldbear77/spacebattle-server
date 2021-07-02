#include "../include/Spacecraft.h"


std::unordered_map<uint8_t, std::pair<std::string, uint8_t>> Spacecraft::craftInfo = {
	{Spacecraft::FIGHTER_JET, std::make_pair("Fighter Jet", 2)},
	{Spacecraft::DROPSHIP, std::make_pair("Dropship", 3)},
	{Spacecraft::STEALTH, std::make_pair("Stealth", 4)},
	{Spacecraft::MOTHER_SHIP, std::make_pair("Mothership", 6)},
	{Spacecraft::SPACE_STATION, std::make_pair("Space Station", 8)}
};


std::unordered_map<uint8_t, std::vector<std::vector<std::pair<int, int>>>> Spacecraft::deployRule = {
	{
		Spacecraft::FIGHTER_JET, {
			{{0, 0}, {0, 1}},
			{{0, 0}, {0, -1}},
			{{0, 0}, {-1, 0}},
			{{0, 0}, {1, 0}}
		}
	},
	{
		Spacecraft::DROPSHIP, {
			{{0, 0}, {0, 1}, {0, 2}},
			{{0, 0}, {0, -1}, {0, -2}},
			{{0, 0}, {-1, 0}, {-2, 0}},
			{{0, 0}, {1, 0}, {2, 0}}
		}
	},
	{
		Spacecraft::STEALTH, {
			{{0, 0}, {0, 1}, {0, 2}, {0, 3}},
			{{0, 0}, {0, -1}, {0, -2}, {0, -3}},
			{{0, 0}, {-1, 0}, {-2, 0}, {-3, 0}},
			{{0, 0}, {1, 0}, {2, 0}, {3, 0}}
		}
	},
	{
		Spacecraft::MOTHER_SHIP, {
			{{0, 0}, {1, 0}, {0, 1}, {1, 1}, {0, 2}, {1, 2}},
			{{0, 0}, {0, 1}, {-1, 0}, {-1, 1}, {-2, 0}, {-2, 1}},
			{{0, 0}, {-1, 0}, {0, -1}, {-1, -1}, {0, -2}, {-1, -2}},
			{{0, 0}, {0, -1}, {1, 0}, {1, -1}, {2, 0}, {2, -1}}
		}
	},
	{
		Spacecraft::SPACE_STATION, {
			{{0, 0}, {1, 0}, {0, 1}, {1, 1}, {0, 2}, {1, 2}, {0, 3}, {1, 3}},
			{{0, 0}, {0, 1}, {-1, 0}, {-1, 1}, {-2, 0}, {-2, 1}, {-3, 0}, {-3, 1}},
			{{0, 0}, {-1, 0}, {0, -1}, {-1, -1}, {0, -2}, {-1, -2}, {0, -3}, {-1, -3}},
			{{0, 0}, {0, -1}, {1, 0}, {1, -1}, {2, 0}, {2, -1}, {3, 0}, {3, -1}}
		}
	}
};