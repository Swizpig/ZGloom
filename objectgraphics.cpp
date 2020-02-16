#include "objectgraphics.h"
#include "crmfile.h"
#include "binresources.h"
#include "config.h"

#include <vector>

static uint16_t Get16(const uint8_t* p)
{
	return (static_cast<uint16_t>(p[0])) << 8 | static_cast<uint16_t>(p[1]);
}

static uint32_t Get32(const uint8_t* p)
{
	return (static_cast<uint16_t>(p[0])) << 24 | (static_cast<uint16_t>(p[1]) << 16) | (static_cast<uint16_t>(p[2])) << 8 | (static_cast<uint16_t>(p[3]) << 0);
}

void Shape::Load(const uint8_t* indata, const uint8_t* palette)
{
	// first 2 entries are handles
	xh = Get16(indata + 0);
	yh = Get16(indata + 2);
	w = Get16(indata + 4);
	h = Get16(indata + 6);

	data.resize(w*h);

	for (uint32_t p = 0; p < w*h; p++)
	{
		uint32_t col = palette[2 * indata[p + 8]] << 8;

		col |= palette[2 * indata[p+8] + 1];

		if (indata[p + 8] == 0)
		{
			//tranparent - pick a colour that can't exist as 444 expanded up
			data[p] = 1;
		}
		else
		{
			uint32_t col32 = (col & 0xf) | ((col & 0xf0) << 4) | ((col & 0xf00) << 8);
			col32 |= col32 << 4;
			data[p] = col32;
		}
	}
}

void Shape::DumpDebug(const char* name)
{
	//dump a ppm file
	FILE* file = fopen(name, "wb");

	fprintf(file, "P6\n%i %i\n255\n", w, h);

	for (uint32_t y = 0; y < h; y++)
	{
		for (uint32_t x = 0; x < w; x++)
		{
			uint32_t col = data[y + x*h];
			fputc((col >>16) & 0xFF, file);
			fputc((col >> 8) & 0xFF, file);
			fputc((col >> 0) & 0xFF, file);
		}
	}

	fclose(file);
}

uint32_t ObjectGraphics::LoadGraphicBin(const uint8_t* bindata, std::vector<Shape>& shape)
{
	uint32_t frames;

	frames = Get16(bindata + 2) << Get16(bindata + 0);

	uint32_t maxw, maxh;

	maxw = Get16(bindata + 4);
	maxh = Get16(bindata + 6);

	uint32_t paletteoffset = Get32(bindata + 8);

	std::vector<uint32_t> frameoffsets;

	frameoffsets.resize(frames);

	for (uint32_t f = 0; f < frames; f++)
	{
		frameoffsets[f] = Get32(bindata + 12 + f * 4);
	}

	shape.resize(frames);

	for (uint32_t f = 0; f < frames; f++)
	{
		shape[f].Load(bindata + frameoffsets[f], bindata + paletteoffset);
	}

	return maxw;
}


uint32_t ObjectGraphics::LoadGraphic(const char* name, std::vector<Shape>& shape)
{
	CrmFile file;

	file.Load(name);

	if (!file.data) return 0;

	uint32_t frames;

	frames = Get16(file.data + 2) << Get16(file.data + 0);

	uint32_t maxw, maxh;

	maxw = Get16(file.data + 4);
	maxh = Get16(file.data + 6);

	uint32_t paletteoffset = Get32(file.data + 8);

	std::vector<uint32_t> frameoffsets;

	frameoffsets.resize(frames);

	for (uint32_t f = 0; f < frames; f++)
	{
		frameoffsets[f] = Get32(file.data + 12 + f * 4);
	}

	shape.resize(frames);

	for (uint32_t f = 0; f < frames; f++)
	{
		shape[f].Load(file.data + frameoffsets[f], file.data + paletteoffset);
	}

	return maxw;
}

