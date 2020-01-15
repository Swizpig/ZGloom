#pragma once

#include <cstdint>
#include <vector>

namespace IffHandler
{
	void DecodeIff(uint8_t* indata, std::vector<uint8_t>& outdata, uint32_t &width);
}