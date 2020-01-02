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

	Quick camrots[4];

	o.movspeed += 8;

	GloomMaths::GetCamRot(o.movspeed&127, camrots);

	o.y = (camrots[2].GetVal() >> 9) & 0xFF;
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

	uint32_t ang = o.rot & 255;

	Quick camrots[4], t;
	GloomMaths::GetCamRot(ang, camrots);
	t.SetVal(o.movspeed);
	// these seem inverted compared to above?
	o.xvec = (t * camrots[1]).GetVal();
	o.zvec = -(t * camrots[3]).GetVal();
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

	t.SetVal(o.xvec);
	newx = newx + t;

	t.SetVal(o.zvec);
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

void MonsterLogic(MapObject& o, GameLogic* logic)
{
	/*
	monsterlogic;
		move	ob_rot(a5), ob_oldrot(a5)
		; monster cruising around minding his own business...
		;
	*/

	o.oldrot = o.rot;
	/* TODO
		subq	#1, ob_delay(a5)
		ble	fire1
		;
	*/

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
		o.rot += (GloomMaths::RndW() > 0) ? 64 : -64;
		CalcVecs(o);
		if (CheckVecs(o, logic)) goto good;

		o.rot += 128;
		CalcVecs(o);
		if (CheckVecs(o, logic)) goto good;

		o.rot = o.oldrot + 128;
		CalcVecs(o);
		CheckVecs(o, logic);
	}

good:
	o.frame += o.framespeed;
	o.frame &= 0x3FFFF;
}