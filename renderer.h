#pragma once

#include <vector>
#include "gloommap.h"
#include "gloommaths.h"
#include "quick.h"
#include "objectgraphics.h"

#include <SDL2/SDL.h>

class GloomMap;

class Camera
{
	public:
	Quick x;
	int16_t y;
	Quick z;
	Quick rotquick;
};

class Wall
{
public:
	int16_t wl_lsx; // leftmost screen X
	int16_t wl_rsx; // rightmost screen X
	int16_t wl_nz;	// near Z!
	int16_t wl_fz;	// far Z!
	int16_t wl_lx;	//
	int16_t wl_lz;	//
	int16_t wl_rx;	//
	int16_t wl_rz;	//
	uint16_t wl_a;	//
	uint16_t wl_b;	//
	uint32_t wl_c;	//
	uint16_t wl_sc;	//
	uint16_t wl_open;// 0 = door shut, $4000 = open!
	uint8_t wl_t[8];// textures

	// added this myself
	int16_t len;

	bool valid;
};

class Renderer
{
	public:
		void Init(SDL_Surface* nrendersurface, GloomMap* ngloommap, ObjectGraphics* nObjectGraphics);
		void Render(Camera* Camera);
		void SetTeleEffect(int32_t timer) {fadetimer = timer;};
		void SetPlayerHit(bool hit) { playerhit = hit; };
		void SetThermo(bool t) { thermo = t; };

		//EXPERIMENTAL MULTITHREAD BUILD!
		SDL_Thread* floorthread;
		SDL_Thread* wallthread;
		Camera* camerastash;

		void DrawFloor(Camera* camera);
		void RenderColumns(int32_t xstart, int32_t xinc)
		{
			for (int32_t x = xstart; x < renderwidth; x += xinc)
			{
				ProcessColumn(x, camerastash->y, ceilend, floorstart);
			}
		}

		Renderer();
		~Renderer();
	
	private:
		bool OriginSide(int16_t fx, int16_t fz, int16_t bx, int16_t bz);
		bool PointInFront(int16_t fx, int16_t fz, Wall& z);
		bool Intersect(int16_t t, int16_t x0, int16_t x1) { return (t >= x0) && (t <= x1); };
		void DrawMap();
		int16_t CastColumn(int32_t x, int16_t& zone, Quick& t);
		void DrawColumn(int32_t x, int32_t ystart, int32_t h, Column* texturedata, int32_t z, int32_t palused);
		void DrawCeil(Camera* camera);
		void DrawObjects(Camera* camera);
		void DrawBlood(Camera* camera);
		Column* GetTexColumn(int hitzone, Quick texpos, int& basetexture);
		void ProcessColumn(const uint32_t& x, const int16_t& y, std::vector<int32_t>& ceilend, std::vector<int32_t>& floorstart);

		SDL_Surface* rendersurface;
		GloomMap* gloommap;
		ObjectGraphics* objectgraphics;
		std::vector<Wall> walls;
		int32_t renderwidth = 320;
		int32_t renderheight = 256;
		int32_t halfrenderwidth = 160;
		int32_t halfrenderheight = 128;
		int32_t focmult = 256;
		std::vector<Quick> castgrads;
		std::vector<int32_t> zbuff;
		std::vector<int32_t> ceilend;
		std::vector<int32_t> floorstart;

		// I'm not sure how gloom does screen dimming, I've implemented my own lookup tables
		static const uint32_t darkpalettes[16][16];
		//for fadeout/in
		int32_t fadetimer;
		//for damage indication
		bool playerhit;
		// for thermo glasses effect
		bool thermo;

		// I've split these out to enable inlining better
		inline uint32_t GetDimPalette(int32_t z)
		{
			auto p = z / 128; if (p > 15) p = 15;
			return p;
		}

		inline void ColourModifyFade(const uint8_t& rin, const uint8_t& gin, const uint8_t& bin, uint32_t& col, const int32_t& p)
		{
			int32_t r = darkpalettes[p][rin >> 4];
			int32_t g = darkpalettes[p][gin >> 4];
			int32_t b = darkpalettes[p][bin >> 4];

			
			// gloom deluxe fade to bluish
			r = (r * (25 - fadetimer) + 128 * fadetimer) / 25;
			g = (g * (25 - fadetimer) + 128 * fadetimer) / 25;
			b = (b * (25 - fadetimer) + 255 * fadetimer) / 25;
			
			if (playerhit)
			{
				g = 0; b = 0;
			}

			col = 0xFF000000 | (r << 16) | (g << 8) | b;
		};

		inline void ColourModify(const uint8_t& rin, const uint8_t& gin, const uint8_t& bin, uint32_t& col, const int32_t& p)
		{
			int32_t r = darkpalettes[p][rin >> 4];
			int32_t g = darkpalettes[p][gin >> 4];
			int32_t b = darkpalettes[p][bin >> 4];

			//if (fadetimer)
			//{
			//	// gloom deluxe fade to bluish
			//	r = (r * (25 - fadetimer) + 128 * fadetimer) / 25;
			//	g = (g * (25 - fadetimer) + 128 * fadetimer) / 25;
			//	b = (b * (25 - fadetimer) + 255 * fadetimer) / 25;
			//}
			if (playerhit)
			{
				g = 0; b = 0;
			}

			col = 0xFF000000 | (r << 16) | (g << 8) | b;
		};

		std::list<MapObject> strips;

		// needed for pushing transparent strips
		SDL_mutex* wallmutex;
};
