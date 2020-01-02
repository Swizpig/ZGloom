#include "gamelogic.h"
#include "renderer.h"
#include "monsterlogic.h"

void GameLogic::Init(GloomMap* gmapin, Camera* cam)
{
	gmap = gmapin;

	camdir = 1;
	std::fill(animframe, animframe + 160, 0);

	for (auto o : gmap->GetMapObjects())
	{
		if (o.t == 0)// player
		{
			cam->x = o.x;
			cam->y = 120; //TODO, and rotation
			cam->z = o.z;
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

	inc.SetVal(0xd0000);

	Quick camrots[4], camrotstrafe[4];

	GloomMaths::GetCamRot(cam->rot, camrots);
	GloomMaths::GetCamRot((cam->rot)+64&0xFF, camrotstrafe);
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);

	Quick newx = cam->x;
	Quick newz = cam->z;

	if (keystate[SDL_SCANCODE_UP])
	{
		// U 
		newx = cam->x + camrots[1] * inc;
		newz = cam->z + camrots[0] * inc;
		cam->y += camdir;
	}
	if (keystate[SDL_SCANCODE_DOWN])
	{
		// D
		newx = cam->x - camrots[1] * inc;
		newz = cam->z - camrots[0] * inc;
		cam->y += camdir;
	}
	if (keystate[SDL_SCANCODE_LEFT])
	{
		//L
		//TODO: Rotation acceleration
		if (keystate[SDL_SCANCODE_LSHIFT])
		{
			//strafe
			newx = newx + camrotstrafe[1] * inc;
			newz = newz + camrotstrafe[0] * inc;
		}
		else
		{
			cam->rot += 2;
		}
	}
	if (keystate[SDL_SCANCODE_RIGHT])
	{
		//R
		if (keystate[SDL_SCANCODE_LSHIFT])
		{
			//strafe
			newx = newx - camrotstrafe[1] * inc;
			newz = newz - camrotstrafe[0] * inc;
		}
		else
		{
			cam->rot -= 2;
		}
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
	}

	// event check

	if (Collision(true, cam->x.GetInt(), cam->z.GetInt(), 32, overshoot, closestzone))
	{
		if (gmap->GetZones()[closestzone].ev>1)
		{
			if (gmap->GetZones()[closestzone].ev == 24)
			{
				done = true;
			}
			gmap->ExecuteEvent(gmap->GetZones()[closestzone].ev);

			// these are one-shot
			if (gmap->GetZones()[closestzone].ev<19)
			{
				gmap->GetZones()[closestzone].ev = -1;
			}
		}
	}

	// move any doors
	DoDoor();

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

	if (cam->y > 136) camdir = -1;
	if (cam->y < 120) camdir = 1;

	for (auto &o : gmap->GetMapObjects())
	{
		o.logic(o, this);
	}

	return done;
}