#pragma once 

#include <cstdint>
#include <vector>
#include "crmfile.h"

class Zone
{
	//
	// NEWTYPE.zone; a 'wall' for zones...
	public:

	void Load(const uint8_t* data);
	void DumpDebug(FILE* fFile);

	static const uint32_t ZoneSize = 32;

	enum ZoneType
	{
		ZT_WALL = 1,
		ZT_MONSTERZONE,
		ZT_EVENTTRIGGER
	};

	int16_t ztype; //1 = wall, 2 = monster zone, 3 = event trigger
	int16_t x1;
	int16_t z1;
	int16_t x2;
	int16_t z2;
	int32_t norm[1]; // non - zero = this is a normal one!
	int16_t	a;
	int16_t	b;
	int16_t	na;
	int16_t	nb;
	int16_t	ln;
	int8_t t[8]; //8 textures to use!
	int16_t	sc;
	int16_t ev;
};

class Column
{
	public:
	uint8_t flag;
	uint8_t data[64];
};

class Texture
{
	public:
	std::vector<Column> columns;
	uint8_t palette[256][3];

	void Load(const char* name);
	void DumpDebug(const char* name);
};

class Flat
{
	public:
	uint8_t data[128][128];
	uint8_t palette[256][3];
	void Load(const char* name);
	void DumpDebug(const char* name);
};

class GloomMap
{
	public:
		bool Load(const char* name);
		void SetFlat(char f);
		void DumpDebug();
		std::vector<Zone>& GetZones() { return zones; };
		Texture* GetTextures(){ return textures; };
		Flat& GetCeil() { return ceil; };
		Flat& GetFloor() { return floor; };

	private:
		CrmFile rawdata;
		std::vector<Zone> zones;
		Texture textures[8];
		Flat floor;
		Flat ceil;
		std::string texturenames[8];
};