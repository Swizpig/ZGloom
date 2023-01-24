#include "gamelogic.h"
#include "renderer.h"
#include "monsterlogic.h"
#include "soundhandler.h"
#include "hud.h"
#include "config.h"

void GameLogic::Init(ObjectGraphics* ograph)
{
	// note weird order of SFX.
	wtable[0].hitpoint = 1;
	wtable[0].damage = 1;
	wtable[0].speed = 32;
	wtable[0].sound = SoundHandler::SOUND_SHOOT3;

	wtable[1].hitpoint = 5;
	wtable[1].damage = 2;
	wtable[1].speed = 36;
	wtable[1].sound = SoundHandler::SOUND_SHOOT5;

	wtable[2].hitpoint = 10;
	wtable[2].damage = 2;
	wtable[2].speed = 40;
	wtable[2].sound = SoundHandler::SOUND_SHOOT;

	wtable[3].hitpoint = 15;
	wtable[3].damage = 3;
	wtable[3].speed = 40;
	wtable[3].sound = SoundHandler::SOUND_SHOOT4;

	wtable[4].hitpoint = 20;
	wtable[4].damage = 5;
	wtable[4].speed = 24;
	wtable[4].sound = SoundHandler::SOUND_SHOOT5;

	for (auto i = 0; i < 5; i++)
	{
		wtable[i].shape = &(ograph->BulletShapes[i]);
	}

	for (auto i = 0; i < 5; i++)
	{
		wtable[i].spark = &(ograph->SparkShapes[i]);
	}

	p1lives = 3;
	p1health = 25;
	p1weapon = 0;
	p1reload = 5;

	playerhit = false;
}

void GameLogic::ResetPlayer(MapObject& o)
{
	// reset the player on death
	o.data.ms.hitpoints = 25;
	o.data.ms.eyey = -110;
	o.data.ms.logic = NullLogic;
	//o.data.ms.colltype = 8;
	//o.data.ms.collwith = 4;
	// 3 seconds invulnrability. Curiously, code comment state 2?
	o.data.ms.delay = 75;
	o.x = origx;
	o.z = origz;
	o.data.ms.rotquick.SetInt(origrot);
	o.data.ms.invisible = 0;
	o.data.ms.thermo = 0;
	o.data.ms.bouncecnt = 0;
}

void GameLogic::InitLevel(GloomMap* gmapin, Camera* cam, ObjectGraphics* ograph)
{
	gmap = gmapin;
	objectgraphics = ograph;
	levelfinished = false;
	levelfinishednow = false;
	sucking = 0;
	sucker = 0;
	std::fill(animframe, animframe + 160, 0);

	for (auto e = 0; e < 25; e++)
	{
		eventhit[e] = false;
	}

	for (auto &o : gmap->GetMapObjects())
	{
		if (o.t == 0)// player
		{
			cam->x = o.x;
			cam->y = 120; //TODO, and rotation
			cam->z = o.z;
			cam->rotquick = o.data.ms.rotquick;

			origx = o.x;
			origz = o.z;
			origrot = o.data.ms.rotquick.GetInt();

			o.data.ms.hitpoints = p1health;
			//o.data.ms.lives = p1lives; TODO
			o.data.ms.weapon = p1weapon;
			o.data.ms.reload = p1reload;
		}
	}
}

bool GameLogic::AdjustPos(int32_t& overshoot, Quick& x, Quick& z, int32_t r, int32_t& closestzone)
{
	/*
	adjustposq;
	neg	d0
	move	d0, d1
		;
	muls	zo_a(a4), d0
	add.l	d0, d0
		;
	muls	zo_b(a4), d1
	add.l	d1, d1
		;
	sub.l	d0, d6
	sub.l	d1, d7
		;
	rts
	*/

	overshoot = -overshoot;

	int32_t xo, zo;

	xo = overshoot * gmap->GetZones()[closestzone].a;
	xo += xo;

	zo = overshoot * gmap->GetZones()[closestzone].b;
	zo += zo;

	x.SetVal(x.GetVal() - xo);
	z.SetVal(z.GetVal() - zo);

	return Collision(false, x.GetInt(), z.GetInt(), r, overshoot, closestzone);
}

