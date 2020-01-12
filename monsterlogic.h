#pragma once
#include "gloommap.h"
#include "gamelogic.h"

void CalcVecs(MapObject& o);
void MonsterLogic(MapObject& o, GameLogic* logic);
void NullLogic(MapObject& o, GameLogic* logic);
void WeaponLogic(MapObject& o, GameLogic* logic);

void Shoot(MapObject& o, GameLogic* logic, int32_t colltype, int32_t collwith, int32_t hitpoints, int32_t damage, int32_t speed, std::vector<Shape>* shape);
