#pragma once
#include "gloommap.h"
#include "gamelogic.h"

void CalcVecs(MapObject& o);
void MonsterLogic(MapObject& o, GameLogic* logic);
void NullLogic(MapObject& o, GameLogic* logic);
void NullLogicComp(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void KillLogicComp(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void WeaponGot(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void HealthGot(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void InvisGot(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void ThermoGot(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
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
void TrollHurt(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void TrollLogic(MapObject& o, GameLogic* logic);
void PhantomLogic(MapObject& o, GameLogic* logic);
void DemonLogic(MapObject& o, GameLogic* logic);
void PlayerDie(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void DeathLogic(MapObject& o, GameLogic* logic);
void BlowDeath(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void HurtDeath(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void BlowDragon(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);
void DragonLogic(MapObject& o, GameLogic* logic);
void PlayerDeath(MapObject& o, GameLogic* logic);
void BouncyLogic(MapObject& o, GameLogic* logic);
void BouncyGot(MapObject& thisobj, MapObject& otherobj, GameLogic* logic);

void Shoot(MapObject& o, GameLogic* logic, int32_t colltype, int32_t collwith, int32_t hitpoints, int32_t damage, int32_t speed, std::vector<Shape>* shape, std::vector<Shape>* spark);