void GameLogic::MoveBlood()
{
	for (auto &b : gmap->GetBlood())
	{
		//TODO: sucking, screen splatter

		if (b.y.GetInt() < 0)
		{
			// normal blood
			b.x = b.x + b.xvec;
			b.y = b.y + b.yvec;
			b.z = b.z + b.zvec;

			Quick temp;
			temp.SetVal(0x8000);
			b.yvec = b.yvec + temp;

			if (b.y.GetInt() >= 0)
			{
				b.killme = true;
			}
		}
		else
		{
			// deaths head suck!
			/*
			move.l	bl_xvec(a5),d0
			add.l	d0,bl_x(a5)
			move.l	bl_zvec(a5),d1
			add.l	d1,bl_z(a5)
			;
			move.l	bl_dest(a5),a0
			move	bl_x(a5),d0
			sub	ob_x(a0),d0
			muls	d0,d0
			move	bl_z(a5),d1
			sub	ob_z(a0),d1
			muls	d1,d1
			add.l	d1,d0
			cmp.l	#64*64,d0
			bcc.s	.loop
			bra.s	.kill
			*/

			if (!sucking) 
			{
				b.killme = true;
			}
			else
			{
				b.x = b.x + b.xvec;
				b.z = b.z + b.zvec;

				MapObject o = GetNamedObj(b.dest);

				int32_t dist = (o.x.GetInt() - b.x.GetInt()) *  (o.x.GetInt() - b.x.GetInt());

				dist += (o.z.GetInt() - b.z.GetInt()) *  (o.z.GetInt() - b.z.GetInt());

				if (dist < 64 * 64)
				{
					b.killme = true;
				}
			}
		}
	}

	// kill pass
	auto b = gmap->GetBlood().begin();

	while (b != gmap->GetBlood().end())
	{
		if (b->killme)
		{
			b = gmap->GetBlood().erase(b);
		}
		else
		{
			++b;
		}
	}
}

void GameLogic::DoDoor()
{
	for (auto &d : gmap->GetActiveDoors())
	{

		/*
		dodoors
		lea	doors(pc), a5
		;
		.loop
		move.l(a5), a5
		tst.l(a5)
		beq.done
		;
		move.l	do_poly(a5), a0
		move	do_fracadd(a5), d0
		add		d0, do_frac(a5)
		move	do_frac(a5), d0
		move	d0, d1
		add		d1, d1
		move	d1, zo_open(a0); copy frac
		;
		*/
		Zone& zone = gmap->GetZones()[d.do_poly];

		d.do_frac += d.do_fracadd;
		zone.open = d.do_frac * 2;

		/*
			move	do_rx(a5), d1
			sub		do_lx(a5), d1; width
			move	d1, d2
			muls	d0, d2
			lsl.l	#2, d2
			swap	d2
			move	do_lx(a5), d3
			sub		d2, d3
			move	d3, zo_lx(a0)
			add		d1, d3
			move	d3, zo_rx(a0)
			;

		*/

		int32_t width = d.do_rx - d.do_lx;
		int32_t origwidth = width;
		width *= d.do_frac;
		width <<= 2;
		width >>= 16;
		zone.x1 = d.do_lx - width;
		zone.x2 = zone.x1 + origwidth;

		/*
			move	do_rz(a5), d1
			sub		do_lz(a5), d1
			move	d1, d2
			muls	d0, d2
			lsl.l	#2, d2
			swap	d2
			move	do_lz(a5), d3
			sub		d2, d3
			move	d3, zo_lz(a0)
			add		d1, d3
			move	d3, zo_rz(a0)
		*/

		width = d.do_rz - d.do_lz;
		origwidth = width;
		width *= d.do_frac;
		width <<= 2;
		width >>= 16;
		zone.z1 = d.do_lz - width;
		zone.z2 = zone.z1 + origwidth;

		/*
			;
			tst		d0
			beq.s	.kill
			cmp		#$4000, d0
			bne.s	.loop
			;
			.kill
			move.l	a5, a0
			killitem	doors
			move.l	a0, a5
			bra.loop
			;
			.done
			rts
			*/
	}

	//kill pass

	auto i = gmap->GetActiveDoors().begin();

	while (i != gmap->GetActiveDoors().end())
	{
		if (i->do_frac == 0x4000)
		{
			gmap->GetZones()[i->do_poly].x1 = -1;
			gmap->GetZones()[i->do_poly].x2 = -1;
			gmap->GetZones()[i->do_poly].z1 = -1;
			gmap->GetZones()[i->do_poly].z2 = -1;

			i = gmap->GetActiveDoors().erase(i);
		}
		else
		{
			++ i;
		}
	}
}

MapObject GameLogic::GetPlayerObj()
{
	for (auto o : gmap->GetMapObjects())
	{
		if (o.t == ObjectGraphics::OLT_PLAYER1)
		{
			return o;
			break;
		}
	}

	// warning squash

	return gmap->GetMapObjects().front();
}

MapObject GameLogic::GetNamedObj(uint64_t id)
{
	for (auto o : gmap->GetMapObjects())
	{
		if (o.identifier == id)
		{
			return o;
			break;
		}
	}

	// warning squash

	return gmap->GetMapObjects().front();
}

