#include "gloommap.h"
#include "gloommaths.h"
#include "gamelogic.h"
#include "monsterlogic.h"

void BaldyPunch(MapObject& o, GameLogic* logic);
int8_t CheckColl(MapObject& o1, MapObject &o2);

void NullLogic(MapObject& o, GameLogic* logic)
{
	return;
}

void NullLogicComp(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	return;
}

int32_t BloodSpeed()
{
	int32_t result = (int16_t)GloomMaths::RndW();
	result <<= 2;
	return result;
}

int32_t BloodSpeed2()
{
	int32_t result = (int16_t)GloomMaths::RndW();
	result <<= 5;
	return result;
}

int32_t BloodSpeed3()
{
	int32_t result = (int16_t)GloomMaths::RndW();
	result <<= 4;
	return result;
}

int16_t RndDelay(MapObject&o)
{
	/*
	rnddelay	move	ob_range(a5), d0
	bsr	rndn
	add	ob_base(a5), d0
	move	d0, ob_delay(a5)
	;
	rts
	*/

	o.data.ms.delay = GloomMaths::RndN(o.data.ms.range) + o.data.ms.base;
	return o.data.ms.delay;
}


void BloodyMess(MapObject& o, GameLogic* logic, int count)
{
	/*
	bloodymess	;throw random blood splots everywhere!
	;
	bsr	bloodspeed2
	add.l	ob_x(a5),d0
	move.l	d0,d2
	bsr	bloodspeed2
	add.l	ob_gutsy(a5),d0
	move.l	d0,d3
	bsr	bloodspeed2
	add.l	ob_z(a5),d0
	move.l	d0,d4
	;
.loop	addlast	blood
	beq.s	.done
	;
	movem.l	d2-d4,bl_x(a0)
	bsr	bloodspeed
	move.l	d0,bl_xvec(a0)
	bsr	bloodspeed
	move.l	d0,bl_yvec(a0)
	bsr	bloodspeed
	move.l	d0,bl_zvec(a0)
	move	ob_blood(a5),bl_color(a0)
	;
	dbf	d7,.loop
	;
.done	rts

	*/

	Quick x, y, z, temp;

	x = o.x;
	y.SetInt(-64);// TODO: wire up gutsy
	z = o.z;

	temp.SetVal(BloodSpeed2());
	x = x + temp;
	temp.SetVal(BloodSpeed2());
	y = y + temp;
	temp.SetVal(BloodSpeed2());
	z = z + temp;

	for (int b = 0; b < count; b++)
	{
		Blood bloodobj;

		bloodobj.x = x;
		bloodobj.y = y;
		bloodobj.z = z;

		bloodobj.xvec.SetVal(BloodSpeed());
		bloodobj.yvec.SetVal(BloodSpeed());
		bloodobj.zvec.SetVal(BloodSpeed());
		bloodobj.color = o.data.ms.blood;

		logic->AddBlood(bloodobj);
	}
}

void BloodyMess2(MapObject& o, GameLogic* logic, int count)
{
	Quick x, y, z, temp;

	x = o.x;
	y.SetInt(-64);// TODO: wire up gutsy
	z = o.z;

	temp.SetVal(BloodSpeed2());
	x = x + temp;
	temp.SetVal(BloodSpeed2());
	y = y + temp;
	temp.SetVal(BloodSpeed2());
	z = z + temp;

	for (int b = 0; b < count; b++)
	{
		Blood bloodobj;

		bloodobj.x = x;
		bloodobj.y = y;
		bloodobj.z = z;

		bloodobj.xvec.SetVal(BloodSpeed3());
		bloodobj.yvec.SetVal(BloodSpeed3());
		bloodobj.zvec.SetVal(BloodSpeed3());
		bloodobj.color = o.data.ms.blood;

		logic->AddBlood(bloodobj);
	}
}

void SparksLogic(MapObject& o, GameLogic* logic)
{
	/*
	sparkslogic	subq	#1,ob_delay(a5)
	ble	killobject
	movem.l	ob_x(a5),d0-d2
	add.l	ob_xvec(a5),d0
	add.l	ob_yvec(a5),d1
	add.l	ob_zvec(a5),d2
	movem.l	d0-d2,ob_x(a5)
	rts
	*/
	o.data.ms.delay--;

	if (o.data.ms.delay <= 0)
	{
		o.killme = true;
		return;
	}

	Quick temp;
	temp.SetVal(o.data.ms.xvec);
	o.x = o.x + temp;
	temp.SetVal(o.data.ms.yvec);
	o.y = o.y + temp;
	temp.SetVal(o.data.ms.zvec);
	o.z = o.z + temp;
}

void WeaponLogic(MapObject& o, GameLogic* logic)
{
	/*
	weaponlogic;
	move.l	camrots(pc), a0
		;
	addq	#8, ob_movspeed(a5)
	move	ob_movspeed(a5), d0
	and	#127, d0
	move	2(a0, d0 * 8), d0
	asr	#8, d0
	move	d0, ob_y(a5)
	*/

	int16_t camrots[4];

	o.data.ms.movspeed += 8;

	GloomMaths::GetCamRotRaw(o.data.ms.movspeed & 127, camrots);

	o.y.SetInt(camrots[1] >> 8);

	o.data.ms.frame += (1 << 16);
	if ((o.data.ms.frame >> 16) >= o.data.ms.shape->size())
	{
		o.data.ms.frame = 0;
	}

	o.data.ms.delay--;

	if (o.data.ms.delay <= 0)
	{
		RndDelay(o);
		MapObject sparksobj;

		sparksobj.t = 999;
		sparksobj.x = o.x;
		sparksobj.y = o.y;
		sparksobj.z = o.z;

		sparksobj.data.ms.xvec = BloodSpeed2();
		sparksobj.data.ms.yvec = BloodSpeed2();
		sparksobj.data.ms.zvec = BloodSpeed2();

		/*
		move.l	ob_chunks(a5),a2
		move.l	a2,ob_shape(a0)
		move	2(a2),d0
		bsr	rndn
		move	d0,ob_frame(a0)
		move.l	#sparkslogic,ob_logic(a0)
		move.l	#drawshape_1,ob_render(a0)
		clr	ob_invisible(a0)
		clr	ob_colltype(a0)
		clr	ob_collwith(a0)
		bsr	rndw
		and	#15,d0
		add	#15,d0
		move	d0,ob_delay(a0)
		*/
		sparksobj.data.ms.shape = o.data.ms.chunks;
		sparksobj.data.ms.frame = GloomMaths::RndN(o.data.ms.chunks->size()) << 16;
		sparksobj.data.ms.logic = SparksLogic;
		sparksobj.data.ms.render = 1;
		sparksobj.data.ms.colltype = 0;
		sparksobj.data.ms.collwith = 0;
		sparksobj.data.ms.delay = (GloomMaths::RndW() & 15) + 15;
		sparksobj.data.ms.blood = 0;

		logic->AddObject(sparksobj, false);
	}
}

