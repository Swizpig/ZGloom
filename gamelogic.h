#pragma once

#include "gloommap.h"
#include "renderer.h"
#include "soundhandler.h"

class Camera;
class GloomMap;

class GameLogic
{
	public:
		void Init(GloomMap* gmapin, Camera* cam, ObjectGraphics* ograph);
		bool Update(Camera* cam);
		int32_t GetEffect();

		// needed public for monster logic. This is a mess, needs refactor to split more cleanly
		MapObject GetPlayerObj();

		bool Collision(bool event, int32_t x, int32_t z, int32_t r, int32_t& overshoot, int32_t& closestzone);
		bool AdjustPos(int32_t& overshoot, Quick& x, Quick& z, int32_t r, int32_t& closestzone);
		void AddObject(MapObject o, bool first) { if (first)  gmap->GetMapObjects().push_front(o);  else gmap->GetMapObjects().push_back(o); };
		uint8_t PickCalc(MapObject& o);

		struct weapontableentry { int32_t hitpoint; int32_t damage; int32_t speed; std::vector<Shape>* shape; std::vector<Shape>* spark; SoundHandler::Sounds sound; };
		weapontableentry wtable[5];
		ObjectGraphics* objectgraphics;
		std::list<MapObject> newobjects;

	private:
		GloomMap* gmap;
		int camdir = 1;

		//currently active teleport
		Teleport activetele;
		bool levelfinished;

		int32_t FindSegDist(int32_t x, int32_t z, Zone& zone);
		void DoDoor();
		void DoRot();
		void Rotter(int16_t x, int16_t z, int16_t&nx, int16_t& nz, int16_t camrots[4]);
		void ObjectCollision();
		uint32_t animframe[160];
		bool eventhit[25];

};