ObjectGraphics::ObjectGraphics()
{
	maxwidths[OLT_THERMO]    = LoadGraphic(Config::GetObjectFilename(OGT_TOKENS).c_str(), TokenShapes);
	maxwidths[OLT_MARINE]    = LoadGraphic(Config::GetObjectFilename(OGT_MARINE).c_str(), MarineShapes);
	maxwidths[OLT_BALDY]     = LoadGraphic(Config::GetObjectFilename(OGT_BALDY).c_str(), BaldyShapes);
	maxwidths[OLT_TERRA]     = LoadGraphic(Config::GetObjectFilename(OGT_TERRA).c_str(), TerraShapes);
	maxwidths[OLT_PHANTOM]   = LoadGraphic(Config::GetObjectFilename(OGT_PHANTOM).c_str(), PhantomShapes);
	maxwidths[OLT_GHOUL]     = LoadGraphic(Config::GetObjectFilename(OGT_GHOUL).c_str(), GhoulShapes);
	maxwidths[OLT_DRAGON]    = LoadGraphic(Config::GetObjectFilename(OGT_DRAGON).c_str(), DragonShapes);
	maxwidths[OLT_LIZARD]    = LoadGraphic(Config::GetObjectFilename(OGT_LIZARD).c_str(), LizardShapes);
	maxwidths[OLT_DEMON]     = LoadGraphic(Config::GetObjectFilename(OGT_DEMON).c_str(), DemonShapes);
	maxwidths[OLT_DEATHHEAD] = LoadGraphic(Config::GetObjectFilename(OGT_DEATHHEAD).c_str(), DeathheadShapes);
	maxwidths[OLT_TROLL]     = LoadGraphic(Config::GetObjectFilename(OGT_TROLL).c_str(), TrollShapes);
	maxwidths[OLT_WEAPON1] = LoadGraphicBin(BinResource::bullet1, BulletShapes[0]);
	maxwidths[OLT_WEAPON2] = LoadGraphicBin(BinResource::bullet2, BulletShapes[1]);
	maxwidths[OLT_WEAPON3] = LoadGraphicBin(BinResource::bullet3, BulletShapes[2]);
	maxwidths[OLT_WEAPON4] = LoadGraphicBin(BinResource::bullet4, BulletShapes[3]);
	maxwidths[OLT_WEAPON5] = LoadGraphicBin(BinResource::bullet5, BulletShapes[4]);

	LoadGraphicBin(BinResource::sparks1, SparkShapes[0]);
	LoadGraphicBin(BinResource::sparks2, SparkShapes[1]);
	LoadGraphicBin(BinResource::sparks3, SparkShapes[2]);
	LoadGraphicBin(BinResource::sparks4, SparkShapes[3]);
	LoadGraphicBin(BinResource::sparks5, SparkShapes[4]);

	maxwidthsgore[OLT_MARINE] = LoadGraphic(Config::GetGoreFilename(OGT_MARINE).c_str(), MarineGore);
	maxwidthsgore[OLT_BALDY] = LoadGraphic(Config::GetGoreFilename(OGT_BALDY).c_str(), BaldyGore);
	maxwidthsgore[OLT_TERRA] = LoadGraphic(Config::GetGoreFilename(OGT_TERRA).c_str(), TerraGore);
	maxwidthsgore[OLT_PHANTOM] = LoadGraphic(Config::GetGoreFilename(OGT_PHANTOM).c_str(), PhantomGore);
	maxwidthsgore[OLT_DRAGON] = LoadGraphic(Config::GetGoreFilename(OGT_DRAGON).c_str(), DragonGore);
	maxwidthsgore[OLT_LIZARD] = LoadGraphic(Config::GetGoreFilename(OGT_LIZARD).c_str(), LizardGore);
	maxwidthsgore[OLT_DEMON] = LoadGraphic(Config::GetGoreFilename(OGT_DEMON).c_str(), DemonGore);
	maxwidthsgore[OLT_TROLL] = LoadGraphic(Config::GetGoreFilename(OGT_TROLL).c_str(), TrollGore);

	maxwidths[OLT_HEALTH] = maxwidths[OLT_THERMO];
	maxwidths[OLT_WEAPON] = maxwidths[OLT_THERMO];
	maxwidths[OLT_INFRA]  = maxwidths[OLT_THERMO];
	maxwidths[OLT_INVISI] = maxwidths[OLT_THERMO];
	maxwidths[OLT_INVINC] = maxwidths[OLT_THERMO];
	maxwidths[OLT_BOUNCY] = maxwidths[OLT_THERMO];

	//TODO check these
	maxwidths[OLT_PLAYER1] = 32;
	maxwidths[OLT_PLAYER2] = 32;

	//
	//for (uint32_t s = 0; s < MarineShapes.size(); s++)
	//{
	//	char name[100];
	//	sprintf(name, "marine%i.ppm", s);
	//
	//	MarineShapes[s].DumpDebug(name);
	//}

	//objectlogic[OLT_PLAYER1].shape = player;
	//objectlogic[OLT_PLAYER2].shape = player;
	objectlogic[OLT_HEALTH].shape = &TokenShapes;
	//objectlogic[OLT_WEAPON].shape = weapon2;
	objectlogic[OLT_THERMO].shape = &TokenShapes;
	objectlogic[OLT_INFRA].shape = &TokenShapes;
	objectlogic[OLT_INVISI].shape = &TokenShapes;
	objectlogic[OLT_INVINC].shape = &TokenShapes;
	objectlogic[OLT_DRAGON].shape = &DragonShapes;
	objectlogic[OLT_BOUNCY].shape = &TokenShapes;
	objectlogic[OLT_MARINE].shape = &MarineShapes;
	objectlogic[OLT_BALDY].shape = &BaldyShapes;
	objectlogic[OLT_TERRA].shape = &TerraShapes;
	objectlogic[OLT_GHOUL].shape = &GhoulShapes;
	objectlogic[OLT_PHANTOM].shape = &PhantomShapes;
	objectlogic[OLT_DEMON].shape = &DemonShapes;
	objectlogic[OLT_WEAPON1].shape = &(BulletShapes[0]); // TODO: sparks and that
	objectlogic[OLT_WEAPON2].shape = &(BulletShapes[1]); // TODO: sparks and that;
	objectlogic[OLT_WEAPON3].shape = &(BulletShapes[2]); // TODO: sparks and that;
	objectlogic[OLT_WEAPON4].shape = &(BulletShapes[3]); // TODO: sparks and that;
	objectlogic[OLT_WEAPON5].shape = &(BulletShapes[4]); // TODO: sparks and that;
	objectlogic[OLT_LIZARD].shape = &LizardShapes;
	objectlogic[OLT_DEATHHEAD].shape = &DeathheadShapes;
	objectlogic[OLT_TROLL].shape = &TrollShapes;

	objectlogic[OLT_WEAPON1].spark = &(SparkShapes[0]); // TODO: sparks and that
	objectlogic[OLT_WEAPON2].spark = &(SparkShapes[1]); // TODO: sparks and that;
	objectlogic[OLT_WEAPON3].spark = &(SparkShapes[2]); // TODO: sparks and that;
	objectlogic[OLT_WEAPON4].spark = &(SparkShapes[3]); // TODO: sparks and that;
	objectlogic[OLT_WEAPON5].spark = &(SparkShapes[4]); // TODO: sparks and that;

	objectlogic[OLT_PLAYER1].frame	=0x0	  ;
	objectlogic[OLT_PLAYER2].frame	=0x0	  ;
	objectlogic[OLT_HEALTH].frame = 0x20000  ;
	objectlogic[OLT_WEAPON].frame	=0x0	  ;
	objectlogic[OLT_THERMO].frame = 0x00000  ;
	objectlogic[OLT_INFRA].frame = 0x00000  ;
	objectlogic[OLT_INVISI].frame = 0x10000  ;
	objectlogic[OLT_INVINC].frame = 0x20000  ;
	objectlogic[OLT_DRAGON].frame = 0x0	  ;
	objectlogic[OLT_BOUNCY].frame = 0x30000  ;
	objectlogic[OLT_MARINE].frame = 0x0	  ;
	objectlogic[OLT_BALDY].frame = 0x0	  ;
	objectlogic[OLT_TERRA].frame = 0x0	  ;
	objectlogic[OLT_GHOUL].frame = 0x0	  ;
	objectlogic[OLT_PHANTOM].frame = 0x0	  ;
	objectlogic[OLT_DEMON].frame = 0x0	  ;
	objectlogic[OLT_WEAPON1].frame = 0x0	  ;
	objectlogic[OLT_WEAPON2].frame = 0x0	  ;
	objectlogic[OLT_WEAPON3].frame = 0x0	  ;
	objectlogic[OLT_WEAPON4].frame = 0x0	  ;
	objectlogic[OLT_WEAPON5].frame = 0x0	  ;
	objectlogic[OLT_LIZARD].frame = 0x0	  ;
	objectlogic[OLT_DEATHHEAD].frame = 0x8000;
	objectlogic[OLT_TROLL].frame = 0x0	  ;

	// how many rotation frames
	objectlogic[OLT_PLAYER1].render = 8;
	objectlogic[OLT_PLAYER2].render = 8;
	objectlogic[OLT_HEALTH].render = 1;
	objectlogic[OLT_WEAPON].render = 1;
	objectlogic[OLT_THERMO].render = 1;
	objectlogic[OLT_INFRA].render = 1;
	objectlogic[OLT_INVISI].render = 1;
	objectlogic[OLT_INVINC].render = 1;
	objectlogic[OLT_DRAGON].render = 8;
	objectlogic[OLT_BOUNCY].render = 1;
	objectlogic[OLT_MARINE].render = 8;
	objectlogic[OLT_BALDY].render = 8;
	objectlogic[OLT_TERRA].render = 8;
	objectlogic[OLT_GHOUL].render = 8;
	objectlogic[OLT_PHANTOM].render = 8;
	objectlogic[OLT_DEMON].render = 8;
	objectlogic[OLT_WEAPON1].render = 1;
	objectlogic[OLT_WEAPON2].render = 1;
	objectlogic[OLT_WEAPON3].render = 1;
	objectlogic[OLT_WEAPON4].render = 1;
	objectlogic[OLT_WEAPON5].render = 1;
	objectlogic[OLT_LIZARD].render = 8;
	objectlogic[OLT_DEATHHEAD].render = 8;
	objectlogic[OLT_TROLL].render = 8;

	objectlogic[OLT_PLAYER1].movspeed = 0xd0000	 ;
	objectlogic[OLT_PLAYER2].movspeed = 0xd0000	 ;
	objectlogic[OLT_HEALTH].movspeed = 0		 ;
	objectlogic[OLT_WEAPON].movspeed = 0		 ;
	objectlogic[OLT_THERMO].movspeed = 0		 ;
	objectlogic[OLT_INFRA].movspeed = 0			 ;
	objectlogic[OLT_INVISI].movspeed = 0		 ;
	objectlogic[OLT_INVINC].movspeed = 0		 ;
	objectlogic[OLT_DRAGON].movspeed = 0xc0000	 ;
	objectlogic[OLT_BOUNCY].movspeed = 0		 ;
	objectlogic[OLT_MARINE].movspeed = 0x60000	 ;
	objectlogic[OLT_BALDY].movspeed = 0x40000	 ;
	objectlogic[OLT_TERRA].movspeed = 0x20000	 ;
	objectlogic[OLT_GHOUL].movspeed = 0x80000	 ;
	objectlogic[OLT_PHANTOM].movspeed = 0xa0000	 ;
	objectlogic[OLT_DEMON].movspeed = 0x70000	 ;
	objectlogic[OLT_WEAPON1].movspeed = 0		 ;
	objectlogic[OLT_WEAPON2].movspeed = 0		 ;
	objectlogic[OLT_WEAPON3].movspeed = 0		 ;
	objectlogic[OLT_WEAPON4].movspeed = 0		 ;
	objectlogic[OLT_WEAPON5].movspeed = 0		 ;
	objectlogic[OLT_LIZARD].movspeed = 0x60000	 ;
	objectlogic[OLT_DEATHHEAD].movspeed = 0xc0000 ;
	objectlogic[OLT_TROLL].movspeed = 0x60000	 ;

	objectlogic[OLT_PLAYER1].framespeed = 0x6000;
	objectlogic[OLT_PLAYER2].framespeed = 0x6000;
	objectlogic[OLT_HEALTH].framespeed = 0x0;
	objectlogic[OLT_WEAPON].framespeed = 0x08000;
	objectlogic[OLT_THERMO].framespeed = 0x0;
	objectlogic[OLT_INFRA].framespeed = 0x0;
	objectlogic[OLT_INVISI].framespeed = 0x0;
	objectlogic[OLT_INVINC].framespeed = 0x0;
	objectlogic[OLT_DRAGON].framespeed = 0x4000;
	objectlogic[OLT_BOUNCY].framespeed = 0x0;
	objectlogic[OLT_MARINE].framespeed = 0x6000;
	objectlogic[OLT_BALDY].framespeed = 0x4000;
	objectlogic[OLT_TERRA].framespeed = 0x6000;
	objectlogic[OLT_GHOUL].framespeed = 0x0;
	objectlogic[OLT_PHANTOM].framespeed = 0xa000;
	objectlogic[OLT_DEMON].framespeed = 0x7000;
	objectlogic[OLT_WEAPON1].framespeed = 0x08000;
	objectlogic[OLT_WEAPON2].framespeed = 0x08000;
	objectlogic[OLT_WEAPON3].framespeed = 0x08000;
	objectlogic[OLT_WEAPON4].framespeed = 0x08000;
	objectlogic[OLT_WEAPON5].framespeed = 0x08000;
	objectlogic[OLT_LIZARD].framespeed = 0x4000;
	objectlogic[OLT_DEATHHEAD].framespeed = 0x6000;
	objectlogic[OLT_TROLL].framespeed = 0x4000;

	objectlogic[OLT_PLAYER1].firey = -60;
	objectlogic[OLT_PLAYER2].firey = -60;
	objectlogic[OLT_HEALTH].firey	= 0;
	objectlogic[OLT_WEAPON].firey	= 0;
	objectlogic[OLT_THERMO].firey	= 0;
	objectlogic[OLT_INFRA].firey	= 0;
	objectlogic[OLT_INVISI].firey	= 0;
	objectlogic[OLT_INVINC].firey	= 0;
	objectlogic[OLT_DRAGON].firey = -144;
	objectlogic[OLT_BOUNCY].firey =	0;
	objectlogic[OLT_MARINE].firey = -60;
	objectlogic[OLT_BALDY].firey = -60;
	objectlogic[OLT_TERRA].firey = -60;
	objectlogic[OLT_GHOUL].firey = -64;
	objectlogic[OLT_PHANTOM].firey = -60;
	objectlogic[OLT_DEMON].firey = -90;
	objectlogic[OLT_WEAPON1].firey = 0;
	objectlogic[OLT_WEAPON2].firey = 0;
	objectlogic[OLT_WEAPON3].firey = 0;
	objectlogic[OLT_WEAPON4].firey = 0;
	objectlogic[OLT_WEAPON5].firey = 0;
	objectlogic[OLT_LIZARD].firey = -60;
	objectlogic[OLT_DEATHHEAD].firey =  -60;
	objectlogic[OLT_TROLL].firey = -60;

	// delay/ranges for rnddelay
	objectlogic[OLT_PLAYER1].range	= 1 ;
	objectlogic[OLT_PLAYER2].range	= 1 ;
	objectlogic[OLT_HEALTH].range = 0;
	objectlogic[OLT_WEAPON].range = 4;
	objectlogic[OLT_THERMO].range = 0;
	objectlogic[OLT_INFRA].range = 0;
	objectlogic[OLT_INVISI].range = 0;
	objectlogic[OLT_INVINC].range = 0;
	objectlogic[OLT_DRAGON].range = 32;
	objectlogic[OLT_BOUNCY].range = 0;
	objectlogic[OLT_MARINE].range = 32;
	objectlogic[OLT_BALDY].range = 16;
	objectlogic[OLT_TERRA].range = 48;
	objectlogic[OLT_GHOUL].range = 48;
	objectlogic[OLT_PHANTOM].range = 16;
	objectlogic[OLT_DEMON].range = 4;
	objectlogic[OLT_WEAPON1].range = 4;
	objectlogic[OLT_WEAPON2].range = 4;
	objectlogic[OLT_WEAPON3].range = 4;
	objectlogic[OLT_WEAPON4].range = 4;
	objectlogic[OLT_WEAPON5].range = 4;
	objectlogic[OLT_LIZARD].range = 8;
	objectlogic[OLT_DEATHHEAD].range = 16;
	objectlogic[OLT_TROLL].range = 8;

	objectlogic[OLT_PLAYER1].base = 1;
	objectlogic[OLT_PLAYER2].base = 1;
	objectlogic[OLT_HEALTH].base = 0;
	objectlogic[OLT_WEAPON].base = 4;
	objectlogic[OLT_THERMO].base = 0;
	objectlogic[OLT_INFRA].base = 0;
	objectlogic[OLT_INVISI].base = 0;
	objectlogic[OLT_INVINC].base = 0;
	objectlogic[OLT_DRAGON].base = 16;
	objectlogic[OLT_BOUNCY].base = 0;
	objectlogic[OLT_MARINE].base = 16;
	objectlogic[OLT_BALDY].base = 8;
	objectlogic[OLT_TERRA].base = 32;
	objectlogic[OLT_GHOUL].base = 32;
	objectlogic[OLT_PHANTOM].base = 8;
	objectlogic[OLT_DEMON].base = 32;
	objectlogic[OLT_WEAPON1].base = 4;
	objectlogic[OLT_WEAPON2].base = 4;
	objectlogic[OLT_WEAPON3].base = 4;
	objectlogic[OLT_WEAPON4].base = 4;
	objectlogic[OLT_WEAPON5].base = 4;
	objectlogic[OLT_LIZARD].base = 8;
	objectlogic[OLT_DEATHHEAD].base = -8;
	objectlogic[OLT_TROLL].base = 8;

	for (int i = 0; i < OLT_END; i++)
	{
		if ((i == OLT_GHOUL) || (i == OLT_TERRA))
		{
			objectlogic[i].firecnt = 5;
			objectlogic[i].firerate = 12;
		}
		else
		{
			objectlogic[i].firecnt = 0;
			objectlogic[i].firerate = 0;
		}
	}

	// this is pretty neat. And these together to get if an object collides
	objectlogic[OLT_PLAYER1].collwith = 4;
	objectlogic[OLT_PLAYER2].collwith = 4;
	objectlogic[OLT_HEALTH].collwith = 24;
	objectlogic[OLT_WEAPON].collwith = 24;
	objectlogic[OLT_THERMO].collwith = 24;
	objectlogic[OLT_INFRA].collwith = 24;
	objectlogic[OLT_INVISI].collwith = 24;
	objectlogic[OLT_INVINC].collwith = 24;
	objectlogic[OLT_DRAGON].collwith = 24 + 3;
	objectlogic[OLT_BOUNCY].collwith = 24;
	objectlogic[OLT_MARINE].collwith = 24 + 3;
	objectlogic[OLT_BALDY].collwith = 24 + 3;
	objectlogic[OLT_TERRA].collwith = 24 + 3;
	objectlogic[OLT_GHOUL].collwith = 24 + 3;
	objectlogic[OLT_PHANTOM].collwith = 3;
	objectlogic[OLT_DEMON].collwith = 3;
	objectlogic[OLT_WEAPON1].collwith = 24;
	objectlogic[OLT_WEAPON2].collwith = 24;
	objectlogic[OLT_WEAPON3].collwith = 24;
	objectlogic[OLT_WEAPON4].collwith = 24;
	objectlogic[OLT_WEAPON5].collwith = 24;
	objectlogic[OLT_LIZARD].collwith = 24 + 3;
	objectlogic[OLT_DEATHHEAD].collwith = 24 + 3;
	objectlogic[OLT_TROLL].collwith = 24 + 3;

	for (int i = 0; i < OLT_END; i++)
	{
		objectlogic[i].colltype = 0;
	}
	objectlogic[OLT_PLAYER1].colltype = 8;
	objectlogic[OLT_PLAYER2].colltype = 16;

	objectlogic[OLT_PLAYER1].damage = 1;
	objectlogic[OLT_PLAYER2].damage = 1;
	objectlogic[OLT_HEALTH].damage = 0;
	objectlogic[OLT_WEAPON].damage = 0;
	objectlogic[OLT_THERMO].damage = 0;
	objectlogic[OLT_INFRA].damage = 0;
	objectlogic[OLT_INVISI].damage = 0;
	objectlogic[OLT_INVINC].damage = 0;
	objectlogic[OLT_DRAGON].damage = 10;
	objectlogic[OLT_BOUNCY].damage = 0;
	objectlogic[OLT_MARINE].damage = 1;
	objectlogic[OLT_BALDY].damage = 2;
	objectlogic[OLT_TERRA].damage = 1;
	objectlogic[OLT_GHOUL].damage = 0;
	objectlogic[OLT_PHANTOM].damage = 3;
	objectlogic[OLT_DEMON].damage = 5;
	objectlogic[OLT_WEAPON1].damage = 0;
	objectlogic[OLT_WEAPON2].damage = 0;
	objectlogic[OLT_WEAPON3].damage = 0;
	objectlogic[OLT_WEAPON4].damage = 0;
	objectlogic[OLT_WEAPON5].damage = 0;
	objectlogic[OLT_LIZARD].damage = 2;
	objectlogic[OLT_DEATHHEAD].damage = 3;
	objectlogic[OLT_TROLL].damage = 3;

	objectlogic[OLT_PLAYER1].hitpoints = 25;
	objectlogic[OLT_PLAYER2].hitpoints = 25;
	objectlogic[OLT_HEALTH].hitpoints = 0;
	objectlogic[OLT_WEAPON].hitpoints = 0;
	objectlogic[OLT_THERMO].hitpoints = 0;
	objectlogic[OLT_INFRA].hitpoints = 0;
	objectlogic[OLT_INVISI].hitpoints = 0;
	objectlogic[OLT_INVINC].hitpoints = 0;
	objectlogic[OLT_DRAGON].hitpoints = 250;
	objectlogic[OLT_BOUNCY].hitpoints = 0;
	objectlogic[OLT_MARINE].hitpoints = 5;
	objectlogic[OLT_BALDY].hitpoints = 10;
	objectlogic[OLT_TERRA].hitpoints = 35;
	objectlogic[OLT_GHOUL].hitpoints = 5;
	objectlogic[OLT_PHANTOM].hitpoints = 10;
	objectlogic[OLT_DEMON].hitpoints = 25;
	objectlogic[OLT_WEAPON1].hitpoints = 0;
	objectlogic[OLT_WEAPON2].hitpoints = 0;
	objectlogic[OLT_WEAPON3].hitpoints = 0;
	objectlogic[OLT_WEAPON4].hitpoints = 0;
	objectlogic[OLT_WEAPON5].hitpoints = 0;
	objectlogic[OLT_LIZARD].hitpoints = 10;
	objectlogic[OLT_DEATHHEAD].hitpoints = 35;
	objectlogic[OLT_TROLL].hitpoints = 18;

	objectlogic[OLT_PLAYER1].weapon = 0;
	objectlogic[OLT_PLAYER2].weapon = 0;
	objectlogic[OLT_HEALTH].weapon = 0;
	objectlogic[OLT_WEAPON].weapon = 1;
	objectlogic[OLT_THERMO].weapon = 0;
	objectlogic[OLT_INFRA].weapon = 0;
	objectlogic[OLT_INVISI].weapon = 0;
	objectlogic[OLT_INVINC].weapon = 0;
	objectlogic[OLT_DRAGON].weapon = 0;
	objectlogic[OLT_BOUNCY].weapon = 0;
	objectlogic[OLT_MARINE].weapon = 0;
	objectlogic[OLT_BALDY].weapon = 0;
	objectlogic[OLT_TERRA].weapon = 0;
	objectlogic[OLT_GHOUL].weapon = 0;
	objectlogic[OLT_PHANTOM].weapon = 0;
	objectlogic[OLT_DEMON].weapon = 0;
	objectlogic[OLT_WEAPON1].weapon = 0;
	objectlogic[OLT_WEAPON2].weapon = 1;
	objectlogic[OLT_WEAPON3].weapon = 2;
	objectlogic[OLT_WEAPON4].weapon = 3;
	objectlogic[OLT_WEAPON5].weapon = 4;
	objectlogic[OLT_LIZARD].weapon = 0;
	objectlogic[OLT_DEATHHEAD].weapon = 0;
	objectlogic[OLT_TROLL].weapon = 0;

	objectlogic[OLT_PLAYER1].hurtpause = 5;
	objectlogic[OLT_PLAYER2].hurtpause = 5;
	objectlogic[OLT_HEALTH].hurtpause = 5;
	objectlogic[OLT_WEAPON].hurtpause = 0;
	objectlogic[OLT_THERMO].hurtpause = 5;
	objectlogic[OLT_INFRA].hurtpause = 5;
	objectlogic[OLT_INVISI].hurtpause = 5;
	objectlogic[OLT_INVINC].hurtpause = 5;
	objectlogic[OLT_DRAGON].hurtpause = 5;
	objectlogic[OLT_BOUNCY].hurtpause = 0;
	objectlogic[OLT_MARINE].hurtpause = 5;
	objectlogic[OLT_BALDY].hurtpause = 3;
	objectlogic[OLT_TERRA].hurtpause = 0;
	objectlogic[OLT_GHOUL].hurtpause = 5;
	objectlogic[OLT_PHANTOM].hurtpause = 7;
	objectlogic[OLT_DEMON].hurtpause = 5;
	objectlogic[OLT_WEAPON1].hurtpause = 0;
	objectlogic[OLT_WEAPON2].hurtpause = 0;
	objectlogic[OLT_WEAPON3].hurtpause = 0;
	objectlogic[OLT_WEAPON4].hurtpause = 0;
	objectlogic[OLT_WEAPON5].hurtpause = 0;
	objectlogic[OLT_LIZARD].hurtpause = 2;
	objectlogic[OLT_DEATHHEAD].hurtpause = 10;
	objectlogic[OLT_TROLL].hurtpause = 2;
	
	objectlogic[  OLT_PLAYER1].blood	= 0xf00	 ;
	objectlogic[  OLT_PLAYER2].blood	= 0xf00	 ;
	objectlogic[   OLT_HEALTH].blood	= 0xf00	 ;
	objectlogic[   OLT_WEAPON].blood	= 0	     ;
	objectlogic[   OLT_THERMO].blood	= 0xf00	 ;
	objectlogic[    OLT_INFRA].blood	= 0xf00	 ;
	objectlogic[   OLT_INVISI].blood	= 0xf00	 ;
	objectlogic[   OLT_INVINC].blood	= 0xf00	 ;
	objectlogic[   OLT_DRAGON].blood	= 0xf00	 ;
	objectlogic[   OLT_BOUNCY].blood	= 0xf00	 ;
	objectlogic[   OLT_MARINE].blood	= 0xf00	 ;
	objectlogic[    OLT_BALDY].blood	= 0xf00	 ;
	objectlogic[    OLT_TERRA].blood	= 0xfff	 ;
	objectlogic[    OLT_GHOUL].blood	= 0x80f0 ;
	objectlogic[  OLT_PHANTOM].blood	= 0xff0	 ;
	objectlogic[    OLT_DEMON].blood	= 0xf00	 ;
	objectlogic[  OLT_WEAPON1].blood	= 0	     ;
	objectlogic[  OLT_WEAPON2].blood	= 0	     ;
	objectlogic[  OLT_WEAPON3].blood	= 0	     ;
	objectlogic[  OLT_WEAPON4].blood	= 0	     ;
	objectlogic[  OLT_WEAPON5].blood	= 0	     ;
	objectlogic[   OLT_LIZARD].blood	= 0xf0f	 ;
	objectlogic[OLT_DEATHHEAD].blood	= 0xf00	 ;
	objectlogic[    OLT_TROLL].blood	= 0xf00	 ;

	objectlogic[OLT_PLAYER1].scale = 0x200;
	objectlogic[OLT_PLAYER2].scale = 0x200;
	objectlogic[OLT_HEALTH].scale = 0x200;
	objectlogic[OLT_WEAPON].scale = 0x200;
	objectlogic[OLT_THERMO].scale = 0x200;
	objectlogic[OLT_INFRA].scale = 0x200;
	objectlogic[OLT_INVISI].scale = 0x200;
	objectlogic[OLT_INVINC].scale = 0x200;
	objectlogic[OLT_DRAGON].scale = 0x300;
	objectlogic[OLT_BOUNCY].scale = 0x200;
	objectlogic[OLT_MARINE].scale = 0x200;
	objectlogic[OLT_BALDY].scale = 0x220;
	objectlogic[OLT_TERRA].scale = 0x280;
	objectlogic[OLT_GHOUL].scale = 0x200;
	objectlogic[OLT_PHANTOM].scale = 0x280;
	objectlogic[OLT_DEMON].scale = 0x380;
	objectlogic[OLT_WEAPON1].scale = 0x200;
	objectlogic[OLT_WEAPON2].scale = 0x200;
	objectlogic[OLT_WEAPON3].scale = 0x200;
	objectlogic[OLT_WEAPON4].scale = 0x200;
	objectlogic[OLT_WEAPON5].scale = 0x200;
	objectlogic[OLT_LIZARD].scale = 0x240;
	objectlogic[OLT_DEATHHEAD].scale = 0x200;
	objectlogic[OLT_TROLL].scale = 0x240;


	for (int i = 0; i < OLT_END; i++)
	{
		objectlogic[i].punchrate = 0;
	}
	objectlogic[OLT_BALDY].punchrate = 4;
	objectlogic[OLT_LIZARD].punchrate = 3;
	objectlogic[OLT_TROLL].punchrate = 3;


	return;
}