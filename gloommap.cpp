#include "gloommap.h"
#include "gloommaths.h"
#include "monsterlogic.h"
#include "soundhandler.h"

static uint16_t Get16(const uint8_t* p)
{
	return (static_cast<uint16_t>(p[0])) << 8 | static_cast<uint16_t>(p[1]);
}

static uint32_t Get32(const uint8_t* p)
{
	return (static_cast<uint16_t>(p[0])) << 24 | (static_cast<uint16_t>(p[1]) << 16) | (static_cast<uint16_t>(p[2])) << 8 | (static_cast<uint16_t>(p[3]) << 0);
}

void Event::Load(const uint8_t* data, uint32_t evnum, std::vector<Object>& objects, std::vector<Door>& doors, std::vector<TextureChange>& tchanges, 
				 std::vector<RotPoly>& rotpolys, std::vector<Teleport>& teles)
{
	uint16_t op;

	do
	{
		op = Get16(data); data += 2;

		switch (op)
		{
			case ET_ADDMONSTER:
				Object o;
				o.t = Get16(data); data += 2;
				o.x = Get16(data); data += 2;
				o.y = Get16(data); data += 2;
				o.z = Get16(data); data += 2;
				o.rot = (uint8_t)Get16(data); data += 2;
				// whats all this? Does the editor use a different coordinate system to the game itself?
				//o.rot = (192 - o.rot + 128) & 255;

				o.ev = evnum;

				if (o.t != 3) // dunno. unused "weapon" type?
				{
					objects.push_back(o);
				}
				break;
			case ET_OPENDOOR:
				Door door;
				door.zone = Get16(data); data+=2;
				door.eventnum = evnum;
				doors.push_back(door);
				break;

			case ET_TELEPORT:
				Teleport tele;

				tele.x = Get16(data); data += 2;
				tele.y = Get16(data); data += 2;
				tele.z = Get16(data); data += 2;
				tele.rot = Get16(data); data += 2;
				tele.ev = evnum;
				teles.push_back(tele);
				break;
				
			case ET_LOADOBJECTS:
				// ignore - we'll load everything
				int16_t temp;
				do
				{
					temp = Get16(data); data += 2;
				} while (temp >= 0);
				break;

			case ET_CHANGETEXTURE:
				TextureChange tc;
				tc.zone = Get16(data); data += 2;
				tc.newtexture = Get16(data); data += 2;
				tc.ev = evnum;
				tchanges.push_back(tc);
				break;

			case ET_ROTATEPOLY:
				RotPoly r;
				r.polynum = Get16(data); data += 2;
				r.count = Get16(data); data += 2;
				r.speed = Get16(data); data += 2;
				r.flags = Get16(data); data += 2;
				r.ev = evnum;
				rotpolys.push_back(r);
				break;

		}
	} while (op != 0);

}


void Zone::Load(const uint8_t* data)
{
	ztype = Get16(data); data += 2;
	x1 = Get16(data); data += 2;
	z1 = Get16(data); data += 2;
	x2 = Get16(data); data += 2;
	z2 = Get16(data); data += 2;
	a = Get16(data); data += 2;
	b = Get16(data); data += 2;
	na = Get16(data); data += 2;
	nb = Get16(data); data += 2;
	ln = Get16(data); data += 2;

	for (auto i = 0; i < 8; i++)
	{
		t[i] = data[i];
	}

	data += 8;

	sc = Get16(data); data += 2;
	ev = Get16(data); data += 2;

	open = 0;
}

void Zone::DumpDebug(FILE* fFile)
{
	const int32_t offset = 32000;

	if (ztype == ZoneType::ZT_WALL)
	{
		fprintf(fFile, "<line x1 = \"%i\" y1 = \"%i\" x2 = \"%i\" y2 = \"%i\" stroke = \"blue\" stroke-width = \"4\" />\n", x1, offset - z1, x2, offset - z2);
	}
	else
	{
		auto sx = x1;
		auto w = x2 - x1;
		auto sy = offset - z1;
		auto h = (offset - z2) - (offset - z1);

		if (w < 0)
		{
			w = -w;
			sx = x2;
		}
		if (h < 0)
		{
			h = -h;
			sy = offset - z2;
		}

		w++;
		h++;

		fprintf(fFile, "<rect x = \"%i\" y = \"%i\" width = \"%i\" height = \"%i\" fill = \"lime\" stroke-width = \"4\" stroke = \"pink\" />\n", sx, sy, w, h);
	}
}

