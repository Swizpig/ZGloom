#pragma once

#include "gloommap.h"
#include "renderer.h"
#include "soundhandler.h"

class Camera;
class GloomMap;

class GameLogic
{
	public:
		void Init(ObjectGraphics* ograph);
		void InitLevel(GloomMap* gmapin, Camera* cam, ObjectGraphics* ograph);
		bool Update(Camera* cam);
		int32_t GetTeleEffect();
		bool GetThermo();
		bool GetPlayerHit() { return playerhit; };
		void WereDoneHere() { levelfinishednow = true; };

		//deathhead suck logic
		void SetSucking(uint64_t ns){ sucking = ns; };
		void SetSucker(uint64_t ns){ sucker = ns; };
		void SetSuckAngle(uint8_t ns){ suckangle = ns; };
		uint64_t GetSucking(){ return sucking; };
		uint64_t GetSucker(){ return sucker; };

		// needed public for monster logic. This is a mess, needs refactor to split more cleanly
		MapObject GetPlayerObj();
		MapObject GetNamedObj(uint64_t id);

		bool Collision(bool event, int32_t x, int32_t z, int32_t r, int32_t& overshoot, int32_t& closestzone);
		bool AdjustPos(int32_t& overshoot, Quick& x, Quick& z, int32_t r, int32_t& closestzone);
		void AddObject(MapObject o, bool first) { if (first)  gmap->GetMapObjects().push_front(o);  else gmap->GetMapObjects().push_back(o); };
		void GetNorm(int32_t zone, int32_t& na, int32_t& nb) { na = gmap->GetZones()[zone].na;  nb = gmap->GetZones()[zone].nb; };
		void AddBlood(Blood b) { gmap->GetBlood().push_back(b); };
		void ResetPlayer(MapObject& o);
		uint8_t PickCalc(MapObject& o);

		struct weapontableentry { int32_t hitpoint; int32_t damage; int32_t speed; std::vector<Shape>* shape; std::vector<Shape>* spark; SoundHandler::Sounds sound; };
		weapontableentry wtable[5];
		ObjectGraphics* objectgraphics;
		std::list<MapObject> newobjects;

	private:
		GloomMap* gmap;

		//currently active teleport
		Teleport activetele;
		bool levelfinished;
		//Game has ended!
		bool levelfinishednow;

		int32_t FindSegDist(int32_t x, int32_t z, Zone& zone);
		void CheckSuck(Camera* cam);
		void DoDoor();
		void DoRot();
		void Rotter(int16_t x, int16_t z, int16_t&nx, int16_t& nz, int16_t camrots[4]);
		void MoveBlood();
		void ObjectCollision();
		uint32_t animframe[160];
		bool eventhit[25];

		bool firedown;

		//was I hit this frame?
		bool playerhit = false;

		//storage for data saved between levels
		int16_t p1health;
		int16_t p1lives;
		int16_t p1weapon;
		int16_t p1reload;

		// and if I die in a level
		Quick origx;
		Quick origz;
		uint32_t origrot;

		//deathhead suck logic
		uint64_t sucking;
		uint64_t sucker;
		uint8_t suckangle;
};
