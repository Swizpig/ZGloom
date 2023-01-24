#include "gloommap.h"
#include "gloommaths.h"
#include "gamelogic.h"
#include "monsterlogic.h"
#include "hud.h"

void BaldyPunch(MapObject& o, GameLogic* logic);
int8_t CheckColl(MapObject& o1, MapObject &o2);
void PutFire(MapObject& o, GameLogic* logic);

//used to pass around collision data for bounce logic

static int32_t clostestwall;

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

void MakeSparksQ(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	MakeSparks(thisobj, logic);
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

	uint32_t ang = o.data.ms.rotquick.GetInt() & 255;

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
			clostestwall = zone;
			return false;
		}
		else
		{
			clostestwall = zone;
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

		int16_t compang = (int16_t)(player.data.ms.rotquick.GetInt()&0xFF) - ang;

		if (compang < 0) compang = -compang;

		if ((compang>64) && (compang < 192))
		{
			o.data.ms.rotquick.SetInt(o.data.ms.oldrot);
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

	o.data.ms.rotquick.SetInt(ang);
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
		o.data.ms.rotquick.SetInt(((GloomMaths::RndW() > 0) ? 64 : -64) + o.data.ms.rotquick.GetInt());
		CalcVecs(o);
		if (CheckVecs(o, logic)) goto good;

		o.data.ms.rotquick.SetInt(128 + o.data.ms.rotquick.GetInt());
		CalcVecs(o);
		if (CheckVecs(o, logic)) goto good;

		o.data.ms.rotquick.SetInt(o.data.ms.oldrot + 128);
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
		o.data.ms.rotquick.SetInt(((GloomMaths::RndW() > 0) ? 64 : -64) + o.data.ms.rotquick.GetInt());
		CalcVecs(o);
		if (CheckVecs(o, logic)) goto good;

		o.data.ms.rotquick.SetInt(128 + o.data.ms.rotquick.GetInt());
		CalcVecs(o);
		if (CheckVecs(o, logic)) goto good;

		o.data.ms.rotquick.SetInt(o.data.ms.oldrot + 128);
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

	o.data.ms.oldrot = o.data.ms.rotquick.GetInt();
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

void CalcBounce(MapObject& o, GameLogic* logic)
{

	/*
		calcbounce; calculate bounce vector...poly in a4, obj in a5
		;
		; R = 2 N(N dot V) - V
		;
		; where R = reflect vector, N = normal to poly, V = original vector
		;
		subq	#1, ob_bouncecnt(a5)
		bge.s.nok
		bsr	makesparksq
		bra	killobject
		.nok;
		move.l	closewall(pc), a4
		movem	ob_nxvec(a5), d0 - d1; normalized dir
		movem	zo_na(a4), d2 - d3; normal to poly
		neg	d2
	*/

	bool done = false;
	while (!done)
	{
		o.data.ms.bouncecnt--;

		if (o.data.ms.bouncecnt < 0)
		{
			o.killme = true;
			MakeSparks(o, logic);
			return;
		}

		int32_t nxvec, nzvec;
		int32_t na, nb;

		nxvec = o.data.ms.nxvec;
		nzvec = o.data.ms.nzvec;
		logic->GetNorm(clostestwall, na, nb);
		na = -na;

		/*
			;
			; calc dot product :
			;
			move	d0, d4
			muls	d2, d4
			move	d1, d5
			muls	d3, d5
			add.l	d5, d4
			add.l	d4, d4
			swap	d4; dot product ?

			*/

		int32_t dp = nxvec*na + nzvec*nb;
		dp += dp;
		dp >>= 16;

		/*
			;
			muls	d4, d2
			lsl.l	#2, d2
			swap	d0
			clr	d0
			sub.l	d0, d2
			swap	d2
			;
			muls	d4, d3
			lsl.l	#2, d3
			swap	d1
			clr	d1
			sub.l	d1, d3
			swap	d3
			;
			*/

		na *= dp;
		na <<= 2;
		nxvec <<= 16;
		nxvec &= 0xFFFF0000;
		na -= nxvec;
		na >>= 16;

		nb *= dp;
		nb <<= 2;
		nzvec <<= 16;
		nzvec &= 0xFFFF0000;
		nb -= nzvec;
		nb >>= 16;
		/*
			movem	d2 - d3, ob_nxvec(a5)
			;
			neg	d2
			muls	ob_movspeed(a5), d2
			add.l	d2, d2
			muls	ob_movspeed(a5), d3
			add.l	d3, d3
			;
			movem.l	d2 - d3, ob_xvec(a5)
			;
			bsr	checkvecs
			beq.s	putfire
			bra	calcbounce
		*/

		o.data.ms.nxvec = na;
		o.data.ms.nzvec = nb;

		na = -na;

		na *= o.data.ms.movspeed;
		na += na;

		nb *= o.data.ms.movspeed;
		nb += nb;

		o.data.ms.xvec = na;
		o.data.ms.zvec = nb;

		if (CheckVecs(o, logic))
		{
			PutFire(o, logic);
			done = true;
		}
	}
}

void FireLogic(MapObject& o, GameLogic* logic)
{
 	if (!CheckVecs(o, logic))
	{
		CalcBounce(o, logic);
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

void PutFire(MapObject& o, GameLogic* logic)
{
	o.data.ms.frame += (1 << 16);
	if ((o.data.ms.frame >> 16) >= o.data.ms.shape->size())
	{
		o.data.ms.frame = 0;
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
	GloomMaths::GetCamRotRaw(o.data.ms.rotquick.GetInt() & 255, camrots);
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
	o.data.ms.rotquick.SetInt(logic->PickCalc(o));
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
	o.data.ms.oldrot = o.data.ms.rotquick.GetInt();
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
	o.data.ms.rotquick.SetInt(ang);
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

	if (thisobj.data.ms.weapon != otherobj.data.ms.weapon)
	{
		otherobj.data.ms.messtimer = -127;
		otherobj.data.ms.mess = Hud::MESSAGES_NEW_WEAPON;
		otherobj.data.ms.weapon = thisobj.data.ms.weapon;
		otherobj.data.ms.reload = 5;
		otherobj.data.ms.mega = 0;
	}
	else
	{
		//megaweapon etc.. .
		if (otherobj.data.ms.reload > 1)
		{
			otherobj.data.ms.messtimer = -127;
			otherobj.data.ms.mess = (otherobj.data.ms.reload==2)?Hud::MESSAGES_WEAPON_BOOST_FULL : Hud::MESSAGES_WEAPON_BOOST;
			otherobj.data.ms.reload--;
		}
		else
		{
			otherobj.data.ms.messtimer = -127;
			otherobj.data.ms.mess = Hud::MESSAGES_MEGA_WEAPON_BOOST;
			otherobj.data.ms.mega += 250;

			if (otherobj.data.ms.mega >= (750 + 125))
			{
				otherobj.data.ms.mess = Hud::MESSAGES_ULTRA_MEGA_OVERKILL;
			}
		}
	}
	thisobj.killme = true;
}

void InvisGot(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	SoundHandler::Play(SoundHandler::SOUND_TOKEN);

	otherobj.data.ms.messtimer = -127;
	otherobj.data.ms.mess = Hud::MESSAGES_INVISIBILITY;
	otherobj.data.ms.invisible += 1500;
	
	thisobj.killme = true;
}

void ThermoGot(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	SoundHandler::Play(SoundHandler::SOUND_TOKEN);

	otherobj.data.ms.messtimer = -127;
	otherobj.data.ms.mess = Hud::MESSAGES_THERMO;
	otherobj.data.ms.thermo += 1500;

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
	ang -= o.data.ms.rotquick.GetInt();

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
	o.data.ms.rotquick.SetInt(logic->PickCalc(o));
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
		o.data.ms.rotquick.SetInt(logic->PickCalc(o));
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
	o.data.ms.oldrot = o.data.ms.rotquick.GetInt();
	o.data.ms.delay--;

	if (o.data.ms.delay > 0)
	{
		MonsterMove(o, logic);
		return;
	}

	o.data.ms.rotquick.SetInt(logic->PickCalc(o));
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
		SoundHandler::Play(logic->wtable[wep].sound);
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

	o.data.ms.oldrot = o.data.ms.rotquick.GetInt();
	o.data.ms.delay--;

	if (o.data.ms.delay > 0)
	{
		MonsterMove(o, logic);
		return;
	}

	o.data.ms.rotquick.SetInt(logic->PickCalc(o));
	CalcVecs(o);
	// passing in a weapon number. Why not HW into demonpause? I thought it was going to be some kind of random weapon selection
	// because, YOU IDIOT, the continual reduction of the "delay" fields allows the demon to fire multiple different kinds of bullet as it ticks down
	o.data.ms.delay = (5<<3)-1;
	o.data.ms.oldlogic = o.data.ms.logic;
	o.data.ms.logic = DemonPause;
	return;
}

void HealthGot(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	SoundHandler::Play(SoundHandler::SOUND_TOKEN);

	otherobj.data.ms.hitpoints += 5;
	if (otherobj.data.ms.hitpoints > 25) otherobj.data.ms.hitpoints = 25;

	otherobj.data.ms.messtimer = -127;
	otherobj.data.ms.mess = Hud::MESSAGES_HEALTH_BONUS;

	thisobj.killme = true;
}

void BouncyLogic(MapObject& o, GameLogic* logic)
{
	static const int frames[4] = { 3, 4, 3, 5 };

	/*
	bouncylogic	
	addq	#1, ob_delay(a5)
	move	ob_delay(a5), d0
	lsr	#1, d0
	and	#3, d0
	move.bnc(pc, d0 * 2), ob_frame(a5)
	rts
	.bnc	dc	3, 4, 3, 5*/

	o.data.ms.delay++;
	o.data.ms.frame = frames[(o.data.ms.delay >> 1) & 3] << 16;
}

void BouncyGot(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	/*
	bouncygot	bsr	playtsfx
	cmp	#3,ob_bouncecnt(a0)
	bcc.s	.rts
	addq	#1,ob_bouncecnt(a0)
	move.l	a5,-(a7)
	move.l	a0,a5
	bsr	message
	dc.b	'bouncy bullets!',0
	even
	move.l	(a7)+,a5
	bra	killobject
	.rts	rts

	*/

	SoundHandler::Play(SoundHandler::SOUND_TOKEN);

	if (otherobj.data.ms.bouncecnt < 3)
	{
		otherobj.data.ms.bouncecnt++;

		otherobj.data.ms.messtimer = -127;
		otherobj.data.ms.mess = Hud::MESSAGES_BOUNCY;

		thisobj.killme = true;
	}
}

void PlayerDead(MapObject& o, GameLogic* logic)
{
	/*
	playerdead	bsr	getcntrl
	;
	subq	#1,ob_delay(a5)
	bgt.s	.rts
	;
	cmp	#2,gametype
	bne.s	.notcom
	;
	;combat game!
	;
	move	#4,finished2
	move	#1,ob_pixsizeadd(a5)
	move.l	#rts,ob_logic(a5)
	bsr	getother
	move	#1,ob_pixsizeadd(a0)
	.rts	rts
	;
	.notcom	;
	tst	ob_lives(a5)
	beq.s	.dead
	move.l	#waitrestart,ob_logic(a5)
	rts
	;
	.dead	move.l	#rts,ob_logic(a5)
	tst	gametype
	bne.s	.not1p
	.allover	move	#2,finished
	rts
	;
	.not1p	;OK, I'm all out of lives...what about other guy...
	bsr	getother
	tst	ob_lives(a0)
	beq.s	.allover
	rts
	*/
	o.data.ms.delay--;

	if (o.data.ms.delay) return;

	logic->ResetPlayer(o);
}

void PlayerDeath(MapObject& o, GameLogic* logic)
{
	/*
	playerdeath	bsr	getcntrl
		;
	addq	#4, ob_rot(a5)
		addq	#4, ob_eyey(a5)
		cmp	# - 32, ob_eyey(a5)
		blt.rts
		;

	*/
	o.data.ms.rotquick.SetInt(o.data.ms.rotquick.GetInt() + 4);
	o.data.ms.eyey += 4;

	if (o.data.ms.eyey < -32)
	{
		return;
	}
	/*
	move	# - 32, ob_eyey(a5)
	move.l	#playerdead, ob_logic(a5)
	move	#63, ob_delay(a5)
		;
	.... combat game logic follows. TODO: Lives check
	*/

	o.data.ms.eyey = -32;
	o.data.ms.logic = PlayerDead;
	o.data.ms.delay = 63;
}

void PlayerDie(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	/*
	clr	ob_hitpoints(a5)
	st	ob_update(a5)
	move.l	#playerdeath, ob_logic(a5)
	clr	ob_colltype(a5)
	clr	ob_collwith(a5)
	*/

	thisobj.data.ms.hitpoints = 0;
	thisobj.data.ms.logic = PlayerDeath;

	thisobj.data.ms.colltype = 0;
	thisobj.data.ms.collwith = 0;
}

void DeathBounce(MapObject& o, GameLogic* logic)
{
	/*
	deathbounce	addq	#4, ob_bounce(a5)
		move	ob_bounce(a5), d0
		move.l	camrots(pc), a0
		and	#255, d0
		move	0(a0, d0 * 8), d0
		ext.l	d0
		lsl.l	#5, d0; +/ -64
		swap	d0
		add	#  - 48, d0
		move	d0, ob_y(a5)
		rts
	*/

	o.data.ms.bounce += 4;
	int16_t camrots[4];
	GloomMaths::GetCamRotRaw(o.data.ms.bounce&255, camrots);
	int32_t  ang = camrots[0];
	ang <<= 5;
	ang >>= 16;
	ang -= 48;
	o.y.SetInt(ang);
}

void DeathAnim(MapObject& o, GameLogic* logic)
{
	/*
	deathanim	move.l	ob_framespeed(a5), d0
		add.l	d0, ob_frame(a5)
		cmp.l	#$8000, ob_frame(a5)
		blt.s.fix
		cmp.l	#$28000, ob_frame(a5)
		blt.s.fok
		.fix	neg.l	d0
		add.l	d0, ob_frame(a5)
		move.l	d0, ob_framespeed(a5)
		;
	.fok	rts
	*/
	int32_t speed = o.data.ms.framespeed;
	o.data.ms.frame += speed;

	if ((o.data.ms.frame < 0x8000) || (o.data.ms.frame >= 0x28000))
	{
		speed = -speed;
		o.data.ms.frame += speed;
		o.data.ms.framespeed = speed;
	}
}

void DeathCharge(MapObject& o, GameLogic* logic)
{
	/*
	deathcharge	bsr	deathbounce
	bsr	deathanim
	;
	bsr	pickcalc
	move	ob_rot(a5),d1
	and	#255,d1
	sub	d1,d0	;am I near?
	bpl.s	.ansk
	neg	d0
	.ansk	cmp	#128,d0
	bcc.s	.hit
	bsr	checkvecs
	bne.s	.hit2
	rts
	.hit2	add	#128,ob_rot(a5)
	.hit	move.l	#deathheadlogic,ob_logic(a5)
	move.l	#$8000,ob_frame(a5)
	bra	rnddelay
	*/
	DeathBounce(o, logic);
	DeathAnim(o, logic);

	int32_t ang = (logic->PickCalc(o) & 255) - (o.data.ms.rotquick.GetInt() & 255);

	if (ang < 0) ang = -ang;

	if (ang < 128)
	{
		if (CheckVecs(o, logic))
		{
			return;
		}
		else
		{
			o.data.ms.rotquick.SetInt(o.data.ms.rotquick.GetInt() +  128);
			o.data.ms.logic = DeathLogic;
			o.data.ms.frame = 0x8000;
			RndDelay(o);
		}
	}
	else
	{
		o.data.ms.logic = DeathLogic;
		o.data.ms.frame = 0x8000;
		RndDelay(o);
	}
}

void DeathLogic(MapObject& o, GameLogic* logic)
{
	/*
	;cruises around rotating at speed ob_delay
	;
	bsr	deathbounce
	;
	bsr	checkvecs
	bne.s	.hit
	;
	;charge player?
	;
	bsr	pickcalc	;find angle to player
	move	ob_rot(a5),d1
	and	#255,d1
	sub	d0,d1	;am I near?
	bpl.s	.ansk
	neg	d1
	.ansk	cmp	#16,d1
	bcc.s	.notnear
	;
	;OK! chargaroony!
	;
	move	d0,ob_rot(a5)
	move.l	#deathcharge,ob_logic(a5)
	bra	calcvecs
	.hit	add	#128,ob_rot(a5)
	bsr	rnddelay
	.notnear	move	ob_delay(a5),d0
	add	d0,ob_rot(a5)
	bsr	calcvecs
	rts
	*/
	DeathBounce(o, logic);

	if (CheckVecs(o, logic))
	{
		int32_t ang = (logic->PickCalc(o)&255) - (o.data.ms.rotquick.GetInt()&255);

		if (ang < 0) ang = -ang;
		
		if (ang < 16)
		{
			o.data.ms.rotquick.SetInt(logic->PickCalc(o) & 255);
			o.data.ms.logic = DeathCharge;
			CalcVecs(o);
			return;
		}
		else
		{
			o.data.ms.rotquick.SetInt(o.data.ms.rotquick.GetInt() + o.data.ms.delay);
			CalcVecs(o);
		}
	}
	else
	{
		o.data.ms.rotquick.SetInt(o.data.ms.rotquick.GetInt() + 128);
		RndDelay(o);
		o.data.ms.rotquick.SetInt(o.data.ms.rotquick.GetInt() + o.data.ms.delay);
		CalcVecs(o);
	}
}

void BlowDeath(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	if (logic->GetSucker())
	{
		logic->SetSucker(0);
		logic->SetSucking(0);
	}

	BlowObjectNoChunks(thisobj, otherobj, logic);
}

void AddSoul(int num, uint8_t ang, MapObject& o, MapObject& pobj, GameLogic* logic)
{
	int16_t camrots[4];

	GloomMaths::GetCamRotRaw(ang, camrots);
	/*

	addsoul; d7 times!
		;
	addlast	blood
		beq.rts
		;
	move	2(a3), d2
		ext.l	d2
		lsl.l	#5, d2
		neg.l	d2
		move	6(a3), d3
		ext.l	d3
		lsl.l	#5, d3
		;
	move.l	d2, bl_xvec(a0)
		move.l	a5, bl_yvec(a0)
		move.l	d3, bl_zvec(a0)
		;
	swap	d2
		swap	d3
		add	d2, d2
		add	d3, d3
		add	ob_x(a2), d2
		add	ob_z(a2), d3
		;
	bsr	rndw
		and	#63, d0
		sub	#32, d0
		add	d2, d0
		move	d0, bl_x(a0)
		;
	bsr	rndw
		and	#63, d0
		sub	#32, d0
		add	#110, d0
		move	d0, bl_y(a0); >0 = funny blood!
		;
	bsr	rndw
		and	#63, d0
		sub	#32, d0
		add	d3, d0
		move	d0, bl_z(a0)
		;
	bsr	rndw
		and	#1, d0
		move	soulcols(pc, d0 * 2), bl_color(a0)
		;
	dbf	d7, addsoul
		;
	.rts	rts
	*/

	for (int i = 0; i < num; i++)
	{
		Blood bloodobj;
		int32_t xvec = camrots[1];
		xvec <<= 5;
		xvec = -xvec;

		int32_t zvec = camrots[3];
		zvec <<= 5;

		bloodobj.xvec.SetVal(xvec);
		bloodobj.zvec.SetVal(zvec);
		bloodobj.dest = o.identifier;

		xvec >>= 16;
		zvec >>= 16;
		xvec *= 2;
		zvec *= 2;

		xvec += pobj.x.GetInt();
		zvec += pobj.z.GetInt();

		bloodobj.x.SetInt(xvec + (GloomMaths::RndW() & 63) - 32);
		bloodobj.y.SetInt(110  + (GloomMaths::RndW() & 63) - 32);
		bloodobj.z.SetInt(zvec + (GloomMaths::RndW() & 63) - 32);

		bloodobj.color = (GloomMaths::RndW() & 1) ? 0x0ff : 0x0f0;

		logic->AddBlood(bloodobj);
	}
}

void DeathSuck(MapObject& o, GameLogic* logic)
{
	/*
	deathsuck	;death head sucking out a players soul!
	;
	bsr	deathbounce
	bsr	deathanim
	subq	#1,ob_delay(a5)
	bgt.s	.more
	move	ob_oldrot(a5),ob_rot(a5)
	move.l	ob_oldlogic(a5),ob_logic(a5)
	move.l	#hurtdeath,ob_hit(a5)
	clr.l	sucker
	clr.l	sucking
	bra	rnddelay
	;
	.more	move.l	sucking(pc),a0
	move.l	a0,a2
	bsr	calcangle	;point at player!
	move	d0,ob_rot(a5)
	;
	add	#128,d0
	and	#255,d0
	move.l	camrots(pc),a3
	lea	0(a3,d0*8),a3
	;
	move.l	a3,suckangle
	;
	;calc x/z vecs
	;
	moveq	#3,d7
	bsr	addsoul
	;
	.rts	rts
	*/

	DeathBounce(o, logic);
	DeathAnim(o, logic);

	o.data.ms.delay--;

	if (o.data.ms.delay <= 0)
	{
		o.data.ms.rotquick.SetInt(o.data.ms.oldrot);
		o.data.ms.logic = o.data.ms.oldlogic;
		o.data.ms.hit = HurtDeath;
		logic->SetSucker(0);
		logic->SetSucking(0);
		RndDelay(o);
		return;
	}

	o.data.ms.rotquick.SetInt(logic->PickCalc(o));

	uint8_t ang = (o.data.ms.rotquick.GetInt() + 128);
	logic->SetSuckAngle(ang);

	MapObject pobj = logic->GetPlayerObj();

	AddSoul(3, ang, o, pobj, logic);
}

void HurtDeath(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	/*
	hurtdeath	;OK! death head hit!
	;
	;point at player, and start to suck his soul!
	;
	move.l	sucking(pc),d0
	bne.s	.rts
	;
	bsr	pickplayer
	cmp.l	#playerlogic,ob_logic(a0)
	bne.s	.rts
	;
	move.l	a0,sucking
	move.l	a5,sucker
	;
	move	ob_rot(a5),ob_oldrot(a5)
	move.l	ob_logic(a5),ob_oldlogic(a5)
	move.l	#deathsuck,ob_logic(a5)
	move.l	#rts,ob_hit(a5)
	move	#64,ob_delay(a5)
	bra.s	deathsuck
	;
	.rts	rts
	*/

	if (logic->GetSucking()) return;
	MapObject pobj = logic->GetPlayerObj();

	if (pobj.data.ms.logic != NullLogic) return;

	logic->SetSucking(pobj.identifier);
	logic->SetSucker(thisobj.identifier);

	thisobj.data.ms.oldrot = thisobj.data.ms.rotquick.GetInt();
	thisobj.data.ms.oldlogic = thisobj.data.ms.logic;

	thisobj.data.ms.logic = DeathSuck;
	thisobj.data.ms.hit = NullLogicComp;
	thisobj.data.ms.delay = 64;
	DeathSuck(thisobj, logic);
}

void DragonDead(MapObject& o, GameLogic* logic)
{
	o.data.ms.delay--;

	if (o.data.ms.delay <= 0)
	{
		logic->WereDoneHere();
	}
}

void BlowDragon(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	/*
	blowdragon	;same, but messier...
	;
	;loud!
	;
	move.l	diesfx(pc),a0
	moveq	#64,d0
	moveq	#50,d1
	bsr	playsfx
	move.l	diesfx(pc),a0
	moveq	#64,d0
	moveq	#50,d1
	bsr	playsfx
	move.l	robodiesfx(pc),a0
	moveq	#64,d0
	moveq	#50,d1
	bsr	playsfx
	move.l	robodiesfx(pc),a0
	moveq	#64,d0
	moveq	#50,d1
	bsr	playsfx
	;
	moveq	#63,d7
	bsr	bloodymess2
	;
	move.l	ob_chunks(a5),a4
	bsr	blowchunx
	bsr	blowchunx
	bsr	blowchunx
	bsr	blowchunx
	;
	move.l	#dragondead,ob_logic(a5)
	move.l	#rts,ob_render(a5)
	clr	ob_colltype(a5)
	clr	ob_collwith(a5)
	move	#127,ob_delay(a5)
	rts
	*/

	SoundHandler::Play(SoundHandler::SOUND_DIE);
	SoundHandler::Play(SoundHandler::SOUND_DIE);
	SoundHandler::Play(SoundHandler::SOUND_ROBODIE);
	SoundHandler::Play(SoundHandler::SOUND_ROBODIE);

	BloodyMess2(thisobj, logic, 63);

	BlowChunx(thisobj, otherobj, logic);
	BlowChunx(thisobj, otherobj, logic);
	BlowChunx(thisobj, otherobj, logic);
	BlowChunx(thisobj, otherobj, logic);

	thisobj.data.ms.logic = DragonDead;
	thisobj.data.ms.render = 0;
	thisobj.data.ms.colltype = 0;
	thisobj.data.ms.collwith = 0;
	thisobj.data.ms.delay = 127;
}

void DragonAnim(MapObject& o)
{
	o.data.ms.frame += o.data.ms.framespeed;
	o.data.ms.frame &= 0x3FFFF;
}

void GetObRot(MapObject& o)
{
	/*
	getobrot	move	ob_rotspeed(a5),d0
	bne.s	.addr
	;
	;OK, randomly left/rite!
	;
	bsr	rndw
	and	#1,d0
	bne.s	.addr2
	moveq	#-1,d0
	;
	.addr2	lsl	#2,d0
	move	d0,ob_rotspeed(a5)
	;
	.addr	rts
	*/

	if (!o.data.ms.rotspeed)
	{
		o.data.ms.rotspeed = (GloomMaths::RndW() & 1) ? -0x40000 : 40000;
	}
}

void BlowDB(MapObject& thisobj, MapObject& otherobj, GameLogic* logic)
{
	MakeSparksQ(thisobj, otherobj, logic);
	thisobj.killme = true;
}

void HomeInLogic(MapObject& o, GameLogic* logic)
{

	/*
	homeinlogic	bsr	checkvecs
	bne.s	blowdb
	bsr	pickcalc	;find angle to player!
	move.l	camrots(pc),a0
	lea	0(a0,d0*8),a0
	*/
	
	if (!CheckVecs(o, logic))
	{
		BlowDB(o, o, logic);
		return;
	}

	int16_t camrots[4];

	GloomMaths::GetCamRotRaw(logic->PickCalc(o), camrots);
	/*
	move	2(a0),d4	;x acc.
	neg	d4
	ext.l	d4
	lsl.l	#2,d4
	move	6(a0),d5	;z acc.
	ext.l	d5
	lsl.l	#2,d5
	;
	*/
	int32_t xacc, zacc;
	xacc = -camrots[1];
	xacc <<= 2;
	zacc = camrots[3];
	zacc <<= 2;
	/*
	add.l	ob_xvec(a5),d4
	move.l	d4,d0
	bpl.s	.pl1
	neg.l	d0
	.pl1	cmp.l	#$200000,d0	;max speed
	bcc.s	.sk1
	move.l	d4,ob_xvec(a5)
	;
	.sk1	add.l	ob_zvec(a5),d5
	move.l	d5,d0
	bpl.s	.pl2
	neg.l	d0
	.pl2	cmp.l	#$200000,d0
	bcc.s	.sk2
	move.l	d5,ob_zvec(a5)
	.sk2	;
	bra	putfire
	*/
	o.data.ms.xvec += xacc;
	if (abs(o.data.ms.xvec) > 0x200000) o.data.ms.xvec -= xacc;

	o.data.ms.zvec += zacc;
	if (abs(o.data.ms.zvec) > 0x200000) o.data.ms.zvec -= zacc;
	PutFire(o, logic);
}

void DragonFire(MapObject& o, GameLogic* logic) // starring Sylvester McCoy as Doctor Who
{
	/*
	dragonfire	;dragon fires at you!
	;
	;d2 : colltype
	;d3 : collwith
	;d4 : hitpoints
	;d5 : damage
	;d6 : speed
	;a2=bullet shape
	;a3=sparks shape
	;
	subq	#1,ob_delay(a5)
	bpl	.rts
	move	ob_delay(a5),d0
	cmp	#-16*8,d0
	bgt.s	.try
	move	#47,ob_delay(a5)
	rts
	.try	and	#7,d0
	bne	.rts
	;
	*/
	o.data.ms.delay--;
	if (o.data.ms.delay > 0) return;
	if (o.data.ms.delay > (-16 * 8))
	{
		if ((o.data.ms.delay & 7) != 0)
		{
			return;
		}
	}
	else
	{
		o.data.ms.delay = 47;
		return;
	}
	/*
	.fire	moveq	#0,d2	;colltype
	moveq	#24+3,d3	;collwith - p1/p2/bullets
	moveq	#1,d4	;hitpoints
	moveq	#3,d5	;damage
	moveq	#16,d6	;speed
	lea	bullet5,a2
	lea	sparks5,a3
	;
	addlast	objects
	beq	.rts
	*/
	MapObject b;
	b.t = 999;
	b.data.ms.blood = 0;
	b.data.ms.colltype = 0;
	b.data.ms.collwith = 24 + 3;
	b.data.ms.hitpoints = 1;
	b.data.ms.damage = 3;
	b.data.ms.movspeed = 16;
	b.data.ms.shape = logic->wtable[4].shape;
	b.data.ms.chunks = logic->wtable[4].spark;
	/*
	;
	move	ob_bouncecnt(a5),ob_bouncecnt(a0)
	move	ob_x(a5),ob_x(a0)
	move	ob_y(a5),d0
	add	ob_firey(a5),d0
	move	d0,ob_y(a0)
	move	ob_z(a5),ob_z(a0)
	move.l	#homeinlogic,ob_logic(a0)
	move.l	#drawshape_1,ob_render(a0)
	move.l	#makesparksq,ob_hit(a0)
	move.l	#blowdb,ob_die(a0)
	*/
	b.data.ms.bouncecnt = o.data.ms.bouncecnt;
	b.x = o.x;
	b.y.SetInt(o.y.GetInt() + o.data.ms.firey);
	b.z = o.z;
	b.data.ms.logic = HomeInLogic;
	b.data.ms.render = 1;
	b.data.ms.hit = MakeSparksQ;
	b.data.ms.die = BlowDB;
	/*
	move	d2,ob_colltype(a0)
	move	d3,ob_collwith(a0)
	move	d4,ob_hitpoints(a0)
	move	d5,ob_damage(a0)
	move	d6,ob_movspeed(a0)
	move.l	a2,ob_shape(a0)
	clr	ob_invisible(a0)
	clr	ob_frame(a0)
	move.l	a3,ob_chunks(a0)
	*/
	// todo INVISIBLE
	b.data.ms.frame = 0;
	/*
	;
	move	ob_rot(a5),d0
	and	#255,d0
	move.l	camrots(pc),a1
	lea	0(a1,d0*8),a1
	;
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
	movem.l	d0-d1,ob_xvec(a0)
	;
	move	#32,ob_rad(a0)
	move.l	#32*32,ob_radsq(a0)
	;
	.rts	rts
	*/

	int16_t camrots[4];
	GloomMaths::GetCamRotRaw(o.data.ms.rotquick.GetInt() & 0xFF, camrots);

	b.data.ms.nxvec = camrots[1];
	b.data.ms.xvec = -(int32_t)camrots[1];
	b.data.ms.xvec *= 16;
	b.data.ms.xvec += o.data.ms.xvec;
	
	b.data.ms.nzvec = camrots[3];
	b.data.ms.zvec = (int32_t)camrots[1];
	b.data.ms.zvec *= 16;
	b.data.ms.zvec += o.data.ms.zvec;

	b.data.ms.rad = 32;
	b.data.ms.radsq = 32;

	logic->AddObject(b, false);
}

void DragonLogic(MapObject& o, GameLogic* logic)
{
	/*
	dragonlogic	;OK! end of game baddy!
	;
	;how about cruising around in a circle a-la
	;deathhead!
	;
	bsr	dragonanim
	bsr	dragonfire
	bsr	checkvecs
	beq.s	.nohit
	;
	;OK, dragon has hit a wall...rot him around till he's clear!
	;
	bsr	getobrot
	lsl	#2,d0
	add	d0,ob_rot(a5)
	bra	calcvecs
	*/
	DragonAnim(o);
	DragonFire(o, logic);

	if (!CheckVecs(o, logic))
	{
		GetObRot(o);
		o.data.ms.rotquick.SetVal(o.data.ms.rotquick.GetVal() + (o.data.ms.rotspeed << 2));
		CalcVecs(o);
		return;
	}
	/*
	.nohit	;
	bsr	pickcalc
	move	ob_rot(a5),d1
	and	#255,d1
	sub	d0,d1	;am I near?
	bpl.s	.ansk
	neg	d1
	*/
	int16_t ang = (o.data.ms.rotquick.GetInt()&255)-logic->PickCalc(o);

	if (ang < 0) ang = -ang;
	/*
	.ansk	moveq	#6,d0
	tst	ob_rotspeed(a5)
	bne.s	.sh
	moveq	#24,d0
	*/
	int16_t d0 = o.data.ms.rotspeed ? 6 : 24;

	/*
	.sh	cmp	d0,d1
	bcs.s	.near
	;
	;not pointed at player!
	bsr	getobrot
	add	d0,ob_rot(a5)
	bra	calcvecs
	;
	.near	tst	ob_rotspeed(a5)
	beq.s	.near2
	clr	ob_rotspeed(a5)	;towards player!
	;
	move.l	dragonsfx(pc),a0
	moveq	#64,d0
	moveq	#20,d1
	bsr	playsfx
	;
	.near2	rts
	*/
	if (ang > d0)
	{
		GetObRot(o);
		o.data.ms.rotquick.SetVal(o.data.ms.rotquick.GetVal() + o.data.ms.rotspeed);
		CalcVecs(o);
		return;
	}

	if (o.data.ms.rotspeed == 0) return;
	o.data.ms.rotspeed = 0;
	SoundHandler::Play(SoundHandler::SOUND_DRAGON);
}