void MakeSparks(MapObject& o, GameLogic* logic)
{
	MapObject sparksobj;

	sparksobj.t = 999;
	sparksobj.x = o.x;
	sparksobj.y = o.y;
	sparksobj.z = o.z;

	int num = o.data.ms.chunks->size();

	for (int i = 0; i < num; i++)
	{
		sparksobj.data.ms.xvec = BloodSpeed2();
		sparksobj.data.ms.yvec = BloodSpeed2();
		sparksobj.data.ms.zvec = BloodSpeed2();

		sparksobj.data.ms.shape = o.data.ms.chunks;
		sparksobj.data.ms.frame = GloomMaths::RndN(o.data.ms.chunks->size()) << 16;
		sparksobj.data.ms.logic = SparksLogic;
		sparksobj.data.ms.render = 1;
		sparksobj.data.ms.colltype = 0;
		sparksobj.data.ms.collwith = 0;
		sparksobj.data.ms.delay = (GloomMaths::RndW() & 15) + 15;
		sparksobj.data.ms.blood = 0;

		logic->AddObject(sparksobj, false);
	}
}

void CalcVecs(MapObject& o)
{
	/*
	calcvecs	move	ob_rot(a5), d0
		;
		and	#255, d0
		move.l	camrots(pc), a0
		lea	0(a0, d0 * 8), a0
		;
		move	ob_movspeed(a5), d4
		move	d4, d5
		muls	2(a0), d4
		add.l	d4, d4
		neg.l	d4
		move.l	d4, ob_xvec(a5)
		muls	6(a0), d5
		add.l	d5, d5
		move.l	d5, ob_zvec(a5)
		rts
	*/

	uint32_t ang = o.data.ms.rot & 255;

	Quick camrots[4], t;
	GloomMaths::GetCamRot(ang, camrots);
	t.SetVal(o.data.ms.movspeed);

	o.data.ms.xvec = -(t * camrots[1]).GetVal();
	o.data.ms.zvec = (t * camrots[3]).GetVal();
	return;
}

bool CheckVecs(MapObject& o, GameLogic* logic)
{
	/*
	checkvecs	movem.l	ob_xvec(a5), d6 - d7
		add.l	ob_x(a5), d6
		add.l	ob_z(a5), d7
		bsr	checknewslow; ok to stand here ?
		beq.s.ok
		;
		move.l	ob_x(a5), d6
		move.l	ob_z(a5), d7
		bsr	checknewslow
		bne.s.fix
		moveq	# - 1, d1; use old pos, and report hit!
		rts
		;
	.fix	bsr	adjustposq; fixup!
		moveq	# - 1, d1
		;
	.ok	move.l	d6, ob_x(a5)
		move.l	d7, ob_z(a5)
		tst	d1
	*/

	Quick newx, newz;

	Quick t;

	newx = o.x;
	newz = o.z;

	t.SetVal(o.data.ms.xvec);
	newx = newx + t;

	t.SetVal(o.data.ms.zvec);
	newz = newz + t;

	int32_t overshoot, zone;

	// TODO: radius
	if (logic->Collision(false, newx.GetInt(), newz.GetInt(), 32, overshoot, zone))
	{
		if (logic->Collision(false, o.x.GetInt(), o.z.GetInt(), 32, overshoot, zone))
		{
			logic->AdjustPos(overshoot, newx, newz, 32, zone);
			o.x = newx;
			o.z = newz;
			return false;
		}
		else
		{
			return false;
		}
	}
	else
	{
		//OK
		o.x = newx;
		o.z = newz;

		return true;
	}
}

void PauseLogic(MapObject&o, GameLogic* logic)
{
	/*
		pauselogic	subq	#1, ob_delay(a5)
		bgt.s.skip
		;
		bsr	rnddelay
		move.l	ob_oldlogic(a5), ob_logic(a5)
		;
		; if in front of player, continue on old course...
		;
		bsr	pickcalc
		;
		move	ob_rot(a0), d1
		and	#255, d1
		sub	d0, d1
		bpl.s.pl
		neg	d1
		.pl	cmp	#64, d1
		bcs.s.skip
		cmp	#192, d1
		bcc.s.skip
		;
		.useold	move	ob_oldrot(a5), ob_rot(a5)
		bsr	calcvecs
		;
		.skip	rts
	*/
	o.data.ms.delay--;

	if (o.data.ms.delay <= 0)
	{
		MapObject player = logic->GetPlayerObj();
		RndDelay(o);
		o.data.ms.logic = o.data.ms.oldlogic;

		uint8_t ang = logic->PickCalc(o);

		int16_t compang = (int16_t)(player.data.ms.rot&0xFF) - ang;

		if (compang < 0) compang = -compang;

		if ((compang>64) && (compang < 192))
		{
			o.data.ms.rot = o.data.ms.oldrot;
			CalcVecs(o);
		}
	}
}

void Fire1(MapObject& o, GameLogic* logic)
{
	/*
	fire1	bsr	pickcalc
	;
	; random noise for inaccuracy!
		;
		move	d0, -(a7)
		bsr	rndw
		and	#31, d0
		sub	#16, d0
		add(a7) + , d0
		and	#255, d0
		;
		move	d0, ob_rot(a5)
		bsr	calcvecs
		move	#7, ob_delay(a5)
		move.l	ob_logic(a5), ob_oldlogic(a5)
		move.l	#pauselogic, ob_logic(a5)
		clr.l	ob_frame(a5)
		;
		moveq	#4, d2; colltype
		moveq	#0, d3; collwith
		moveq	#1, d4; hitpoints
		moveq	#1, d5; damage
		moveq	#20, d6; speed
		moveq	#0, d7; acceleration!
		lea	bullet1, a2
		lea	sparks1, a3
		;
		bsr	shoot
		;
		rts
	*/

	uint16_t ang = logic->PickCalc(o);
	uint16_t rand = GloomMaths::RndW();

	ang += (rand & 31) - 16;
	ang &= 0xFF;

	o.data.ms.rot = ang;
	CalcVecs(o);
	o.data.ms.delay = 7;

	o.data.ms.oldlogic = o.data.ms.logic;

	o.data.ms.logic = PauseLogic;
	o.data.ms.frame = 0;

	Shoot(o, logic, 4, 0, 1, 1, 20, logic->wtable[0].shape, logic->wtable[0].spark);
}

