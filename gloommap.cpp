#include "gloommap.h"

static uint16_t Get16(const uint8_t* p)
{
	return (static_cast<uint16_t>(p[0])) << 8 | static_cast<uint16_t>(p[1]);
}

static uint32_t Get32(const uint8_t* p)
{
	return (static_cast<uint16_t>(p[0])) << 24 | (static_cast<uint16_t>(p[1]) << 16) | (static_cast<uint16_t>(p[2])) << 8 | (static_cast<uint16_t>(p[3]) << 0);
}

void Event::Load(const uint8_t* data, uint32_t evnum, std::vector<Object>& objects, std::vector<Door>& doors, std::vector<TextureChange>& tchanges)
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
				o.rot = (192 - o.rot + 128) & 255;

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
				//TODO
				data += 8;
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
				//TODO
				data += 8;
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

	fprintf(file, "P6\n64 %i\n255\n", columns.size());

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

	objectlogic = nobj;

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
		events[e].Load(rawdata.data + eventpointers[e], e + 1, objects, doors, tchanges);
	}

	//set up the object sideband

	for (auto &o : objects)
	{
		o.frame = objectlogic->objectlogic[o.t].frame;
		o.framespeed = objectlogic->objectlogic[o.t].framespeed;
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

	ExecuteEvent(1);

	// set up the texture pointers

	int texturestotal = 0;

	for (int t = 0; t < 8; t++)
	{
		texturestotal += textures[t].columns.size() / 64;
	}

	for (auto i = 0; i < 160; i++)
	{
		if (i<texturestotal)
		{
			texturepointers[i] = &(textures[i / 20].columns[64 * (i % 20)]);
			texturepointersorig[i] = &(textures[i / 20].columns[64 * (i % 20)]);
		}
	}

	return true;
}

void GloomMap::SetFlat(char f)
{
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

	floor.DumpDebug("floor.ppm");
	ceil.DumpDebug("roof.ppm");
}

void GloomMap::ExecuteEvent(uint32_t e)
{
	// add objects?

	for (auto o : objects)
	{
		if (o.ev == e)
		{
			MapObject mo(o);

			mapobjects.push_back(mo);
		}
	}

	// doors. TODO: Add animation
	for (auto d : doors)
	{
		if (d.eventnum == e)
		{
			zones[d.zone].x1 = zones[d.zone].x2 = -1;
			zones[d.zone].z1 = zones[d.zone].z2 = -1;
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
}