uint8_t GameLogic::PickCalc(MapObject& o)
{
	/*
	pickcalc; pick a player and calculate angle to player!
		;
		bsr	pickplayer
		bsr	calcangle
		tst	ob_invisible(a0)
		beq.s.rts
		move	d0, -(a7)
		bsr	rndw
		and	#63, d0
		sub	#32, d0
		add(a7) + , d0
		and	#255, d0
		.rts	rts
	*/
	MapObject player = GetPlayerObj();
	uint8_t ang = GloomMaths::CalcAngle(player.x.GetInt(), player.z.GetInt(), o.x.GetInt(), o.z.GetInt());

	if (player.data.ms.invisible)
	{
		ang = ang + (GloomMaths::RndW() & 63) - 32;
	}
	return ang;
}

void  GameLogic::DoRot()
{
	std::vector<ActiveRotPoly>&rotpolys = gmap->GetActiveRotPolys();
	std::vector<Zone>&zones = gmap->GetZones();

	for (auto& r: rotpolys)
	{
		if (r.speed)
		{
			r.rot += r.speed;

			if (r.flags & 1)
			{
				// Morph
				if (r.rot < 0)
				{
					if (r.flags & 3)
					{
						r.speed = -r.speed;
					}
					else
					{
						r.speed = 0;
					}
				}
				else if (r.rot > 0x4000)
				{
					if (r.flags & 2)
					{
						r.speed = -r.speed;
					}
					else
					{
						r.speed = 0;
					}
				}

				for (int vertex = 0; vertex < r.num; vertex++)
				{
					auto thiszone = r.first + vertex;
					auto prevzone = thiszone - 1;

					if (vertex == 0) prevzone = r.first + r.num - 1;

					/*
						.loop	movem(a3) + , d0 - d3, (vx, vz, ox, oz, d4 is rot)
						muls	d4, d0
						lsl.l	#2, d0
						swap	d0
						add	d2, d0
						muls	d4, d1
						lsl.l	#2, d1
						swap	d1
						add	d3, d1
						movem	d0 - d1, zo_lx(a2)
						movem	d0 - d1, zo_rx(a1)
						move.l	a2, a1
					*/
					int32_t vx = r.vx[vertex];
					vx *= r.rot;
					vx <<= 2;
					vx >>= 16;
					vx += r.ox[vertex];

					int32_t vz= r.vz[vertex];
					vz *= r.rot;
					vz <<= 2;
					vz >>= 16;
					vz += r.oz[vertex];

					zones[thiszone].x1 = vx;
					zones[thiszone].z1 = vz;

					zones[prevzone].x2 = vx;
					zones[prevzone].z2 = vz;
				}

				// norm recalc
				for (int vertex = 0; vertex < r.num; vertex++)
				{
					auto thiszone = r.first + vertex;
					int16_t rx, rz;

					GloomMaths::CalcNormVec(zones[thiszone].x2 - zones[thiszone].x1, zones[thiszone].z2 - zones[thiszone].z1, rx, rz);

					zones[thiszone].na = rx;
					zones[thiszone].nb = rz;

					zones[thiszone].a = -rz;
					zones[thiszone].b = rx;
				}

			}
			else
			{
				//rot
				r.rot += r.speed;

				for (int vertex = 0; vertex < r.num; vertex++)
				{
					auto thiszone = r.first + vertex;
					auto prevzone = thiszone - 1;

					if (vertex == 0) prevzone = r.first + r.num - 1;

					int16_t rotmatrix[4];
					GloomMaths::GetCamRot2Raw(r.rot & 1023, rotmatrix);

					int16_t nx, nz;

					Rotter(r.lx[vertex], r.lz[vertex], nx, nz, rotmatrix);

					zones[thiszone].x1 = nx + r.cx;
					zones[thiszone].z1 = nz + r.cz;

					zones[prevzone].x2 = nx + r.cx;
					zones[prevzone].z2 = nz + r.cz;

					Rotter(r.na[vertex], r.nb[vertex], nx, nz, rotmatrix);

					zones[thiszone].na = nx;
					zones[thiszone].nb = nz;

					zones[thiszone].a = -nz;
					zones[thiszone].b = nx;
				}
			}
		}
	}
}

void GameLogic::Rotter(int16_t x, int16_t z, int16_t&nx, int16_t& nz, int16_t camrots[4])
{
	/*
		move	d0, d2
		move	d1, d3
		;
		muls(a4), d0
		muls	2(a4), d3
		add.l	d3, d0
		add.l	d0, d0
		swap	d0; new x!
		;
		muls	4(a4), d2
		muls	6(a4), d1
		add.l	d2, d1
		add.l	d1, d1
		swap	d1
	*/
	int32_t newx = (int32_t)camrots[0] * (int32_t)x + (int32_t)camrots[1] * (int32_t)z;
	int32_t newz = (int32_t)camrots[2] * (int32_t)x + (int32_t)camrots[3] * (int32_t)z;

	// compensate for 1.15 fracs
	newx += newx;
	newz += newz;

	nx = (newx >> 16);
	nz = (newz >> 16);
}