void Texture::Load(const char* name)
{
	std::string fname = "txts/";

	fname += name;

	CrmFile file;
	file.Load(fname.c_str());

	// palette offset
	uint32_t pof = Get32(file.data);

	columns.resize((pof - 4) / 65);

	for (size_t c = 0; c < columns.size(); c++)
	{
		columns[c].flag = file.data[4 + c * 65];
		std::copy(file.data + 4 + c * 65 + 1, file.data + 4 + c * 65 + 65, columns[c].data);
	}

	//now do the palette. 4 bits (at least on original gloom?)
	// 0 appears to not be stored, as is the mask entry 
	palette[0][0] = 0;
	palette[0][1] = 0;
	palette[0][2] = 0;

	auto entries = Get16(file.data + pof);

	for (auto p = 0; p < entries; p++)
	{
		auto colval = Get16(file.data + pof + 2 + p * 2);

		palette[p + 1][0] = (colval >> 8) & 0xF;
		palette[p + 1][1] = (colval >> 4) & 0xF;
		palette[p + 1][2] = (colval >> 0) & 0xF;

		// may as well hoof this up to 8bpc here
		palette[p + 1][0] |= palette[p + 1][0] << 4;
		palette[p + 1][1] |= palette[p + 1][1] << 4;
		palette[p + 1][2] |= palette[p + 1][2] << 4;
	}
}

void Texture::DumpDebug(const char* name)
{
	//dump a ppm file
	FILE* file = fopen(name, "wb");

	fprintf(file, "P6\n64 %lu\n255\n", columns.size());

	for (size_t c = 0; c < columns.size(); c++)
	{
		for (auto x = 0; x < 64; x++)
		{
			auto entry = columns[c].data[x];
			fputc(palette[entry][0], file);
			fputc(palette[entry][1], file);
			fputc(palette[entry][2], file);
		}
	}

	fclose(file);
}

