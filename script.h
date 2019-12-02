#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Script
{
	public:
		Script();
		uint32_t numlines;
		std::vector<std::string> lines;
};