bool GameLogic::Collision(bool event, int32_t x, int32_t z, int32_t r, int32_t& overshoot, int32_t& closestzone)
{
	// do 3x3 square

	int32_t closest = 0x3fff;
	bool good = true;

	for (int32_t dx = -1; dx < 2; dx++)
	{
		for (int32_t dz = -1; dz < 2; dz++)
		{
			int32_t gx = x / 256 + dx;
			int32_t gz = z / 256 + dz;

			if ((gx >= 0) && (gx < 32) && (gz >= 0) && (gz < 32))
			{
				std::vector<uint32_t> collzones = gmap->GetCollisions(event?1:0, gx, gz);

				for (size_t checkzone = 0; checkzone < collzones.size(); checkzone++)
				{
					int32_t dist = FindSegDist(x, z, gmap->GetZones()[collzones[checkzone]]);

					if (dist < r)
					{
						good = false;

						if (dist < closest)
						{
							closest = dist;
							closestzone = collzones[checkzone];
						}
					}
				}
			}
		}
	}

	// explicit check of rotpolys as they may have gone out of their collision grid spot

	std::vector<ActiveRotPoly>& rotpolys = gmap->GetActiveRotPolys();

	for (auto &thisrot : rotpolys)
	{
		for (int16_t i = 0; i < thisrot.num; i++)
		{
			int32_t dist = FindSegDist(x, z, gmap->GetZones()[thisrot.first + i]);

			if (dist < r)
			{
				good = false;

				if (dist < closest)
				{
					closest = dist;
					closestzone = thisrot.first + i;
				}
			}
		}
	}

	overshoot = closest - r;

	return !good;
}

int32_t GameLogic::FindSegDist(int32_t x, int32_t z, Zone& zone)
{
	// tranlation of source. Some kind of cross product to determine if exceeds line length, then similar perp check
	/*
	findsegdist; find distance from d6, d7 to zone in a4...
		;
	; find end dist
		move	zo_rx(a4), d0
		sub	d6, d0
		muls	zo_na(a4), d0
		move	zo_rz(a4), d1
		sub	d7, d1
		muls	zo_nb(a4), d1
		add.l	d1, d0
		add.l	d0, d0
		swap	d0; distance from end
		;
	cmp	zo_ln(a4), d0
		bcs.s.perp; use perpendicular distance!
		;
	move	#$3fff, d0
		rts
		;
	.perp; find perpendicular dist.
		;
	move	zo_rx(a4), d0
		sub	d6, d0
		muls	zo_a(a4), d0
		move	zo_rz(a4), d1
		sub	d7, d1
		muls	zo_b(a4), d1
		add.l	d1, d0
		add.l	d0, d0
		bpl.s.pl
		neg.l	d0
		.pl	swap	d0; perpendicular dist.w
		rts

	*/

	int32_t tx, tz;

	tx = zone.x2 - x;
	tx *= zone.na;
	tz = zone.z2 - z;
	tz *= zone.nb;

	tx += tz;
	tx *= 2;

	if (((tx >> 16) < zone.ln) && (tx>=0))
	{
		tx = zone.x2 - x;
		tx *= zone.a;
		tz = zone.z2 - z;
		tz *= zone.b;

		tx += tz;
		tx *= 2;

		if (tx < 0) tx = -tx;

		return tx >> 16;
	}
	
	return 0x3FFF;
}

