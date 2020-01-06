#pragma once 

#include <cstdint>
#include <vector>
#include <list>
#include "crmfile.h"
#include "quick.h"
#include "objectgraphics.h"

// rot/morph poly

class ActiveRotPoly
{
	public:
	int16_t speed;
	int16_t rot;
	int16_t flags;// what to do ?
	int16_t cx; //only for rot
	int16_t cz;
	int32_t	first; //pointer to first!
	int16_t num;
		
	int16_t vx[32];
	int16_t lx[32];
	int16_t vz[32];
	int16_t lz[32];
	int16_t ox[32];
	int16_t na[32];
	int16_t oz[32];
	int16_t nb[32];
};

//texture animation

class Anim
{
	public:
		int16_t frames;
		int16_t first;
		int16_t delay;
		int16_t current;
};

class TextureChange
{
	public:
	uint32_t zone;
	uint32_t newtexture;
	uint16_t ev;
};

class RotPoly
{
	public:
	int16_t polynum;
	int16_t count;
	int16_t speed;
	int16_t flags;
	uint16_t ev;
};

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
	uint32_t render;
	int32_t movspeed;
};

// an object actually in play

class GameLogic;

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

	uint32_t render;

	uint32_t rot;
	uint32_t oldrot;
	int32_t movspeed;
	int32_t xvec;
	int32_t zvec;

	void(*logic)(MapObject&, GameLogic*);
	void(*oldlogic)(MapObject&, GameLogic*);
	void(*oldlogic2)(MapObject&, GameLogic*);

	MapObject(Object m);
};

class Door
{
	public:
		uint32_t zone;
		uint32_t eventnum;
};

class ActiveDoor
{
	public:
		uint32_t do_poly; //door polygon
		int16_t do_lx;
		int16_t do_lz;
		int16_t do_rx;
		int16_t do_rz;
		int16_t do_frac;
		int16_t	do_fracadd;
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

	void Load(const uint8_t* data, uint32_t evnum, std::vector<Object>& objects, std::vector<Door>& doors, std::vector<TextureChange>& tchanges, std::vector<RotPoly>& rotpolys);
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
	int16_t open;
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
		std::vector<Anim>& GetAnims() { return anims; };
		Texture* GetTextures(){ return textures; };
		std::vector<TextureChange>& GetTChange(){ return tchanges; };
		bool HasFlat() { return hasflat; };
		Flat& GetCeil() { return ceil; };
		Flat& GetFloor() { return floor; };
		std::list<MapObject>& GetMapObjects() { return mapobjects; };
		std::list<ActiveDoor>& GetActiveDoors() { return activedoors; };
		std::vector<ActiveRotPoly>& GetActiveRotPolys() { return activerotpolys; };
		Column** GetTexPointers(){ return texturepointers;};
		Column** GetTexPointersOrig(){ return texturepointersorig; };
		std::vector<uint32_t>& GetCollisions(int zt, int x, int z) {return collisionpolys[zt][x][z];};
		void ExecuteEvent(uint32_t e);
		GloomMap() { hasflat = false; };

	private:
		static const int numevents = 24;

		CrmFile rawdata;
		std::vector<Zone> zones;
		Texture textures[8];
		Flat floor;
		Flat ceil;
		std::string texturenames[8];
		Event events[numevents];
		bool hasflat;

		ObjectGraphics* objectlogic;

		std::vector<Object> objects;
		std::vector<Door> doors;
		std::vector<Anim> anims;
		std::vector<TextureChange> tchanges;
		std::vector<RotPoly> rotpolys;

		std::list<ActiveDoor> activedoors;
		std::list<MapObject> mapobjects;
		std::vector<ActiveRotPoly> activerotpolys;

		// texture pointers, used for remapping anims. 160 = 20 * 8;
		Column* texturepointers[160];
		Column* texturepointersorig[160];

		// collision grid, for both walls and events

		uint32_t gridnums[2][32][32];
		uint32_t polyoffsets[2][32][32];
		std::vector<uint32_t> collisionpolys[2][32][32];
};