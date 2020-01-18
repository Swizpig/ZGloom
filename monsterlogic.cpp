#include "gloommap.h"
#include "gloommaths.h"
#include "gamelogic.h"
#include "monsterlogic.h"

void NullLogic(MapObject& o, GameLogic* logic)
{
	return;
}

void WeaponLogic(MapObject& o, GameLogic* logic)
{
	// TODO: Sparks, frames
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

	o.y = (camrots[1] >> 8);

	o.data.ms.frame += (1 << 16);
	if ((o.data.ms.frame >> 16) >= o.data.ms.shape->size()) o.data.ms.frame = 0;
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

	Shoot(o, logic, 4, 0, 1, 1, 20, logic->wtable[0].shape);
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
	/* TODO
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
		o.killme = true;
	}
	else
	{
		o.data.ms.frame+= (1<<16);
		if ((o.data.ms.frame>>16) >= o.data.ms.shape->size()) o.data.ms.frame = 0;
	}
}

void KillLogic(MapObject& o, GameLogic* logic)
{
	o.killme = true;
}

void Shoot(MapObject& o, GameLogic* logic, int32_t colltype, int32_t collwith, int32_t hitpoints, int32_t damage, int32_t speed, std::vector<Shape>* shape)
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
	newobject.y = o.y + o.data.ms.firey;
	newobject.z = o.z;
	newobject.data.ms.logic = FireLogic;
	newobject.data.ms.render = 1;
	newobject.data.ms.hit = NullLogic;
	newobject.data.ms.die = KillLogic;
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

	
	logic->AddObject(newobject);
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
	Shoot(o, logic, 2, 0, 1, 3, 16, logic->wtable[3].shape);
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
	o.y = y;
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
		Shoot(o, logic, 4, 0, 2, 3, 20, logic->wtable[1].shape);
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
	int16_t rndval = GloomMaths::RndW();

	if (rndval < 0)
	{
		// what is going on here
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