bool GameLogic::Update(Camera* cam)
{
	Quick inc;
	bool done = false;
	bool moved = false;

	inc.SetVal(0xd0000);
	newobjects.clear();

	Quick camrots[4], camrotstrafe[4];

	GloomMaths::GetCamRot(cam->rotquick.GetInt()&0xFF, camrots);
	GloomMaths::GetCamRot(((cam->rotquick.GetInt())+64)&0xFF, camrotstrafe);
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);

	Quick newx = cam->x;
	Quick newz = cam->z;

	MapObject playerobj = GetPlayerObj();
	int16_t initialhealth = playerobj.data.ms.hitpoints;
	bool squished = false;

	if (playerobj.data.ms.logic == NullLogic)
	{
		playerobj.x = cam->x;
		playerobj.y.SetInt(0);
		playerobj.z = cam->z;
		playerobj.data.ms.rotquick = cam->rotquick;

		inc.SetVal(playerobj.data.ms.movspeed);

		//wire these up to controller as well at some point

		bool controlfire  = keystate[Config::GetKey(Config::KEY_SHOOT)] != 0;
		bool controlup = keystate[Config::GetKey(Config::KEY_UP)] != 0;
		bool controldown = keystate[Config::GetKey(Config::KEY_DOWN)] != 0;
		bool controlleft = keystate[Config::GetKey(Config::KEY_LEFT)] != 0;
		bool controlright = keystate[Config::GetKey(Config::KEY_RIGHT)] != 0;
		bool controlstrafeleft = keystate[Config::GetKey(Config::KEY_SLEFT)] != 0;
		bool controlstraferight = keystate[Config::GetKey(Config::KEY_SRIGHT)] != 0;
		bool controlstrafemod = keystate[Config::GetKey(Config::KEY_STRAFEMOD)] != 0;

		if (Config::HaveController())
		{
			Sint32 contX = Config::GetControllerX();
			Sint32 contY = Config::GetControllerY();
			
			if (contX < -8000) controlstrafeleft = true;
			if (contX >  8000) controlstraferight = true;
			if (contY < -8000) controlup = true;
			if (contY >  8000) controldown = true;

			if (Config::GetControllerFire()) controlfire = true;
		}

		if (SDL_GetMouseState(NULL, NULL))
		{
			controlfire = true;
		}

		if (controlup)
		{
			// U 
			newx = cam->x - camrots[1] * inc;
			newz = cam->z + camrots[0] * inc;
			moved = true;
		}
		if (controldown)
		{
			// D
			newx = cam->x + camrots[1] * inc;
			newz = cam->z - camrots[0] * inc;
			moved = true;
		}
		if (controlleft || controlstrafeleft)
		{
			//L
			//TODO: Rotation acceleration
			if (controlstrafemod || controlstrafeleft)
			{
				//strafe
				newx = newx + camrotstrafe[1] * inc;
				newz = newz - camrotstrafe[0] * inc;
				moved = true;
			}
			else
			{
				cam->rotquick.SetInt(cam->rotquick.GetInt() - 4);
			}
		}
		if (controlright || controlstraferight)
		{
			//R
			if (controlstrafemod || controlstraferight)
			{
				//strafe
				newx = newx - camrotstrafe[1] * inc;
				newz = newz + camrotstrafe[0] * inc;
				moved = true;
			}
			else
			{
				cam->rotquick.SetInt(cam->rotquick.GetInt() + 4);
			}
		}

		if (!moved)
		{
			//unbounce
			if (playerobj.data.ms.bounce)
			{
				playerobj.data.ms.bounce += 30;

				if ((playerobj.data.ms.bounce & 127) < 30)
				{
					playerobj.data.ms.bounce = 0;
					SoundHandler::Play(SoundHandler::SOUND_FOOTSTEP);
				}
			}
		}
		else
		{
			// bounce!
			/*
			.bounce	move	ob_bounce(a5),d2
			add	#20,ob_bounce(a5)
			move	ob_bounce(a5),d1
			and	#255,d2
			cmp	#64,d2
			bcc.s	.fskip
			and	#255,d1
			cmp	#64,d1
			bcs.s	.fskip
			;
			bsr	footstep
			*/

			int16_t d2 = playerobj.data.ms.bounce;
			playerobj.data.ms.bounce += 20;
			int16_t d1 = playerobj.data.ms.bounce;

			d2 &= 255;
			d1 &= 255;

			if ((d1 >= 64) && (d2 < 64))
			{
				SoundHandler::Play(SoundHandler::SOUND_FOOTSTEP);
			}
		}

		if (controlfire)
		{
			//Shoot!
			if ((playerobj.data.ms.reloadcnt == 0) && (!firedown))
			{
				auto wep = playerobj.data.ms.weapon;

				if (playerobj.data.ms.mega)
				{
					if (playerobj.data.ms.mega >= (750 + 125))
					{
						// ULTRA MEGA OVERKILL
						playerobj.data.ms.rotquick.SetInt(playerobj.data.ms.rotquick.GetInt() + 8);
						Shoot(playerobj, this, (playerobj.data.ms.collwith & 3) ^ 3, 0, wtable[wep].hitpoint, wtable[wep].damage, wtable[wep].speed, wtable[wep].shape, wtable[wep].spark);
						playerobj.data.ms.rotquick.SetInt(playerobj.data.ms.rotquick.GetInt() - 16);
						Shoot(playerobj, this, (playerobj.data.ms.collwith & 3) ^ 3, 0, wtable[wep].hitpoint, wtable[wep].damage, wtable[wep].speed, wtable[wep].shape, wtable[wep].spark);
						playerobj.data.ms.rotquick.SetInt(playerobj.data.ms.rotquick.GetInt() + 8);
						Shoot(playerobj, this, (playerobj.data.ms.collwith & 3) ^ 3, 0, wtable[wep].hitpoint, wtable[wep].damage, wtable[wep].speed, wtable[wep].shape, wtable[wep].spark);
					}
					else
					{
						playerobj.data.ms.rotquick.SetInt(playerobj.data.ms.rotquick.GetInt() + 4);
						Shoot(playerobj, this, (playerobj.data.ms.collwith & 3) ^ 3, 0, wtable[wep].hitpoint, wtable[wep].damage, wtable[wep].speed, wtable[wep].shape, wtable[wep].spark);
						playerobj.data.ms.rotquick.SetInt(playerobj.data.ms.rotquick.GetInt() - 8);
						Shoot(playerobj, this, (playerobj.data.ms.collwith & 3) ^ 3, 0, wtable[wep].hitpoint, wtable[wep].damage, wtable[wep].speed, wtable[wep].shape, wtable[wep].spark);
						playerobj.data.ms.rotquick.SetInt(playerobj.data.ms.rotquick.GetInt() + 4);
					}
				}
				else
				{
					Shoot(playerobj, this, (playerobj.data.ms.collwith & 3) ^ 3, 0, wtable[wep].hitpoint, wtable[wep].damage, wtable[wep].speed, wtable[wep].shape, wtable[wep].spark);
				}
				SoundHandler::Play(wtable[wep].sound);
				playerobj.data.ms.reloadcnt = playerobj.data.ms.reload;
				if (!Config::GetAutoFire()) firedown = true;
				playerobj.data.ms.fired = 10;
			}
		}
		else
		{
			firedown = false;
		}

		if (playerobj.data.ms.reloadcnt > 0) playerobj.data.ms.reloadcnt--;

		if (keystate[SDL_SCANCODE_F1])
		{
			// cheat for debug
			done = true;
		}


		int32_t overshoot, closestzone;

		if (!Collision(false, newx.GetInt(), newz.GetInt(), 32, overshoot, closestzone))
		{
			cam->x = newx;
			cam->z = newz;
		}
		else
		{
			// well, it's what the original seems to do...
			if (!AdjustPos(overshoot, newx, newz, 32, closestzone))
			{
				cam->x = newx;
				cam->z = newz;
			}
			else if (!AdjustPos(overshoot, newx, newz, 32, closestzone))
			{
				cam->x = newx;
				cam->z = newz;
			}
			else
			{
				if (Collision(false, cam->x.GetInt(), cam->z.GetInt(), 32, overshoot, closestzone))
				{
					squished = true;
				}
			}
		}

		// mouse control
		{
			int mx, my;
			SDL_GetRelativeMouseState(&mx, &my);

			cam->rotquick.SetVal(cam->rotquick.GetVal() + mx*Config::GetMouseSens() * 800);
		}

		//gamepad control
		if (Config::HaveController())
		{
			Sint32 rot = Config::GetControllerRot();

			if (abs(rot) < 8000) rot = 0;

			cam->rotquick.SetVal(cam->rotquick.GetVal() + rot * 10);
		}
		

		CheckSuck(cam);

		cam->y = -(playerobj.y.GetInt() + playerobj.data.ms.eyey);
		// add bounce
		int16_t camrotsraw[4];
		GloomMaths::GetCamRotRaw(playerobj.data.ms.bounce & 255, camrotsraw);
		cam->y -= (camrotsraw[1] * 20) >> 16;

		// event check
		Teleport tele;
		bool gotele = false;

		// prevent multiple sound playing!
		if (!playerobj.data.ms.pixsizeadd)
		{
			if (Collision(true, cam->x.GetInt(), cam->z.GetInt(), 32, overshoot, closestzone))
			{
				if (gmap->GetZones()[closestzone].ev > 1)
				{
					if (gmap->GetZones()[closestzone].ev == 24)
					{
						levelfinished = true;
						SoundHandler::Play(SoundHandler::SOUND_TELEPORT);
						playerobj.data.ms.pixsizeadd = 1;
					}

					if (!eventhit[gmap->GetZones()[closestzone].ev])
					{
						gmap->ExecuteEvent(gmap->GetZones()[closestzone].ev, gotele, tele);
					}

					// these are one-shot
					if (gmap->GetZones()[closestzone].ev < 19)
					{
						eventhit[gmap->GetZones()[closestzone].ev] = true;
					}
				}
			}
		}

		if (gotele)
		{
			// teleport animation
			activetele = tele;
			playerobj.data.ms.pixsizeadd = 2;
		}
	}
	else
	{
		// we're dead, jim
		//playerobj.data.ms.logic(playerobj, this);
	}

	// actually do the tele animation

	playerobj.data.ms.pixsize += playerobj.data.ms.pixsizeadd;

	if (playerobj.data.ms.pixsize >= 24)
	{
		cam->x.SetInt(activetele.x);
		cam->z.SetInt(activetele.z);
		cam->rotquick.SetInt((uint8_t)activetele.rot);
		playerobj.data.ms.pixsizeadd = -playerobj.data.ms.pixsizeadd;
		if (levelfinished) done = true;
	}

	if (levelfinishednow) done = true;

	if (playerobj.data.ms.pixsize == 0) playerobj.data.ms.pixsizeadd = 0;

	MoveBlood();

	// move any doors
	DoDoor();

	// update rots/morphs

	DoRot();

	//update the anims

	Column** tp = gmap->GetTexPointers();
	Column** to = gmap->GetTexPointersOrig();

	for (auto& a : gmap->GetAnims())
	{
		a.current--;

		if (a.current < 0)
		{
			a.current = a.delay;

			int curframe = animframe[a.first];

			curframe++;
			if (curframe >= a.frames)
			{
				curframe = 0;
			}

			animframe[a.first] = curframe;

			tp[a.first] = to[a.first + curframe];
		}
	}

	for (auto &o : gmap->GetMapObjects())
	{
		o.data.ms.logic(o, this);
	}

	ObjectCollision();
	
	//made a bit of a horlicks of this. 
	//I'm not confident about passing pointers to list members around, is that safe? I moved the kill pass to the end, so it should be OK, but erred on the side of safely
	auto playerobjupdated = GetPlayerObj();

	playerobj.data.ms.hitpoints = playerobjupdated.data.ms.hitpoints;
	playerobj.data.ms.weapon = playerobjupdated.data.ms.weapon;
	playerobj.data.ms.reload = playerobjupdated.data.ms.reload;
	playerobj.data.ms.eyey = playerobjupdated.data.ms.eyey;
	playerobj.data.ms.delay = playerobjupdated.data.ms.delay;
	playerobj.data.ms.colltype = playerobjupdated.data.ms.colltype;
	playerobj.data.ms.collwith = playerobjupdated.data.ms.collwith;
	playerobj.data.ms.mega = playerobjupdated.data.ms.mega;
	playerobj.data.ms.mess = playerobjupdated.data.ms.mess;
	playerobj.data.ms.messtimer = playerobjupdated.data.ms.messtimer;
	playerobj.data.ms.invisible = playerobjupdated.data.ms.invisible;
	playerobj.data.ms.thermo = playerobjupdated.data.ms.thermo;
	playerobj.data.ms.bouncecnt = playerobjupdated.data.ms.bouncecnt;

	if (squished)
	{
		playerobj.data.ms.hitpoints--;
	}

	if (playerobj.data.ms.mega)
	{
		playerobj.data.ms.mega--;

		if (playerobj.data.ms.mega == 0)
		{
			playerobj.data.ms.mess = Hud::MESSAGES_MEGA_WEAPON_OUT;
			playerobj.data.ms.messtimer = -127;
		}
	}
	if (playerobj.data.ms.invisible)
	{
		playerobj.data.ms.invisible--;
		if (playerobj.data.ms.invisible==0)
		{
			playerobj.data.ms.mess = Hud::MESSAGES_INVISIBILITY_OUT;
			playerobj.data.ms.messtimer = -127;
		}
	}
	if (playerobj.data.ms.thermo)
	{
		playerobj.data.ms.thermo--;
		if (playerobj.data.ms.thermo == 0)
		{
			playerobj.data.ms.mess = Hud::MESSAGES_THERMO_OUT;
			playerobj.data.ms.messtimer = -127;
		}
	}
	if (playerobj.data.ms.messtimer < 0)
	{
		playerobj.data.ms.messtimer++;
	}
	if (playerobj.data.ms.fired>0)
	{
		playerobj.data.ms.fired--;
	}

	playerhit = playerobj.data.ms.hitpoints < initialhealth;

	if (playerobj.data.ms.logic != NullLogic)
	{
		// we're still dead, jim
		cam->x = playerobj.x = playerobjupdated.x;
		playerobj.y = playerobjupdated.y;
		cam->y = -(playerobj.y.GetInt() + playerobj.data.ms.eyey);
		cam->z = playerobj.z = playerobjupdated.z;

		playerobj.data.ms.rotquick = playerobjupdated.data.ms.rotquick;
		cam->rotquick = playerobj.data.ms.rotquick;
		playerhit = true;
	}

	//do this after the above otherwise I don't pick up the player reset on death
	playerobj.data.ms.logic = playerobjupdated.data.ms.logic;

	if (playerobj.data.ms.logic == NullLogic)
	{
		//invuln timer after death
		if (playerobj.data.ms.delay)
		{
			playerobj.data.ms.delay--;

			if (playerobj.data.ms.delay == 0)
			{
				//reset collision data so can be hit again
				playerobj.data.ms.colltype = 8;
				playerobj.data.ms.collwith = 4;
			}
		}
	}

	if (squished &&  (playerobj.data.ms.hitpoints <= 0))
	{
		playerobj.data.ms.hitpoints = 0;
		playerobj.data.ms.logic = PlayerDeath;

		playerobj.data.ms.colltype = 0;
		playerobj.data.ms.collwith = 0;
	}

	//kill pass

	auto i = gmap->GetMapObjects().begin();

	while (i != gmap->GetMapObjects().end())
	{
		if (i->killme)
		{
			i = gmap->GetMapObjects().erase(i);
		}
		else
		{
			++i;
		}
	}

	gmap->GetMapObjects().insert(gmap->GetMapObjects().end(), newobjects.begin(), newobjects.end());

	for (auto& o : gmap->GetMapObjects())
	{
		if (o.t == ObjectGraphics::OLT_PLAYER1)
		{
			o = playerobj;

			if (done)
			{
				//p1lives = 3; TODO
				p1health = o.data.ms.hitpoints;
				p1weapon = o.data.ms.weapon;
				p1reload = o.data.ms.reload;
			}

			break;
		}
	}

	return done;
}