void MonsterFix(MapObject& o, GameLogic* logic)
{
	if (1)//!CheckVecs(o, logic))
	{
		// try +/- 90 degrees
		o.data.ms.rot += (GloomMaths::RndW() > 0) ? 64 : -64;
		CalcVecs(o);
		if (CheckVecs(o, logic)) goto good;

		o.data.ms.rot += 128;
		CalcVecs(o);
		if (CheckVecs(o, logic)) goto good;

		o.data.ms.rot = o.data.ms.oldrot + 128;
		CalcVecs(o);
		CheckVecs(o, logic);
	}

good:
	o.data.ms.frame += o.data.ms.framespeed;
	o.data.ms.frame &= 0x3FFFF;
}

void MonsterMove(MapObject& o, GameLogic* logic)
{
	/*
	monstermove	bsr	checkvecs
	beq.s	monsternew
	;
	; OK, try 90 / -90 degrees...
	;
	monsterfix	bsr	rndw
	moveq	#64, d1
	tst	d0
	bpl.s.umk
	moveq	# - 64, d1
	.umk	add	d1, ob_rot(a5)
	bsr	calcvecs
	bsr	checkvecs
	beq.s	monsternew
	;
	add	#128, ob_rot(a5)
	bsr	calcvecs
	bsr	checkvecs
	beq.s	monsternew
	;
	move	ob_oldrot(a5), d0
	add	#128, d0
	move	d0, ob_rot(a5)
	;
	bsr	calcvecs
	bsr	checkvecs
	monsternew;
	move.l	ob_framespeed(a5), d0
	add.l	d0, ob_frame(a5)
	and	#3, ob_frame(a5)
	rts

	*/

	if (!CheckVecs(o, logic))
	{
		// try +/- 90 degrees
		o.data.ms.rot += (GloomMaths::RndW() > 0) ? 64 : -64;
		CalcVecs(o);
		if (CheckVecs(o, logic)) goto good;

		o.data.ms.rot += 128;
		CalcVecs(o);
		if (CheckVecs(o, logic)) goto good;

		o.data.ms.rot = o.data.ms.oldrot + 128;
		CalcVecs(o);
		CheckVecs(o, logic);
	}

good:
	o.data.ms.frame += o.data.ms.framespeed;
	o.data.ms.frame &= 0x3FFFF;
}

void MonsterLogic(MapObject& o, GameLogic* logic)
{
	/*
	monsterlogic;
	move	ob_rot(a5), ob_oldrot(a5)
	; monster cruising around minding his own business...
	;
	*/

	o.data.ms.oldrot = o.data.ms.rot;
	/* 
		subq	#1, ob_delay(a5)
		ble	fire1
		;
		*/

	o.data.ms.delay--;

	if (o.data.ms.delay <= 0)
	{
		Fire1(o, logic);
	}

	MonsterMove(o, logic);

}

void FireLogic(MapObject& o, GameLogic* logic)
{
 	if (!CheckVecs(o, logic))
	{
		// todo: bounce, sparks
		MakeSparks(o, logic);
		o.killme = true;
	}
	else
	{
		o.data.ms.frame+= (1<<16);
		if ((o.data.ms.frame >> 16) >= o.data.ms.shape->size())
		{
			o.data.ms.frame = 0;
		}
	}
}

void KillLogicComp(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	thisobj.killme = true;
}

void Shoot(MapObject& o, GameLogic* logic, int32_t colltype, int32_t collwith, int32_t hitpoints, int32_t damage, int32_t speed, std::vector<Shape>* shape, std::vector<Shape>* spark)
{
	MapObject newobject;

	/*
	shoot	;
	;fire off a bullet...
	;
	;d2 : colltype
	;d3 : collwith
	;d4 : hitpoints
	;d5 : damage
	;d6 : speed
	;a2=bullet shape
	;a3=sparks shape
	;
	addfirst	objects
	beq	.rts
	;
	move	ob_bouncecnt(a5),ob_bouncecnt(a0)
	move	ob_x(a5),ob_x(a0)
	move	ob_y(a5),d0
	add	ob_firey(a5),d0
	move	d0,ob_y(a0)
	move	ob_z(a5),ob_z(a0)
	move.l	#firelogic,ob_logic(a0)
	move.l	#drawshape_1,ob_render(a0)
	move.l	#rts,ob_hit(a0)
	move.l	#killobject,ob_die(a0)
	*/

	newobject.t = 999;
	newobject.data.ms.bouncecnt = o.data.ms.bouncecnt;
	newobject.x = o.x;
	Quick temp;
	temp.SetInt(o.data.ms.firey);
	newobject.y = o.y + temp;
	newobject.z = o.z;
	newobject.data.ms.logic = FireLogic;
	newobject.data.ms.render = 1;
	newobject.data.ms.hit = NullLogicComp;
	newobject.data.ms.die = KillLogicComp;
	/*
	move	d2,ob_colltype(a0)
	move	d3,ob_collwith(a0)
	move	d4,ob_hitpoints(a0)
	move	d5,ob_damage(a0)
	move	d6,ob_movspeed(a0)
	move.l	a2,ob_shape(a0)
	clr	ob_invisible(a0)
	*/
	newobject.data.ms.colltype = colltype;
	newobject.data.ms.collwith = collwith;
	newobject.data.ms.hitpoints = hitpoints;
	newobject.data.ms.damage = damage;
	newobject.data.ms.movspeed = speed;
	newobject.data.ms.shape = shape;
	newobject.data.ms.frame = 0;

	/*
	clr	ob_frame(a0)
	move.l	a3,ob_chunks(a0)
	;
	move	ob_rot(a5),d0
	and	#255,d0
	move.l	camrots(pc),a1
	lea	0(a1,d0*8),a1
	;
	*/
	newobject.data.ms.chunks = spark;
	int16_t camrots[4];
	GloomMaths::GetCamRotRaw(o.data.ms.rot & 255, camrots);
	/*
	move	2(a1),d0
	move	d0,ob_nxvec(a0)
	neg	d0
	muls	d6,d0
	add.l	d0,d0
	move	6(a1),d1
	move	d1,ob_nzvec(a0)
	muls	d6,d1
	add.l	d1,d1
	;
	*/

	newobject.data.ms.nxvec = camrots[1];
	newobject.data.ms.xvec = -(int32_t)(camrots[1])*speed * 2;
	newobject.data.ms.nzvec = camrots[3];
	newobject.data.ms.zvec = (int32_t)(camrots[3])*speed * 2;
	
	/*
	movem.l	d0-d1,ob_xvec(a0)
	;add.l	d0,ob_x(a0)
	;add.l	d1,ob_z(a0)
	;
	move	#32,ob_rad(a0)
	move.l	#32*32,ob_radsq(a0)
	;
	
	*/

	newobject.data.ms.rad = 32;
	newobject.data.ms.radsq = 32 * 32;

	newobject.data.ms.blood = 0;

	logic->AddObject(newobject, true);
}

