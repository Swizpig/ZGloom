#pragma once 

#include <cstdint>
#include <vector>
#include <list>
#include "crmfile.h"
#include "quick.h"
#include "objectgraphics.h"


class Object
{
	// these are loaded from map
	public:
	uint16_t t;
	int16_t x;
	int16_t y;
	int16_t z;
	uint8_t rot;
	uint16_t ev;

	uint32_t frame;
	uint32_t framespeed;
};

// an object actuall in play

class MapObject
{
	public:
	Quick x;
	int16_t y;
	Quick z;
	int16_t t;

	int16_t rotx;
	int16_t rotz;

	uint32_t frame;
	uint32_t framespeed;

	MapObject(Object m)
	{
		x.SetInt(m.x);
		y = m.y;
		z.SetInt(m.z);
		t = m.t;

		frame = m.frame;
		framespeed = m.framespeed;
	}
};

class Door
{
	public:
		uint32_t zone;
		uint32_t eventnum;
};

class Event
{
	public:

	enum EventType
	{
		ET_ADDMONSTER = 1,
		ET_OPENDOOR = 2,
		ET_TELEPORT = 3,
		ET_LOADOBJECTS = 4,
		ET_CHANGETEXTURE = 5,
		ET_ROTATEPOLY = 6
	};

	void Load(const uint8_t* data, uint32_t evnum, std::vector<Object>& objects, std::vector<Door>& doors);
};

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
		bool Load(const char* name, ObjectGraphics* nobj);
		void SetFlat(char f);
		void DumpDebug();
		std::vector<Zone>& GetZones() { return zones; };
		Texture* GetTextures(){ return textures; };
		Flat& GetCeil() { return ceil; };
		Flat& GetFloor() { return floor; };
		std::list<MapObject>& GetMapObjects() { return mapobjects; };
		std::vector<uint32_t>& GetCollisions(int zt, int x, int z) {return collisionpolys[zt][x][z];};
		void ExecuteEvent(uint32_t e);

	private:
		static const int numevents = 24;

		CrmFile rawdata;
		std::vector<Zone> zones;
		Texture textures[8];
		Flat floor;
		Flat ceil;
		std::string texturenames[8];
		Event events[numevents];

		ObjectGraphics* objectlogic;

		std::vector<Object> objects;
		std::vector<Door> doors;

		std::list<MapObject> mapobjects;

		// collision grid, for both walls and events

		uint32_t gridnums[2][32][32];
		uint32_t polyoffsets[2][32][32];
		std::vector<uint32_t> collisionpolys[2][32][32];
};