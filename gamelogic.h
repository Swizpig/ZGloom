#pragma once

#include "gloommap.h"
#include "renderer.h"

class GameLogic
{
	public:
		void Init(GloomMap* gmapin, Camera* cam);
		bool Update(Camera* cam);

	private:
		GloomMap* gmap;
		int camdir = 1;

		int32_t FindSegDist(int32_t x, int32_t z, Zone& zone);
		bool Collision(bool event, int32_t x, int32_t z, int32_t r, int32_t& overshoot, int32_t& closestzone);
		bool AdjustPos(int32_t& overshoot, Quick& x, Quick& z, int32_t r, int32_t& closestzone);
		void DoDoor();
		uint32_t animframe[160];
};