void TerraLogic2(MapObject& o, GameLogic* logic)
{
	/*
	terralogic2	;
	subq	#1,ob_delay(a5)
	bgt.s	.rts
	;
	move	ob_firerate(a5),ob_delay(a5)
	;
	;OK, to to face player and fire away!
	;
	bsr	pickcalc
	move	d0,ob_rot(a5)
	bsr	calcvecs
	;
	moveq	#4,d2	;colltype
	moveq	#0,d3	;collwith
	moveq	#1,d4	;hitpoints
	moveq	#3,d5	;damage
	moveq	#16,d6	;speed
	moveq	#0,d7	;acceleration!
	lea	bullet4,a2
	lea	sparks4,a3
	;
	bsr	shoot
	;
	move.l	shootsfx3(pc),a0
	moveq	#32,d0
	moveq	#5,d1
	bsr	playsfx
	;
	subq	#1,ob_delay2(a5)
	bgt.s	.rts
	;
	bsr	rnddelay
	move.l	#terralogic,ob_logic(a5)
	;
	*/

	o.data.ms.delay--;

	if (o.data.ms.delay) return;

	o.data.ms.delay = o.data.ms.firerate;
	o.data.ms.rot = logic->PickCalc(o);
	CalcVecs(o);
	Shoot(o, logic, 4, 0, 1, 3, 16, logic->wtable[3].shape, logic->wtable[3].spark);
	SoundHandler::Play(SoundHandler::SOUND_SHOOT3);

	o.data.ms.delay2--;

	if (o.data.ms.delay2) return;
	RndDelay(o);
	o.data.ms.logic = TerraLogic;
}

void TerraLogic(MapObject& o, GameLogic* logic)
{
	/*
	terralogic	;
	move	ob_rot(a5),ob_oldrot(a5)
	subq	#1,ob_delay(a5)
	ble	.fire
	;
	move	ob_delay(a5),d0
	and	#31,d0
	bne	monstermove
	;
	move.l	robotsfx(pc),a0
	moveq	#64,d0
	moveq	#10,d1
	bsr	playsfx
	bra	monstermove
	;
	.fire	;OK, terra goes apeshit! stand there firing off at player!
	;use punchrate as firedelay!
	;
	clr	ob_frame(a5)
	move	#1,ob_delay(a5)
	move	ob_firecnt(a5),ob_delay2(a5)
	move.l	#terralogic2,ob_logic(a5)
	rts
	*/
	o.data.ms.oldrot = o.data.ms.rot;
	o.data.ms.delay--;

	if (o.data.ms.delay > 0)
	{
		if ((o.data.ms.delay & 31) != 0)
		{
			MonsterMove(o, logic);
			return;
		}

		SoundHandler::Play(SoundHandler::SOUND_ROBOT);
		MonsterMove(o, logic);
		return;
	}
	else
	{
		o.data.ms.frame = 0;
		o.data.ms.delay = 1;
		o.data.ms.delay2 = o.data.ms.firecnt;
		o.data.ms.logic = TerraLogic2;
	}
}

void GhoulLogic(MapObject& o, GameLogic* logic)
{
	/*
	ghoullogic	;
	addq	#8,ob_bounce(a5)
	move	ob_bounce(a5),d0
	move.l	camrots(pc),a0
	and	#255,d0
	move	0(a0,d0*8),d0
	ext.l	d0
	lsl.l	#5,d0	;+/- 32
	swap	d0
	add	#-32,d0
	move	d0,ob_y(a5)
	*/
	o.data.ms.bounce += 8;
	int16_t camrots[4];
	GloomMaths::GetCamRotRaw(o.data.ms.bounce&255, camrots);
	int32_t y = camrots[0];
	y <<= 5;
	y >>= 16;
	y -= 32;
	o.y.SetInt(y);
	/*
	;
	bsr	pickcalc
	move	d0,ob_rot(a5)
	;
	subq	#1,ob_delay(a5)
	bgt.s	.skip
	;
	move	#1,ob_frame(a5)
	move.l	#$2000,ob_framespeed(a5)
	moveq	#4,d2	;colltype
	moveq	#0,d3	;collwith
	moveq	#1,d4	;hitpoints
	moveq	#3,d5	;damage
	moveq	#20,d6	;speed
	moveq	#0,d7	;acceleration!
	lea	bullet2,a2
	lea	sparks2,a3
	;
	bsr	shoot
	bsr	rnddelay
	;
	*/

	uint8_t ang = logic->PickCalc(o);
	o.data.ms.rot = ang;
	o.data.ms.delay--;

	if (o.data.ms.delay<=0)
	{
		o.data.ms.frame = 1;
		o.data.ms.framespeed = 0x2000;
		Shoot(o, logic, 4, 0, 2, 3, 20, logic->wtable[1].shape, logic->wtable[1].spark);
		RndDelay(o);
	}

	/*
	.skip	;OK, ghoul moves around ignoring walls!
	;
	;he's pointed at player...how about randomly selected to make
	;this his new movement vector?
	;
	bsr	rndw
	move	ob_movspeed(a5),d1
	lsl	#8,d1
	cmp	d1,d0
	bcc.s	.no
	;
	bsr	calcvecs
	;
	move.l	ghoulsfx(pc),a0
	moveq	#32,d0
	moveq	#-5,d1
	bsr	playsfx
	;
	.no	movem.l	ob_xvec(a5),d0-d1
	add.l	d0,ob_x(a5)
	add.l	d1,ob_z(a5)
	;
	move.l	ob_framespeed(a5),d0
	beq.s	.rts
	add.l	d0,ob_frame(a5)
	cmp	#3,ob_frame(a5)
	bcs.s	.rts
	;
	clr	ob_frame(a5)
	clr.l	ob_framespeed(a5)
	;
	.rts	rts
	*/
	int32_t rndval = GloomMaths::RndW();
	rndval &= 0xFFFF;

	//BCC IS UNSIGNED COMPARISON!
	if ((rndval - (o.data.ms.movspeed>>16<<8))<0)
	{
		SoundHandler::Play(SoundHandler::SOUND_GHOUL);
		CalcVecs(o);
	}

	Quick temp;
	temp.SetVal(o.data.ms.xvec);
	o.x = o.x + temp;

	temp.SetVal(o.data.ms.zvec);
	o.z = o.z + temp;

	if (o.data.ms.framespeed)
	{
		o.data.ms.frame += o.data.ms.framespeed;
	}

	if (o.data.ms.frame >= 0x30000)
	{
		o.data.ms.frame = 0;
		o.data.ms.framespeed = 0;
	}
}