int32_t GameLogic::GetTeleEffect()
{
	for (auto o : gmap->GetMapObjects())
	{
		if (o.t == ObjectGraphics::OLT_PLAYER1)
		{
			return o.data.ms.pixsize;
		}
	}

	return 0;
}

bool GameLogic::GetThermo()
{
	for (auto o : gmap->GetMapObjects())
	{
		if (o.t == ObjectGraphics::OLT_PLAYER1)
		{
			return o.data.ms.thermo != 0;
		}
	}

	return false;
}

void GameLogic::ObjectCollision()
{
	for (auto &o : gmap->GetMapObjects())
	{
		if (o.data.ms.collwith)
		{
			for (auto &o2 : gmap->GetMapObjects())
			{
				// don't compare with self!
				if (o.identifier == o2.identifier)
				{
					o.data.ms.washit = 0;

					// note goes back to the outer loop in the original? But that seems to mess with the asymettric nature of the collision system?
					// UPDATE: this may be why objects get added to both the back and front of the list
					break;
				}

				if ((o.data.ms.collwith & o2.data.ms.colltype) == 0)
				{
					continue;
				}

				int32_t radsum = o2.data.ms.rad + o.data.ms.rad;

				if (abs(o2.x.GetInt() - o.x.GetInt()) > radsum)
				{
					continue;
				}
				if (abs(o2.z.GetInt() - o.z.GetInt()) > radsum)
				{
					continue;
				}

				int32_t xd = abs(o2.x.GetInt() - o.x.GetInt());
				int32_t zd = abs(o2.z.GetInt() - o.z.GetInt());

				if ((xd*xd + zd*zd) < (radsum*radsum))
				{
					//printf("COLLISION %i %i\n", o.t, o2.t);
					if (o.data.ms.washit == o2.identifier)
					{
						// prevents double collision, as this part of the code hits *both* objects. I think?
						break;
					}

					o.data.ms.washit = o2.identifier;

					o.data.ms.hitpoints -= o2.data.ms.damage;
					if (o.data.ms.hitpoints <= 0)
					{
						o.data.ms.die(o, o2, this);
					}
					else
					{
						o.data.ms.hit(o, o2, this);
					}

					o2.data.ms.hitpoints -= o.data.ms.damage;
					if (o2.data.ms.hitpoints <= 0)
					{
						o2.data.ms.die(o2, o, this);
					}
					else
					{
						o2.data.ms.hit(o2, o, this);
					}

					// note break here. 
					break;
				}
			}
		}
	}
}

