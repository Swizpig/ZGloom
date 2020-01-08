#pragma once

#include "gloommap.h"
#include "renderer.h"

class Camera;
class GloomMap;

class GameLogic
{
	public:
		void Init(GloomMap* gmapin, Camera* cam);
		bool Update(Camera* cam);

		// needed public for monster logic

		bool Collision(bool event, int32_t x, int32_t z, int32_t r, int32_t& overshoot, int32_t& closestzone);
		bool AdjustPos(int32_t& overshoot, Quick& x, Quick& z, int32_t r, int32_t& closestzone);

	private:
		GloomMap* gmap;
		int camdir = 1;

		int32_t FindSegDist(int32_t x, int32_t z, Zone& zone);
		void DoDoor();
		void DoRot();
		void Rotter(int16_t x, int16_t z, int16_t&nx, int16_t& nz, int16_t camrots[4]);
		uint32_t animframe[160];
		bool eventhit[25];
};