void WeaponGot(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	SoundHandler::Play(SoundHandler::SOUND_TOKEN);

	if (thisobj.data.ms.weapon > otherobj.data.ms.weapon)
	{
		otherobj.data.ms.weapon = thisobj.data.ms.weapon;
		otherobj.data.ms.reload = 5;
	}
	else
	{
		//todo: megaweapon etc.. .
		if (otherobj.data.ms.reload > 1)
		{
			otherobj.data.ms.reload--;
		}
	}
	thisobj.killme = true;
}

void PauseLogic2(MapObject& o, GameLogic* logic)
{
	/*
	pauselogic2	
	subq	#1, ob_hurtwait(a5)
	bgt.s.rts
	clr	ob_frame(a5)
	move.l	ob_oldlogic2(a5), ob_logic(a5)
	move.l	ob_oldhit(a5), ob_hit(a5)
	.rts	rts
	*/
	o.data.ms.hurtwait--;

	if (o.data.ms.hurtwait < 0)
	{
		o.data.ms.frame = 0;
		o.data.ms.logic = o.data.ms.oldlogic2;
		o.data.ms.hit = o.data.ms.oldhit;
	}
}

void HurtObject(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	/*
	hurtobject	move	ob_colltype(a0), d0
		and	#24, d0
		bne.s.rts
		;
	moveq	#23, d7
		bsr	bloodymess
		move	ob_hurtpause(a5), ob_hurtwait(a5)
		beq.rts
		;
	move	#4, ob_frame(a5)
		move.l	ob_logic(a5), ob_oldlogic2(a5)
		move.l	ob_hit(a5), ob_oldhit(a5)
		move.l	#pauselogic2, ob_logic(a5)
		move.l	#rts, ob_hit(a5)
		;
	.rts	rts
	*/

	// this checks so it doesn't flinch on collision with the player (as opposed to the players *bullets*)
	if (!(otherobj.data.ms.colltype & 24))
	{
		BloodyMess(thisobj, logic, 23);

		thisobj.data.ms.hurtwait = thisobj.data.ms.hurtpause;

		if (thisobj.data.ms.hurtpause)
		{
			thisobj.data.ms.frame = 0x40000;
			thisobj.data.ms.oldlogic2 = thisobj.data.ms.logic;
			thisobj.data.ms.oldhit = thisobj.data.ms.hit;
			thisobj.data.ms.logic = PauseLogic2;
			thisobj.data.ms.hit = NullLogicComp;
		}
	}
}

int16_t lastgrunt;

void HurtNGrunt(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	/*
	hurtngrunt	move.l	a0, -(a7)
	bsr	rndw
	and	#3, d0
	cmp	lastgrunt(pc), d0
	bne.s	.new
	addq	#1, d0
	and	#3, d0
	.new	move	d0, lastgrunt
	lea	grunttable(pc), a0
	move.l	0(a0, d0 * 4), a0
	move.l(a0), a0
	moveq	#64, d0
	moveq	#1, d1
	bsr	playsfx
	move.l(a7) + , a0
	;
	*/

	auto random = GloomMaths::RndW()&3;

	if (random == lastgrunt)
	{
		random = (random + 1) & 3;
	}

	lastgrunt = random;

	switch (random)
	{
		case 0:
			SoundHandler::Play(SoundHandler::SOUND_GRUNT);
			break;
		case 1:
			SoundHandler::Play(SoundHandler::SOUND_GRUNT2);
			break;
		case 2:
			SoundHandler::Play(SoundHandler::SOUND_GRUNT3);
			break;
		case 3:
			SoundHandler::Play(SoundHandler::SOUND_GRUNT4);
			break;
	}

	HurtObject(thisobj, otherobj, logic);
}

void ChunkLogic(MapObject& o, GameLogic* logic)
{
	/*
	chunklogic	move	mode(pc),d0
	beq	chunklogic2
	;
	add.l	#$8000,ob_yvec(a5)
	move.l	ob_yvec(a5),d0
	add.l	ob_y(a5),d0
	blt	.skip
	;
	;OK...hit ground!
	;
	bsr	splat
	addlast	gore
	bne.s	.gok
	;
	move.l	gore(pc),a0
	killitem	gore
	addlast	gore
	beq	killobject
	;
	.gok	move	ob_x(a5),go_x(a0)
	move	ob_z(a5),go_z(a0)
	move.l	ob_shape(a5),a1
	move	ob_frame(a5),d0
	add.l	12(a1,d0*4),a1
	move.l	a1,go_shape(a0)
	;
	bra	killobject
	;
	.skip	move.l	d0,ob_y(a5)
	bsr	checkvecs
	beq.s	.rts
	clr.l	ob_xvec(a5)
	clr.l	ob_zvec(a5)
	.rts	rts
	*/

	//TODO meaty mode
	o.data.ms.yvec += 0x8000;
	Quick temp;
	temp.SetVal(o.data.ms.yvec);
	o.y = o.y + temp;

	if (o.y.GetVal() >= 0)
	{
		SoundHandler::Play(SoundHandler::SOUND_SPLAT);
		o.data.ms.logic = NullLogic;
		return;
	}

	if (!CheckVecs(o, logic))
	{
		o.data.ms.xvec = 0;
		o.data.ms.zvec = 0;
	}
}

