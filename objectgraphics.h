#pragma once

#include <cstdint>
#include <vector>

class Shape
{
	public:
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
	uint32_t frame;
	uint32_t framespeed;
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

		ObjectGraphics();
		void LoadGraphic(const char* name, std::vector<Shape>& shape);
		void LoadGraphicBin(const uint8_t* bindata, std::vector<Shape>& shape);

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

		//std::vector<Shape> Bullet1Shapes;

		ObjectLogic objectlogic[OLT_END];
};