void GameLogic::CheckSuck(Camera* cam)
{
	/*
	checksuck	cmp.l	sucking(pc),a5
	bne.s	.nosuck
	;
	move.l	suckangle(pc),a0
	moveq	#25,d0
	move	d0,d1
	muls	2(a0),d0
	neg.l	d0
	add.l	d0,d6
	;
	muls	6(a0),d1
	add.l	d1,d7
	bsr	checknewslow
	beq.s	.newok
	bsr	adjustpos
	beq.s	.newok
	bsr	adjustpos
	beq.s	.newok
	;
	move.l	ob_x(a5),d6
	move.l	ob_z(a5),d7
	bra.s	.nosuck
	;
	.newok	move.l	d6,ob_x(a5)
	move.l	d7,ob_z(a5)
	;
	.nosuck	rts

	*/
	Quick xpos, zpos;

	xpos = cam->x;
	zpos = cam->z;

	if (!sucking) return;

	int16_t camrots[4];
	GloomMaths::GetCamRotRaw(suckangle, camrots);

	int32_t xvec = 25 * camrots[1];
	int32_t zvec = 25 * camrots[3];

	xvec = -xvec;

	xpos.SetVal(xpos.GetVal() + xvec);
	zpos.SetVal(zpos.GetVal() + zvec);

	int32_t overshoot, closestzone;

	if (!Collision(false, xpos.GetInt(), zpos.GetInt(), 32, overshoot, closestzone))
	{
		cam->x = xpos;
		cam->z = zpos;
	}
	else
	{
		if (!AdjustPos(overshoot, xpos, zpos, 32, closestzone))
		{
			cam->x = xpos;
			cam->z = zpos;
		}
		else if (!AdjustPos(overshoot, xpos, zpos, 32, closestzone))
		{
			cam->x = xpos;
			cam->z = zpos;
		}
	}

}