void BlowChunx(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	/*
	.loop	addlast	objects
	beq	killobject
	movem.l	ob_x(a5),d0-d2
	move.l	#-64<<16,d1
	movem.l	d0-d2,ob_x(a0)
	;
	bsr	bloodspeed3
	move.l	d0,ob_xvec(a0)
	bsr	bloodspeed3
	sub.l	#$40000,d0
	move.l	d0,ob_yvec(a0)
	bsr	bloodspeed3
	move.l	d0,ob_zvec(a0)
	;
	clr	ob_invisible(a0)
	clr	ob_colltype(a0)
	clr	ob_collwith(a0)
	move.l	#chunklogic,ob_logic(a0)
	move.l	a4,ob_shape(a0)
	move.l	#drawshape_1sc,ob_render(a0)
	move	d7,ob_frame(a0)
	move	ob_scale(a5),ob_scale(a0)
	;
	move	an_maxw(a4),d0
	move	d0,ob_rad(a0)
	mulu	d0,d0
	move.l	d0,ob_radsq(a0)
	;
	dbf	d7,.loop
	rts
	*/

	for (size_t frame = 0; frame < logic->objectgraphics->GetGoreShape(thisobj.t).size(); frame++)
	{
		MapObject chunks;
		chunks.x = thisobj.x;
		chunks.y.SetInt(-64);
		chunks.z = thisobj.z;

		chunks.t = 999;
		chunks.data.ms.xvec = BloodSpeed3();
		chunks.data.ms.yvec = BloodSpeed3() - 0x40000;
		chunks.data.ms.zvec = BloodSpeed3();
		chunks.data.ms.colltype = 0;
		chunks.data.ms.collwith = 0;
		chunks.data.ms.render = 1;
		chunks.data.ms.logic = ChunkLogic;
		chunks.data.ms.shape = &logic->objectgraphics->GetGoreShape(thisobj.t);
		chunks.data.ms.frame = frame << 16;
		//chunks.data.ms.shape = scale;
		chunks.data.ms.rad = logic->objectgraphics->maxwidthsgore[thisobj.t];
		chunks.data.ms.radsq = chunks.data.ms.rad*chunks.data.ms.rad;
		chunks.data.ms.blood = 0;

		logic->newobjects.push_back(chunks);
	}
}

void BlowObject(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	SoundHandler::Play(SoundHandler::SOUND_DIE);
	BloodyMess2(thisobj, logic, 31);
	BlowChunx(thisobj, otherobj, logic);
	thisobj.killme = true;
}

void BlowObjectNoChunks(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	SoundHandler::Play(SoundHandler::SOUND_DIE);
	BloodyMess2(thisobj, logic, 31);
	BloodyMess2(thisobj, logic, 15);
	thisobj.killme = true;
}

void HurtTerra(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	/*
		hurtterra	move.l	a0, -(a7)
		move.l	shootsfx2(pc), a0
		moveq	#64, d0
		moveq	#2, d1
		bsr	playsfx
		move.l(a7) + , a0
		bra.s	hurtobject
	*/

	SoundHandler::Play(SoundHandler::SOUND_SHOOT2);
	HurtObject(thisobj, otherobj, logic);
}

void BlowTerra(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	SoundHandler::Play(SoundHandler::SOUND_ROBODIE);
	BlowChunx(thisobj, otherobj, logic);
	thisobj.killme = true;
}

//baldylogic2?
void Baldy2Norm(MapObject& o, GameLogic* logic)
{
	/*
	baldy_tonorm	move.l	ob_movspeed(a5),d0
	lsr.l	#2,d0
	move.l	d0,ob_movspeed(a5)
	;
	move.l	ob_framespeed(a5),d0
	lsr.l	#2,d0
	move.l	d0,ob_framespeed(a5)
	;
	move.l	ob_oldlogic(a5),ob_logic(a5)
	bsr	rnddelay
	;
	bra	monsterfix
	*/
	o.data.ms.movspeed >>= 2;
	o.data.ms.framespeed >>= 2;
	o.data.ms.logic = o.data.ms.oldlogic;
	RndDelay(o);
	MonsterFix(o, logic);
}

void BaldyCharge(MapObject& o, GameLogic* logic)
{
	/*
	baldycharge	;
	;baldy charging at player!
	;
	bsr	checkvecs
	beq	baldy_skip
	;
	baldy_tonorm	move.l	ob_movspeed(a5),d0
	lsr.l	#2,d0
	move.l	d0,ob_movspeed(a5)
	;
	move.l	ob_framespeed(a5),d0
	lsr.l	#2,d0
	move.l	d0,ob_framespeed(a5)
	;
	move.l	ob_oldlogic(a5),ob_logic(a5)
	bsr	rnddelay
	;
	bra	monsterfix
	*/
	if (!CheckVecs(o, logic))
	{
		Baldy2Norm(o, logic);
		return;
	}
	/*
	;
	baldy_skip	;close to player? start throwing punches around!
	;
	bsr	pickcalc
	;
	sub	ob_rot(a5),d0
	cmp	#32,d0
	bgt	baldy_tonorm
	cmp	#-32,d0
	blt	baldy_tonorm
	;

	*/
	int8_t ang = logic->PickCalc(o);
	ang -= o.data.ms.rot;

	if (ang > 32)
	{
		Baldy2Norm(o, logic);
		return;
	}
	if (ang < -32)
	{
		Baldy2Norm(o, logic);
		return;
	}

	// Punching!
	/*
	move.l	a0,ob_washit(a5)
	bsr	checkcoll
	beq	monsternew	;no collisions!
	;
	;go into punch mode!
	;
	move.l	#baldypunch,ob_logic(a5)
	move	ob_punchrate(a5),ob_delay(a5)
	clr.l	ob_frame(a5)
	rts*/

	MapObject playerobj = logic->GetPlayerObj();

	o.data.ms.washit = playerobj.identifier;

	if (!CheckColl(playerobj, o))
	{
		o.data.ms.frame += o.data.ms.framespeed;
		o.data.ms.frame &= 0x3FFFF;
		return;
	}

	o.data.ms.logic = BaldyPunch;
	o.data.ms.delay = o.data.ms.punchrate;
	o.data.ms.frame = 0;
}

