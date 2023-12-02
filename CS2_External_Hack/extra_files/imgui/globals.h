#pragma once
#include <vector>
#include <thread>
#include <array>


// globals
namespace g {
	inline bool run = true;
}

// vars
namespace v {
	inline std::pair<bool, std::array<float, 4U>> team_glow{
		false,
		{ 0.f, 0.f, 1.f, 1.f }
	};

	inline std::pair<bool, std::array<float, 4U>> enemy_glow{
		false,
		{ 1.f, 0.f, 1.f, 1.f }
	};

	inline bool radar = false;
}
