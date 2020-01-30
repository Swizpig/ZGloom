#pragma once
#include "gloommap.h"
#include "gamelogic.h"

void CalcVecs(MapObject& o);
void MonsterLogic(MapObject& o, GameLogic* logic);
void NullLogic(MapObject& o, GameLogic* logic);
void NullLogicComp(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void KillLogicComp(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void WeaponGot(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void WeaponLogic(MapObject& o, GameLogic* logic);
void TerraLogic(MapObject& o, GameLogic* logic);
void GhoulLogic(MapObject& o, GameLogic* logic);
void HurtNGrunt(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void BlowObject(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void BlowObjectNoChunks(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void HurtTerra(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void BlowTerra(MapObject& thisobj, MapObject& otherobj, GameLogic* logic); 
void BaldyLogic(MapObject& o, GameLogic* logic);
void LizardLogic(MapObject& o, GameLogic* logic);
void LizHurt(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);

void Shoot(MapObject& o, GameLogic* logic, int32_t colltype, int32_t collwith, int32_t hitpoints, int32_t damage, int32_t speed, std::vector<Shape>* shape, std::vector<Shape>* spark);