void BL2(MapObject& o, GameLogic* logic)
{
	/*
	bl2	bsr	pickcalc
	move	d0,ob_rot(a5)
	;
	move.l	ob_movspeed(a5),d0
	lsl.l	#2,d0
	move.l	d0,ob_movspeed(a5)
	move.l	ob_framespeed(a5),d0
	lsl.l	#2,d0
	move.l	d0,ob_framespeed(a5)
	;
	bsr	calcvecs
	move.l	ob_logic(a5),ob_oldlogic(a5)
	move.l	#baldycharge,ob_logic(a5)
	;
	rts
	*/
	o.data.ms.rot = logic->PickCalc(o);
	o.data.ms.movspeed <<= 2;
	o.data.ms.framespeed <<= 2;
	CalcVecs(o);
	o.data.ms.oldlogic = o.data.ms.logic;
	o.data.ms.logic = BaldyCharge;
}

void BaldyLogic(MapObject& o, GameLogic* logic)
{
	o.data.ms.delay--;

	if (o.data.ms.delay>0)
	{
		MonsterMove(o, logic);
		return;
	}

	BL2(o, logic);
}

int8_t CheckColl(MapObject& o1, MapObject &o2)
{
	/*
	checkcoll	;check for collision between a5, and a0
	;
	move	ob_rad(a5),d1
	move	ob_rad(a0),d2
	add	d1,d2	;r sum
	;
	move	ob_x(a0),d3
	sub	ob_x(a5),d3
	bpl.s	.xpl
	neg	d3
	.xpl	cmp	d2,d3
	bcc.s	.no
	;
	move	ob_z(a0),d4
	sub	ob_z(a5),d4
	bpl.s	.ypl
	neg	d4
	.ypl	cmp	d2,d4
	bcc.s	.no
	;
	mulu	d2,d2
	mulu	d3,d3
	mulu	d4,d4
	add.l	d4,d3
	cmp.l	d2,d3
	bcc.s	.no
	;
	moveq	#-1,d0
	rts
	;
	.no	moveq	#0,d0
	rts
	*/
	int32_t radsum = o1.data.ms.rad + o2.data.ms.rad;

	int32_t dx = o1.x.GetInt() - o2.x.GetInt();
	int32_t dz = o1.z.GetInt() - o2.z.GetInt();

	if (dx < 0) dx = -dx;
	if (dz < 0) dz = -dz;

	if (dx>radsum) return 0;
	if (dz>radsum) return 0;

	dx = dx*dx + dz*dz;
	radsum *= radsum;

	return (dx > radsum) ? 0 : -1;
}

void BaldyPunch(MapObject& o, GameLogic* logic)
{
	/*
	baldypunch	;
	bsr	pickplayer
	bsr	checkcoll
	bne.s	.doit
	;
	clr.l	ob_frame(a5)
	bra	baldy_tonorm
	;
	.doit	subq	#1,ob_delay(a5)
	ble.s	.punch
	rts
	.punch	move	ob_punchrate(a5),ob_delay(a5)
	moveq	#0,d0	;stand frame
	cmp	ob_frame(a5),d0
	bne	.skip
	;
	clr.l	ob_washit(a5)	;punch!
	bsr	calcangle
	move	d0,ob_rot(a5)
	moveq	#5,d0
	.skip	move	d0,ob_frame(a5)
	rts
	*/
	MapObject playerobj = logic->GetPlayerObj();

	if (!CheckColl(o, playerobj))
	{
		o.data.ms.frame = 0;
		Baldy2Norm(o, logic);
		return;
	}

	o.data.ms.delay--;
	if (o.data.ms.delay>0) return;

	 o.data.ms.delay = o.data.ms.punchrate;

	if (o.data.ms.frame == 0)
	{
		o.data.ms.washit = 0;
		o.data.ms.rot = logic->PickCalc(o);
		o.data.ms.frame = 5 << 16;
	}
	else
	{
		o.data.ms.frame = 0;
	}

	
}

void LizardLogic(MapObject& o, GameLogic* logic)
{
	/*
	lizardlogic	;
	subq	#1,ob_delay(a5)
	bgt	monstermove	;charge?
	;
	bsr	pickcalc	;pic player in a0!
	move	ob_x(a5),d0
	sub	ob_x(a0),d0
	muls	d0,d0
	move	ob_z(a5),d1
	sub	ob_z(a0),d1
	muls	d1,d1
	add.l	d1,d0
	cmp.l	#256*256,d0
	bcc.s	bl2
	;
	move.l	lizsfx(pc),a0
	moveq	#32,d0
	moveq	#5,d1
	bsr	playsfx
	;
	bra	bl2
	*/
	o.data.ms.delay--;

	if (o.data.ms.delay>0)
	{
		MonsterMove(o, logic);
		return;
	}

	uint8_t ang = logic->PickCalc(o);

	MapObject player = logic->GetPlayerObj();

	int16_t dx = o.x.GetInt() - player.x.GetInt();
	int16_t dz = o.z.GetInt() - player.z.GetInt();

	int32_t res = (int32_t)dx * (int32_t)dx + (int32_t)dz * (int32_t)dz;

	if (res < (256 * 256))
	{
		SoundHandler::Play(SoundHandler::SOUND_LIZARD);
	}

	BL2(o, logic);
}

void LizHurt(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	/*
	lizhurt	move.l	a0,-(a7)
	move.l	lizhitsfx(pc),a0
	moveq	#64,d0
	moveq	#1,d1
	bsr	playsfx
	move.l	(a7)+,a0
	bra	hurtobject
	*/
	SoundHandler::Play(SoundHandler::SOUND_LIZHIT);
	HurtObject(thisobj, otherobj, logic);
}

void TrollHurt(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	SoundHandler::Play(SoundHandler::SOUND_TROLLHIT);
	HurtObject(thisobj, otherobj, logic);
}

