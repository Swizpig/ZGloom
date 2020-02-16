#pragma once

#include <cstdint>
#include <vector>
#include <cstdio>

//#include "gloommap.h"
//#include "gamelogic.h"

class Shape
{
	public:
		int32_t xh;
		int32_t yh;
		uint32_t w;
		uint32_t h;

		void Load(const uint8_t* indata, const uint8_t* palette);
		void DumpDebug(const char* name);

		std::vector<uint32_t> data;
};

class ObjectLogic
{
	public:
	std::vector<Shape>* shape;
	std::vector<Shape>* spark;
	uint32_t frame;
	uint32_t framespeed;
	uint32_t render; // "is 8 frame rotatable sprite", basically
	int16_t firey;
	int32_t movspeed;
	//setup for rnddelay
	int16_t range;
	int16_t base;
	int16_t firecnt;
	int16_t firerate;
	int16_t collwith;
	int16_t colltype;
	int16_t damage;
	int16_t hitpoints;
	int16_t weapon;
	int16_t hurtpause;
	uint16_t blood;
	int16_t punchrate;
	int16_t scale;
};

class ObjectGraphics
{
	public:

		enum ObjectGraphicType
		{
			OGT_PLAYER		,
			OGT_TOKENS		,
			OGT_MARINE		,
			OGT_BALDY		,
			OGT_TERRA		,
			OGT_GHOUL		,
			OGT_DEMON		,
			OGT_PHANTOM		,
			OGT_LIZARD		,
			OGT_DEATHHEAD	,
			OGT_DRAGON		,
			OGT_TROLL,		
			OGT_END
		};

		enum ObjectLogicType
		{
			OLT_PLAYER1,
			OLT_PLAYER2,
			OLT_HEALTH,
			OLT_WEAPON,
			OLT_THERMO,
			OLT_INFRA,
			OLT_INVISI,
			OLT_INVINC,
			OLT_DRAGON,
			OLT_BOUNCY,
			OLT_MARINE,
			OLT_BALDY,
			OLT_TERRA,
			OLT_GHOUL,
			OLT_PHANTOM,	
			OLT_DEMON,
			OLT_WEAPON1,	
			OLT_WEAPON2,	
			OLT_WEAPON3,	
			OLT_WEAPON4,	
			OLT_WEAPON5,	
			OLT_LIZARD,
			OLT_DEATHHEAD,	
			OLT_TROLL,
			OLT_END
		};

		uint32_t maxwidths[OLT_END];
		uint32_t maxwidthsgore[OLT_END];

		ObjectGraphics();
		static uint32_t LoadGraphic(const char* name, std::vector<Shape>& shape);
		static uint32_t LoadGraphicBin(const uint8_t* bindata, std::vector<Shape>& shape);

		std::vector<Shape> TokenShapes;
		std::vector<Shape> MarineShapes;
		std::vector<Shape> BaldyShapes;
		std::vector<Shape> GhoulShapes;
		std::vector<Shape> TerraShapes;
		std::vector<Shape> PhantomShapes;
		std::vector<Shape> DemonShapes;
		std::vector<Shape> DragonShapes;
		std::vector<Shape> LizardShapes;
		std::vector<Shape> DeathheadShapes;
		std::vector<Shape> TrollShapes;
		std::vector<Shape> BulletShapes[5];
		std::vector<Shape> SparkShapes[5];

		//ORGANS
		std::vector<Shape> MarineGore;
		std::vector<Shape> BaldyGore;
		std::vector<Shape> TerraGore;
		std::vector<Shape> PhantomGore;
		std::vector<Shape> DemonGore;
		std::vector<Shape> DragonGore;
		std::vector<Shape> LizardGore;
		std::vector<Shape> TrollGore;

		ObjectLogic objectlogic[OLT_END];

		std::vector<Shape>& GetGoreShape(int i)
		{
			if (i == OLT_MARINE) return MarineGore;
			if (i == OLT_BALDY) return BaldyGore;
			if (i == OLT_TERRA) return TerraGore;
			if (i == OLT_PHANTOM) return PhantomGore;
			if (i == OLT_DRAGON) return DragonGore;
			if (i == OLT_LIZARD) return LizardGore;
			if (i == OLT_DEMON) return DemonGore;
			if (i == OLT_TROLL) return TrollGore;

			printf("No gore object for %i\n", i);
			return MarineGore;
		};
};