bool GloomMap::Load(const char* name, ObjectGraphics* nobj)
{
	doors.clear();
	objects.clear();
	mapobjects.clear();
	objects.clear();
	anims.clear();
	tchanges.clear();
	activedoors.clear();
	rotpolys.clear();
	activerotpolys.clear();
	teles.clear();
	activeblood.clear();

	objectlogic = nobj;

	GloomMaths::SeedRnd(0xa3f7);

	for (auto zonetype = 0; zonetype < 2; zonetype++)
	{
		for (auto z = 0; z < 32; z++)
		{
			for (auto x = 0; x < 32; x++)
			{
				collisionpolys[zonetype][x][z].clear();
			}
		}
	}

	if (!rawdata.Load(name))
	{
		return false;
	}

	uint32_t eventpointers[numevents];

	uint32_t gridoff;
	uint32_t polyoff;
	uint32_t polypnt;
	uint32_t animpnt;
	uint32_t txtnames;

	gridoff = Get32(rawdata.data + 0);
	polyoff = Get32(rawdata.data + 4);
	polypnt = Get32(rawdata.data + 8);
	animpnt = Get32(rawdata.data + 12);
	txtnames= Get32(rawdata.data +16);

	for (auto e = 0; e < numevents; e++)
	{
		eventpointers[e] = Get32(rawdata.data + 20 + e * 4);
	}

	for (auto zonetype = 0; zonetype < 2; zonetype++)
	{
		for (auto z = 0; z < 32; z++)
		{
			for (auto x = 0; x < 32; x++)
			{
				gridnums[zonetype][x][z] = (Get16(rawdata.data + gridoff + (x + z * 32) * 8 + zonetype * 4) + 1) & 0xFFFF;
				polyoffsets[zonetype][x][z] = Get16(rawdata.data + gridoff + (x + z * 32) * 8 + zonetype * 4 + 2);

				collisionpolys[zonetype][x][z].resize(gridnums[zonetype][x][z]);

				for (uint32_t poly = 0; poly < gridnums[zonetype][x][z]; poly++)
				{
					collisionpolys[zonetype][x][z][poly] = Get16(rawdata.data + polypnt + polyoffsets[zonetype][x][z] * 2 + poly * 2);
				}
			}
		}
	}

#if 0
	for (auto z = 0; z < 32; z++)
	{
		for (auto x = 0; x < 32; x++)
		{
			printf("%c", gridnums[0][x][31-z] ? ('0' + gridnums[0][x][31-z]) : ' ');
		}
		printf("\n");
	}
#endif

	zones.resize((polypnt - polyoff) / Zone::ZoneSize);

	auto p = polyoff;
	for (auto &z : zones)
	{
		z.Load(rawdata.data + p);
		p += Zone::ZoneSize;
	}

	for (auto s = 0; s < 8; s++)
	{
		texturenames[s] = "";
	}

	auto s = 0;
	auto spos = rawdata.data + txtnames;

	while (s < 8)
	{
		uint8_t c = *spos;

		if (c == 0)
		{
			s++;
		}
		else
		{
			texturenames[s] += c;
		}

		spos++;
	}

	for (auto t = 0; t < 8; t++)
	{
		if (texturenames[t].length())
		{
			textures[t].Load(texturenames[t].c_str());
		}
	}

	for (auto e = 0; e < numevents; e++)
	{
		// this starts at 1. 
		events[e].Load(rawdata.data + eventpointers[e], e + 1, objects, doors, tchanges, rotpolys, teles);
	}

	//set up the object sideband

	for (auto &o : objects)
	{
		o.frame = objectlogic->objectlogic[o.t].frame;
		o.framespeed = objectlogic->objectlogic[o.t].framespeed;
		o.render = objectlogic->objectlogic[o.t].render;
		o.movspeed = objectlogic->objectlogic[o.t].movspeed;

		o.shape = objectlogic->objectlogic[o.t].shape;
		o.chunk = objectlogic->objectlogic[o.t].spark;
		o.firey = objectlogic->objectlogic[o.t].firey;

		o.base = objectlogic->objectlogic[o.t].base;
		o.range = objectlogic->objectlogic[o.t].range;

		o.firecnt = objectlogic->objectlogic[o.t].firecnt;
		o.firerate = objectlogic->objectlogic[o.t].firerate;

		o.colltype = objectlogic->objectlogic[o.t].colltype;
		o.collwith = objectlogic->objectlogic[o.t].collwith;
		o.rad = objectlogic->maxwidths[o.t];
		o.damage = objectlogic->objectlogic[o.t].damage;
		o.hitpoints = objectlogic->objectlogic[o.t].hitpoints;
		o.weapon = objectlogic->objectlogic[o.t].weapon;
		o.hurtpause = objectlogic->objectlogic[o.t].hurtpause;
		o.blood = objectlogic->objectlogic[o.t].blood;
		o.punchrate = objectlogic->objectlogic[o.t].punchrate;

		o.scale = objectlogic->objectlogic[o.t].scale;
	}

	// load wall anims

	if (animpnt)
	{
		uint32_t a = 0;

		while (1)
		{
			Anim thisanim;

			thisanim.frames = Get16(rawdata.data + animpnt + a * 8);

			if (thisanim.frames)
			{
				thisanim.first = Get16(rawdata.data + animpnt + a * 8 + 2);
				thisanim.delay = Get16(rawdata.data + animpnt + a * 8 + 4);
				thisanim.current = Get16(rawdata.data + animpnt + a * 8 + 6);

				anims.push_back(thisanim);
				a++;
			}
			else
			{
				break;
			}
		}
	}

	//I'm assuming the start event can't have a teleport...
	bool dummy;
	Teleport dummyt;
	ExecuteEvent(1, dummy, dummyt);

	// set up the texture pointers

	int texturestotal = 0;

	for (int t = 0; t < 8; t++)
	{
		// G3 (and others?) occasionally have short textures
		//texturestotal += textures[t].columns.size() / 64;
		if (textures[t].columns.size()) texturestotal += 20;
	}

	for (auto i = 0; i < 160; i++)
	{
		if (i<texturestotal)
		{
			if ((64 * (i % 20)) < (int)textures[i / 20].columns.size())
			{
				texturepointers[i] = &(textures[i / 20].columns[64 * (i % 20)]);
				texturepointersorig[i] = &(textures[i / 20].columns[64 * (i % 20)]);
			}
		}
	}

	//DumpDebug();

	return true;
}

