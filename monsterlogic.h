#pragma once
#include "gloommap.h"
#include "gamelogic.h"

void CalcVecs(MapObject& o);
void MonsterLogic(MapObject& o, GameLogic* logic);
void NullLogic(MapObject& o, GameLogic* logic);
void WeaponLogic(MapObject& o, GameLogic* logic);