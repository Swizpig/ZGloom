#pragma once

#include "quick.h"

namespace GloomMaths
{
	void GetCamRot(uint8_t ang, Quick result[4]);
	void GetCamRotRaw(uint8_t ang, int16_t result[4]);
	void GetCamRot2Raw(uint16_t ang, int16_t result[4]);
	uint8_t CalcAngle(int32_t x1, int32_t z1, int32_t x2, int32_t z2);
	void SeedRnd(uint16_t seed);
	uint16_t RndW();
	uint16_t RndN(int16_t mult);
	void CalcNormVec(int16_t x, int16_t y, int16_t& xr, int16_t& yr);
}