void GloomMap::SetFlat(char f)
{
	hasflat = true;

	std::string name = "txts/floor";

	name += f + '0';

	floor.Load(name.c_str());

	name = "txts/roof";

	name += f + '0';

	ceil.Load(name.c_str());
	return;
}

void Flat::Load(const char* name)
{
	CrmFile file;

	file.Load(name);

	for (int x = 0; x < 128; x++)
	{
		for (int y = 0; y < 128; y++)
		{
			// makeflat goes y first?
			data[x][y] = file.data[y + x * 128];
		}
	}

	// makeflat also indicates it dumps out the depth, but this isn't present in the files I've seen

	uint32_t palpos = 0;
	for (uint32_t p = 128 * 128; p < file.size; p += 2)
	{
		palette[palpos][0] = file.data[p + 0] & 0xF;
		palette[palpos][1] = file.data[p + 1] >> 4;
		palette[palpos][2] = file.data[p + 1] & 0xF;

		palette[palpos][0] |= palette[palpos][0] << 4;
		palette[palpos][1] |= palette[palpos][1] << 4;
		palette[palpos][2] |= palette[palpos][2] << 4;

		palpos++;
	}

	return;
}

void Flat::DumpDebug(const char* name)
{
	//dump a ppm file
	FILE* file = fopen(name, "wb");

	fprintf(file, "P6\n128 128\n255\n");

	for (auto y = 0; y < 128; y++)
	{
		for (auto x = 0; x < 128; x++)
		{
			auto entry = data[x][y];
			fputc(palette[entry][0], file);
			fputc(palette[entry][1], file);
			fputc(palette[entry][2], file);
		}
	}

	fclose(file);
}