void TrollLogic2(MapObject& o, GameLogic* logic)
{
	/*
	trolllogic2	subq	#1, ob_delay(a5)
		bgt	monstermove; charge ?
		;
	bsr	pickcalc; pic player in a0!
		move	ob_x(a5), d0
		sub	ob_x(a0), d0
		muls	d0, d0
		move	ob_z(a5), d1
		sub	ob_z(a0), d1
		muls	d1, d1
		add.l	d1, d0
		cmp.l	#320 * 320, d0
		bcc	bl2
		;
	move.l	trollsfx(pc), a0
		moveq	#64, d0
		moveq	#5, d1
		bsr	playsfx
		;
	bra	bl2
	*/

	o.data.ms.delay--;

	if (o.data.ms.delay>0)
	{
		MonsterMove(o, logic);
		return;
	}

	uint8_t ang = logic->PickCalc(o);

	MapObject player = logic->GetPlayerObj();

	int16_t dx = o.x.GetInt() - player.x.GetInt();
	int16_t dz = o.z.GetInt() - player.z.GetInt();

	int32_t res = (int32_t)dx * (int32_t)dx + (int32_t)dz * (int32_t)dz;

	if (res < (320 * 320))
	{
		SoundHandler::Play(SoundHandler::SOUND_TROLLMAD);
	}

	BL2(o, logic);
}

void TrollLogic(MapObject& o, GameLogic* logic)
{
	/*trolllogic	
	move	ob_rad(a5),d0
	mulu	#$a000,d0
	swap	d0
	move	d0,ob_rad(a5)
	mulu	d0,d0
	move.l	d0,ob_radsq(a5)
	move.l	#trolllogic2,ob_logic(a5)
	;
	*/

	int32_t newrad = o.data.ms.rad;

	newrad *= 0xa000;
	newrad >>= 16;

	o.data.ms.rad = newrad;
	o.data.ms.radsq = newrad*newrad;
	o.data.ms.logic = TrollLogic2;
	TrollLogic2(o, logic);
}

void PhantomLogic(MapObject& o, GameLogic* logic)
{
	/*
	phantomlogic	;
	move	ob_rot(a5),ob_oldrot(a5)
	subq	#1,ob_delay(a5)
	bgt	monstermove
	;
	bsr	pickcalc
	;
	move	d0,ob_rot(a5)
	bsr	calcvecs
	move	#7,ob_delay(a5)
	move.l	ob_logic(a5),ob_oldlogic(a5)
	move.l	#pauselogic,ob_logic(a5)
	move	#5,ob_frame(a5)
	;
	moveq	#4,d2	;colltype
	moveq	#0,d3	;collwith
	moveq	#1,d4	;hitpoints
	moveq	#3,d5	;damage
	moveq	#20,d6	;speed
	moveq	#0,d7	;acceleration!
	lea	bullet3,a2
	lea	sparks3,a3
	;
	bra	shoot
	*/
	o.data.ms.oldrot = o.data.ms.rot;
	o.data.ms.delay--;

	if (o.data.ms.delay > 0)
	{
		MonsterMove(o, logic);
		return;
	}

	o.data.ms.rot = logic->PickCalc(o);
	CalcVecs(o);
	o.data.ms.delay = 7;
	o.data.ms.oldlogic = o.data.ms.logic;
	o.data.ms.logic = PauseLogic;
	o.data.ms.frame = 5 << 16;
	Shoot(o, logic, 4, 0, 1, 3, 20, logic->wtable[2].shape, logic->wtable[2].spark);
	return;
}

void DemonPause(MapObject& o, GameLogic* logic)
{
	/*
	demonpause	
	move	ob_delay(a5),d0
	move	d0,d1
	and	#4,d0
	sne	d0
	ext	d0
	and	#5,d0	;0 or 5
	move	d0,ob_frame(a5)
	;
	and	#7,d1	;do a fire?
	cmp	#7,d1
	bne.s	.nofire
	;
	*/
	uint32_t frame = (o.data.ms.delay & 4) ? 0 : 0xFFFFFFFF;
	frame &= 5;
	o.data.ms.frame = frame << 16;
	if ((o.data.ms.delay & 7) == 7)
	{
		/*
		move	ob_delay(a5),d0
		lsr	#3,d0
		mulu	#18,d0
		lea	wtable(pc),a0
		moveq	#4,d2	;colltype
		moveq	#0,d3	;collwith
		movem	0(a0,d0),d4-d6	;hits,dam,speed
		mulu	#$c000,d5
		swap	d5	;3/4 damage!
		moveq	#0,d7	;acc
		movem.l	6(a0,d0),a2-a3	;bullets/sparks
		move.l	14(a0,d0),-(a7)	;sfx!
		;
		bsr	shoot
		;
		move.l	(a7)+,a0
		move.l	(a0),a0
		moveq	#32,d0
		moveq	#0,d1
		bsr	playsfx
		*/
		uint32_t wep = (o.data.ms.delay>>3);
		if (wep > 4)//this should never happen?
		{
			wep = 4;
		}
		Shoot(o, logic, 4, 0, logic->wtable[wep].hitpoint, logic->wtable[wep].damage*3/4, logic->wtable[wep].speed, logic->wtable[wep].shape, logic->wtable[wep].spark);
		SoundHandler::Play(SoundHandler::SOUND_SHOOT + wep);
	}
	/*
	;
	.nofire	subq	#1,ob_delay(a5)
	bgt.s	.rts
	;
	bsr	rnddelay
	move.l	ob_oldlogic(a5),ob_logic(a5)
	;
	.rts	rts
	*/
	o.data.ms.delay--;

	if (o.data.ms.delay <= 0)
	{
		o.data.ms.delay = RndDelay(o);
		o.data.ms.logic = o.data.ms.oldlogic;
	}
}

void DemonLogic(MapObject& o, GameLogic* logic)
{

	/*
	demonlogic	;
	move	ob_rot(a5),ob_oldrot(a5)
	subq	#1,ob_delay(a5)
	bgt	monstermove
	;
	bsr	pickcalc
	;
	move	d0,ob_rot(a5)
	bsr	calcvecs
	move	#5<<3-1,ob_delay(a5)
	move.l	ob_logic(a5),ob_oldlogic(a5)
	move.l	#demonpause,ob_logic(a5)
	;
	rts
	*/

	o.data.ms.oldrot = o.data.ms.rot;
	o.data.ms.delay--;

	if (o.data.ms.delay > 0)
	{
		MonsterMove(o, logic);
		return;
	}

	o.data.ms.rot = logic->PickCalc(o);
	CalcVecs(o);
	// passing in a weapon number. Why not HW into demonpause? I thought it was going to be some kind of random weapon selection
	o.data.ms.delay = (5<<3)-1;
	o.data.ms.oldlogic = o.data.ms.logic;
	o.data.ms.logic = DemonPause;
	return;

}