void GloomMap::DumpDebug()
{
	FILE* fFile = fopen("debug.svg", "w");

	fprintf(fFile, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
	fprintf(fFile, "<svg width=\"32000\" height=\"32000\" xmlns=\"http://www.w3.org/2000/svg\">\n");
	for (auto &z : zones)
	{
		z.DumpDebug(fFile);
	}
	fprintf(fFile, "</svg>");
	fclose(fFile);

	for (auto t = 0; t < 8; t++)
	{
		if (texturenames[t].length())
		{
			std::string name = texturenames[t];
			name += ".ppm";
			textures[t].DumpDebug(name.c_str());
		}
	}

	if (hasflat)
	{
		floor.DumpDebug("floor.ppm");
		ceil.DumpDebug("roof.ppm");
	}
}

void GloomMap::ExecuteEvent(uint32_t e, bool& gotele, Teleport& teleout)
{
	// add objects?
	
	// DEMOS events seem off by one? 
	// e++;

	gotele = false;

	//printf("EVENT: %i\n", e);

	for (auto o : objects)
	{
		if (o.ev == e)
		{
			MapObject mo(o);

			CalcVecs(mo);

			// ordering seems needed for collison?
			if (mo.t == ObjectGraphics::OLT_PLAYER1)
			{
				mapobjects.push_front(mo);
			}
			else if (mo.t != ObjectGraphics::OLT_PLAYER2)
			{
				// DO NOTHING FOR P2: Invisible P2 object causes enemies to be hurt!
				mapobjects.push_back(mo);
			}
		}
	}

	// doors.
	bool diddoor = false;

	for (auto d : doors)
	{
		if (d.eventnum == e)
		{
			//zones[d.zone].x1 = zones[d.zone].x2 = -1;
			//zones[d.zone].z1 = zones[d.zone].z2 = -1;
			ActiveDoor ad;

			/*
			NOTE MOVE LONGS: This should copy Z's as well?

			move.l	a1, do_poly(a0)
			move.l	zo_lx(a1), do_lx(a0)
			move.l	zo_rx(a1), do_rx(a0)
			clr	do_frac(a0)
			move	#$100, do_fracadd(a0)
			*/
			ad.do_poly = d.zone;
			ad.do_lx = zones[d.zone].x1;
			ad.do_rx = zones[d.zone].x2;
			ad.do_lz = zones[d.zone].z1;
			ad.do_rz = zones[d.zone].z2;
			ad.do_frac = 0;
			ad.do_fracadd = 0x100;

			diddoor = true;
			activedoors.push_back(ad);
		}
	}

	// texture changes

	for (auto t : tchanges)
	{
		if (t.ev == e)
		{
			zones[t.zone].t[0] = t.newtexture;
		}
	}

	// rot/morph polys

	for (auto r : rotpolys)
	{
		if (r.ev == e)
		{
			ActiveRotPoly ar;

			/*
			exec_rotpolys	;
			;could also be morphpolys depending on bit 0 of flags!
				;
				addlast	rotpolys
				bne.s	.ok
				addq	#8,a6
				bra	exec_loop
				;
			.ok	st	doorsfxflag
				movem	(a6)+,d0-d3	;polynum,count,speed,flags
				;
				clr	rp_rot(a0)
				move	d1,rp_num(a0)
				move	d2,rp_speed(a0)
				move	d3,rp_flags(a0)
				move	d1,d5
				subq	#1,d5
				move.l	map_poly(pc),a2	;polygons!
				lsl	#5,d0
				add	d0,a2
				move.l	a2,rp_first(a0)
				;
				btst	#0,d3
				beq	.rot
			*/

			ar.rot = 0;
			ar.num = r.count;
			ar.speed = r.speed;
			ar.flags = r.flags;
			ar.first = r.polynum;

			diddoor = true;

			if (r.flags & 1)
			{
				/*
				;
				;OK, prepare for morph
				;
				lsl	#5,d1
				lea	0(a2,d1),a3
				lea	rp_vx(a0),a1
				;
			.loop	movem	zo_lx(a3),d0-d1
				movem	zo_lx(a2),d2-d3
				sub	d2,d0
				sub	d3,d1
				movem	d0-d3,(a1)
				addq	#8,a1
				;
				lea	32(a2),a2
				lea	32(a3),a3
				dbf	d5,.loop
				;
				bra	exec_loop
				;
				*/

				// MORPH
				for (int16_t p = 0; p < r.count; p++)
				{
					ar.vx[p] = zones[ar.first + p + ar.num].x1 - zones[ar.first + p].x1;
					ar.vz[p] = zones[ar.first + p + ar.num].z1 - zones[ar.first + p].z1;
					ar.ox[p] = zones[ar.first + p].x1;
					ar.oz[p] = zones[ar.first + p].z1;
				}
			}
			else
			{

				/*
				.rot	;First, calc centre X,Z into d6,d7
				;
				moveq	#0,d6
				moveq	#0,d7
				move.l	a2,a1
				;
				.loop0	movem	zo_lx(a1),d0/d2
				add.l	d0,d6
				add.l	d2,d7
				lea	32(a1),a1
				dbf	d5,.loop0
				;
				divu	d1,d6
				divu	d1,d7
				movem	d6-d7,rp_cx(a0)
				;
				lea	rp_lx(a0),a1
				subq	#1,d1
				;
				.loop2	movem	zo_lx(a2),d0/d2
				sub	d6,d0
				move	d0,(a1)+
				sub	d7,d2
				move	d2,(a1)+
				move.l	zo_na(a2),(a1)+
				;
				lea	32(a2),a2
				dbf	d1,.loop2
				*/
				int32_t cx, cz;
				cx = 0; cz = 0;
				for (int16_t p = 0; p < r.count; p++)
				{
					cx += zones[ar.first + p].x1;
					cz += zones[ar.first + p].z1;
				}
				cx /= r.count;
				cz /= r.count;

				ar.cx = cx;
				ar.cz = cz;

				for (int16_t p = 0; p < r.count; p++)
				{
					ar.lx[p] = zones[ar.first + p].x1 - cx;
					ar.lz[p] = zones[ar.first + p].z1 - cz;
					ar.na[p] = zones[ar.first + p].na;
					ar.nb[p] = zones[ar.first + p].nb;
				}
			}

			activerotpolys.push_back(ar);
		}
	}

	if (diddoor) SoundHandler::Play(SoundHandler::SOUND_DOOR);

	//teleports

	for (auto t : teles)
	{
		if (t.ev == e)
		{
			SoundHandler::Play(SoundHandler::SOUND_TELEPORT);
			gotele = true;
			teleout = t;
		}
	}
}

MapObject::MapObject(Object m)
{
	isstrip = false;
	killme = false;

	data.ms.pixsizeadd = 0;
	data.ms.pixsize = 0;

	x.SetInt(m.x);
	y.SetInt(m.y);
	z.SetInt(m.z);
	t = m.t;

	data.ms.rotspeed = 0;

	data.ms.frame = m.frame;
	data.ms.framespeed = m.framespeed;

	data.ms.render = m.render;
	data.ms.rotquick.SetInt(m.rot);
	data.ms.movspeed = m.movspeed;
	data.ms.shape = m.shape;
	data.ms.chunks = m.chunk;
	data.ms.firey = m.firey;

	data.ms.delay = 0;
	data.ms.range = m.range;
	data.ms.base = m.base;
	data.ms.firecnt = m.firecnt;
	data.ms.firerate = m.firerate;

	data.ms.reload = (t == ObjectGraphics::OLT_PLAYER1)? 5 : 0;
	data.ms.reloadcnt = 0;

	data.ms.bounce = 0;
	data.ms.bouncecnt = 0;
	data.ms.mega = 0;

	data.ms.messtimer = 0;
	data.ms.mess = 0;

	data.ms.colltype = m.colltype;
	data.ms.collwith = m.collwith;
	data.ms.washit = 0;
	data.ms.rad = m.rad;
	data.ms.radsq = data.ms.rad * data.ms.rad;
	data.ms.damage = m.damage;
	data.ms.hitpoints = m.hitpoints;
	data.ms.weapon = m.weapon;
	data.ms.hurtwait = 0;
	data.ms.hurtpause = m.hurtpause;
	data.ms.blood = m.blood;
	data.ms.punchrate = m.punchrate;
	data.ms.scale = m.scale;
	data.ms.fired = 0;
	data.ms.invisible = 0;
	data.ms.thermo = 0;

	switch (t)
	{
		case ObjectGraphics::OLT_PLAYER1:
			data.ms.logic = NullLogic;
			data.ms.hit = NullLogicComp;
			data.ms.die = PlayerDie;
			data.ms.eyey = -110;
			break;
		case ObjectGraphics::OLT_PLAYER2:
			data.ms.logic = NullLogic;
			data.ms.hit = NullLogicComp;
			data.ms.die = NullLogicComp;
			data.ms.eyey = -110;
			break;
		case ObjectGraphics::OLT_MARINE:
			data.ms.logic = MonsterLogic;
			data.ms.hit = HurtNGrunt;
			data.ms.die = BlowObject;
			break;
		case ObjectGraphics::OLT_BALDY:
			data.ms.logic = BaldyLogic;
			data.ms.hit = HurtNGrunt;
			data.ms.die = BlowObject;
			break;
		case ObjectGraphics::OLT_PHANTOM:
			data.ms.logic = PhantomLogic;
			data.ms.hit = HurtNGrunt;
			data.ms.die = BlowObject;
			break;
		case ObjectGraphics::OLT_DEMON:
			data.ms.logic = DemonLogic;
			data.ms.hit = HurtNGrunt;
			data.ms.die = BlowObject;
			break;
		case ObjectGraphics::OLT_TERRA:
			data.ms.logic = TerraLogic;
			data.ms.hit = HurtTerra;
			data.ms.die = BlowTerra;
			break;
		case ObjectGraphics::OLT_GHOUL:
			data.ms.logic = GhoulLogic;
			data.ms.hit = NullLogicComp;
			data.ms.die = BlowObjectNoChunks;
			break;
		case ObjectGraphics::OLT_LIZARD:
			data.ms.logic = LizardLogic;
			data.ms.hit = LizHurt;
			data.ms.die = BlowObject;
			break;
		case ObjectGraphics::OLT_TROLL:
			data.ms.logic = TrollLogic;
			data.ms.hit = TrollHurt;
			data.ms.die = BlowObject;
			break;
		case ObjectGraphics::OLT_DEATHHEAD:
			data.ms.logic = DeathLogic;
			data.ms.hit = HurtDeath;
			data.ms.die = BlowDeath;
			break;
		case ObjectGraphics::OLT_DRAGON:
			data.ms.logic = DragonLogic;
			data.ms.hit = NullLogicComp;
			data.ms.die = BlowDragon;
			break;
		case ObjectGraphics::OLT_WEAPON1:
		case ObjectGraphics::OLT_WEAPON2:
		case ObjectGraphics::OLT_WEAPON3:
		case ObjectGraphics::OLT_WEAPON4:
		case ObjectGraphics::OLT_WEAPON5:
			data.ms.logic = WeaponLogic;
			data.ms.hit = WeaponGot;
			data.ms.die = WeaponGot;
			break;
		case ObjectGraphics::OLT_HEALTH:
			data.ms.logic = NullLogic;
			data.ms.hit = HealthGot;
			data.ms.die = HealthGot;
			break;
		case ObjectGraphics::OLT_INVISI:
			data.ms.logic = NullLogic;
			data.ms.hit = InvisGot;
			data.ms.die = InvisGot;
			break;
		case ObjectGraphics::OLT_THERMO:
			data.ms.logic = NullLogic;
			data.ms.hit = ThermoGot;
			data.ms.die = ThermoGot;
			break;
		case ObjectGraphics::OLT_BOUNCY:
			data.ms.logic = BouncyLogic;
			data.ms.hit = BouncyGot;
			data.ms.die = BouncyGot;
			break;
		default:
			data.ms.logic = NullLogic;
			data.ms.hit = NullLogicComp;
			data.ms.die = KillLogicComp;
	}

	// avoid zero as I need to flag nothing
	identifier = counter+1;
	counter++;
}

MapObject::MapObject()
{
	isstrip = false;
	killme = false;

	data.ms.pixsizeadd = 0;
	data.ms.pixsize = 0;
	data.ms.delay = 0;
	data.ms.reload = 0;
	data.ms.reloadcnt = 0;


	data.ms.colltype = 0;
	data.ms.collwith = 0;
	data.ms.washit = 0;
	data.ms.damage = 0;
	data.ms.hitpoints = 0;
	data.ms.weapon = 0;
	data.ms.hurtwait = 0;
	data.ms.hurtpause = 0;
	data.ms.punchrate = 0;
	data.ms.scale = 0x200;
	data.ms.rotspeed = 0;
	data.ms.mega = 0;
	data.ms.bounce = 0;
	data.ms.bouncecnt = 0;

	data.ms.messtimer = 0;
	data.ms.mess = 0;

	data.ms.fired = 0;
	data.ms.invisible = 0;
	data.ms.thermo = 0;

	// avoid zero as I need to flag nothing
	identifier = counter + 1;
	counter++;
}

uint64_t MapObject::counter;