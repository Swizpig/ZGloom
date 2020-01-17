#include "renderer.h"
#include <cstdint>
#include "quick.h"
#include "gloommaths.h"
#include "objectgraphics.h"

static void debugVline(int x, int y1, int y2, SDL_Surface* s, uint32_t c)
{
	if ((x < 0) || (x >= s->w)) return;
	if (y2 < y1) std::swap(y1, y2);

	for (int y = y1; y <= y2; y++)
	{
		if ((y >= 0) && (y < s->h))
		{
			((uint32_t*)(s->pixels))[x + s->pitch/4 * y] = c;
		}
	}	
}

static void debugline(int x1, int y1, int x2, int y2, SDL_Surface* s, uint32_t c)
{
	if ((x1 < 0) && (x2 < 0)) return;
	if ((y1 < 0) && (y2 < 0)) return;
	if ((x1 >= 320) && (x2 >= 320)) return;
	if ((y1 >= 240) && (y2 >= 240)) return;

	if (x1 == x2)
	{
		debugVline(x1, y1, y2, s, c);
		return;
	}

	if (x1 > x2)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	// can't be arsed implementing full brenhhnhnnnsnnann for just debug

	float m = (float)(y2 - y1) / (float)(x2 - x1);
	float fy = (float)y1;

	int ly = y1;

	for (auto x = x1; x <= x2; x++)
	{
		int y = (int)fy;

		debugVline(x, y, ly, s, c);

		ly = y;
		fy += m;
	}
}

bool Renderer::PointInFront(int16_t fx, int16_t fz, Wall& z)
{

	if (fz < z.wl_nz) return true;
	if (fz > z.wl_fz) return false;
	// cross product to check if angle pivots towards or away from the origin compared to the wall

	// wall deltas
	int32_t wdx = z.wl_rx;
	int32_t wdz = z.wl_rz;
	wdx -= z.wl_lx;
	wdz -= z.wl_lz;

	// left wall to test point
	int32_t tx = fx;
	int32_t tz = fz;

	tx -= z.wl_lx;
	tz -= z.wl_lz;

	return (wdx*tz - wdz*tx) <= 0;
}

void Renderer::DrawMap()
{
	int16_t scale = 16;
	int16_t sx1, sx2, sz1, sz2;

	int z = 0;
	for (auto w : walls)
	{
		if (gloommap->GetZones()[z].ztype == 1)
		{
			sx1 = 160 + w.wl_lx / scale;
			sz1 = 120 - w.wl_lz / scale;
			sx2 = 160 + w.wl_rx / scale;
			sz2 = 120 - w.wl_rz / scale;

			debugline(sx1, sz1, sx2, sz2, rendersurface, 0xFFFFFF00);
		}

		++z;
	}
}

void Renderer::ClipWalls()
{
	for (size_t i = 0; i < walls.size(); i++)
	{
		for (size_t j = 0; j < walls.size(); j++)
		{
			if (walls[i].valid && walls[j].valid && (i != j))
			{
				// clip left side
				if (Intersect(walls[i].wl_lsx, walls[j].wl_lsx, walls[j].wl_rsx))
				{
					if (!PointInFront(walls[i].wl_lx, walls[i].wl_lz, walls[j]))
					{
						walls[i].wl_lsx = walls[j].wl_rsx;
					}
				}
				// clip right side
				if (Intersect(walls[i].wl_rsx, walls[j].wl_lsx, walls[j].wl_rsx))
				{
					if (!PointInFront(walls[i].wl_rx, walls[i].wl_rz, walls[j]))
					{
						walls[i].wl_rsx = walls[j].wl_lsx;
					}
				}
			}
		}
	}

	for (size_t i = 0; i < walls.size(); i++)
	{
		if (walls[i].valid)
		{
			if (walls[i].wl_rsx <= walls[i].wl_lsx) walls[i].valid = false;
		}
	}
}

bool Renderer::OriginSide(int16_t fx, int16_t fz, int16_t bx, int16_t bz)
{
	// quick cross product to check what side of the origin a line that goes behing the camera falls

	// front to origin
	int32_t ftoox = -fx;
	int32_t ftooz = -fz;

	//front to back
	int32_t ftobx = bx;
	int32_t ftobz = bz;

	ftobx -= fx;
	ftobz -= fz;

	return (ftobx*ftooz - ftobz*ftoox) > 0;
}

void Renderer::Init(SDL_Surface* nrendersurface, GloomMap* ngloommap, ObjectGraphics* nobjectgraphics)
{
	rendersurface = nrendersurface;
	renderwidth = rendersurface->w;
	renderheight = rendersurface->h;
	halfrenderwidth = renderwidth / 2;
	halfrenderheight = renderheight / 2;
	gloommap = ngloommap;
	objectgraphics = nobjectgraphics;

	castgrads.resize(renderwidth);
	zbuff.resize(renderwidth);

	walls.resize(gloommap->GetZones().size());

	for (auto x = 0; x < renderwidth; x++)
	{
		Quick f;
		Quick g;

		f.SetVal(1 << focshift);
		g.SetVal(x - halfrenderwidth);

		castgrads[x] = g / f;
	}


	// darkness tables
	for (auto d = 0; d < 16; d++)
	{
		for (auto c = 0; c < 16; c++)
		{
			darkpalettes[d][c] = c * (16 - d) / 16;
		}
	}
}

void Renderer::DrawFlat(std::vector<int32_t>& ceilend, std::vector<int32_t>& floorstart, Camera* camera)
{
	//TODO
	// skip over invalid runs for performance
	// work out why tz needs a weird +32 to align properly
	if (!gloommap->HasFlat())
	{
		return;
	}

	Flat& ceil = gloommap->GetCeil();
	Flat& floor = gloommap->GetFloor();

	Quick camrots[4];

	int32_t maxend = *std::max_element(ceilend.begin(), ceilend.end());
	int32_t minstart = *std::min_element(floorstart.begin(), floorstart.end());

	if (minstart <= halfrenderheight) minstart = halfrenderheight + 1;
	if (maxend >= halfrenderheight) maxend = halfrenderheight - 1;

	GloomMaths::GetCamRot(camera->rot, camrots);

	for (int32_t y = 0; y < maxend; y++)
	{
		int32_t z = ((256 - camera->y) << focshift) / (halfrenderheight - y);

		Quick qx, dx, qz, temp, dz;
		Quick f;

		f.SetInt(1 << focshift);

		qx.SetInt(z*-halfrenderwidth / (1 << focshift));
		qz.SetInt(z);

		temp = qx;

		qx = qx*camrots[0] + qz*camrots[1];
		qz = temp*camrots[2] + qz*camrots[3];

		dx.SetInt(z);
		dx = dx / f;
		temp = dx;

		//dz is initially zero!

		dx = dx*camrots[0];
		dz = temp*camrots[2];

		qz = qz + camera->z;
		qx = qx + camera->x;

		for (int32_t x = 0; x < renderwidth; x++)
		{
			if (y < ceilend[x])
			{
				auto ix = qx.GetInt() & 0x7F;
				auto iz = (qz.GetInt()+32) & 0x7F;

				uint8_t r = ceil.palette[ceil.data[ix][iz]][0];
				uint8_t g = ceil.palette[ceil.data[ix][iz]][1];
				uint8_t b = ceil.palette[ceil.data[ix][iz]][2];

				// dim it
				uint32_t dimcol;
				ColourModify(r, g, b, dimcol, z);
				((uint32_t*)(rendersurface->pixels))[x + y*renderwidth] = dimcol;
			}

			qx = qx + dx;
			qz = qz + dz;
		}
	}

	// and the same thing again for the floor

	for (int32_t y = minstart; y < renderheight; y++)
	{
		int32_t z = (camera->y << focshift) / (y - halfrenderheight);

		Quick qx, dx, qz, temp, dz;
		Quick f;

		f.SetInt(1 << focshift);

		qx.SetInt(z*-halfrenderwidth / (1 << focshift));
		qz.SetInt(z);

		temp = qx;

		qx = qx*camrots[0] + qz*camrots[1];
		qz = temp*camrots[2] + qz*camrots[3];

		dx.SetInt(z);
		dx = dx / f;
		temp = dx;

		//dz is initially zero!

		dx = dx*camrots[0];
		dz = temp*camrots[2];

		qz = qz + camera->z;
		qx = qx + camera->x;

		for (int32_t x = 0; x < renderwidth; x++)
		{
			if (y >= floorstart[x])
			{
				auto ix = qx.GetInt() & 0x7F;
				auto iz = (qz.GetInt()+32) & 0x7F;

				uint8_t r = floor.palette[floor.data[ix][iz]][0];
				uint8_t g = floor.palette[floor.data[ix][iz]][1];
				uint8_t b = floor.palette[floor.data[ix][iz]][2];

				// dim it
				uint32_t dimcol;
				ColourModify(r, g, b, dimcol, z);

				((uint32_t*)(rendersurface->pixels))[x + y*renderwidth] = dimcol;
			}

			qx = qx + dx;
			qz = qz + dz;
		}
	}
}

void Renderer::DrawColumn(int32_t x, int32_t ystart, int32_t h, Column* texturedata, int32_t z, int32_t palused)
{
	Quick temp;
	Quick tscale;
	Quick tstart;
	int32_t yend = ystart + h;

	if (h == 0) return;
	if (h > 65535) return; // this overflows a quick! Can happen in high res

	uint32_t* surface = (uint32_t*)(rendersurface->pixels);

	if (yend > renderheight) yend = renderheight;

	tscale.SetInt(64);
	temp.SetInt(h);
	tstart.SetInt(0);

	tscale = tscale / temp;

	if (ystart < 0)
	{
		temp.SetInt(-ystart);
		ystart = 0;
		tstart = tscale * temp;
	}

	for (auto y = ystart; y < yend; y++)
	{
		uint16_t row = tstart.GetInt();

		if (row>63) row = 63;

		//if (gloommap->GetTextures()[t / 1280].columns.size())
		{
			uint8_t colour = texturedata->data[row];

			uint8_t r = gloommap->GetTextures()[palused].palette[colour][0];
			uint8_t g = gloommap->GetTextures()[palused].palette[colour][1];
			uint8_t b = gloommap->GetTextures()[palused].palette[colour][2];

			if (texturedata->flag && (colour == 0))
			{
				//translucent
				const uint32_t stripands[] = { 0xff00ffff, 0xffff00ff, 0xffffff00, 0xff0000ff, 0xff00ff00, 0xffff0000, 0xffffffff };

				uint32_t andval = stripands[7 + texturedata->flag];

				surface[x + y*renderwidth] = surface[x + y*renderwidth] & andval;
			}
			else
			{
				// dim it
				uint32_t dimcol;
				ColourModify(r, g, b, dimcol, z);
				surface[x + y*renderwidth] = dimcol;
			}
		}

		tstart = tstart + tscale;
	}
}

bool zcompare(const MapObject& first, const MapObject& second)
{
	return first.rotz > second.rotz;
}

void Renderer::DrawObjects(Camera* camera)
{
	Quick x, z, temp;
	Quick cammatrix[4];
	int32_t ix, iz, iy;

	GloomMaths::GetCamRot(-camera->rot, cammatrix);

	uint32_t* surface = (uint32_t*)(rendersurface->pixels);

	strips.insert(strips.end(), gloommap->GetMapObjects().begin(), gloommap->GetMapObjects().end());

	for (auto &o : strips)
	{
		// don't draw the player!

		if (o.isstrip)
		{
		}
		else
		{
			if ((o.t > 1) && (o.t != 3))
			{
				x = o.x;
				z = o.z;

				x = x - camera->x;
				z = z - camera->z;

				temp = x;
				x = (x * cammatrix[0]) + (z * cammatrix[1]);
				z = (temp * cammatrix[2]) + (z * cammatrix[3]);

				o.rotx = x.GetInt();
				o.rotz = z.GetInt();
			}
		}
	}

	// z sort

	strips.sort(zcompare);

	for (auto o:strips)
	{
		if (o.isstrip)
		{
			int32_t h = (256 << focshift) / o.rotz;
			int32_t ystart = halfrenderheight - ((256 - camera->y) << focshift) / o.rotz;

			if (o.rotz < zbuff[o.rotx]) DrawColumn(o.rotx, ystart, h, o.data.ts.column, o.rotz, o.data.ts.palette);
		}
		else
		{
			// don't draw the player!
			if ((o.t > 1) && (o.t != 3))
			{
				ix = o.rotx;
				iz = o.rotz;
				iy = o.y;
				iy -= camera->y;

				if (iz > 0)
				{
					ix <<= focshift;
					ix /= iz;

					iy <<= focshift;
					iy /= iz;

					if (1)//o.t == ObjectGraphics::OLT_MARINE)
					{
						std::vector<Shape>* s = o.data.ms.shape;

						uint16_t column = 0;

						int frametouse = 0;

						if (o.data.ms.render == 8)
						{
							// rotatable!
							/*
							bsr	calcangle2
							add	#16, d0
							sub	ob_rot(a5), d0
							lsr	#5, d0
							and	#7, d0
							*/
							//uint16_t ang = GloomMaths::CalcAngle(o.x.GetInt(), o.z.GetInt(), camera->x.GetInt(), camera->z.GetInt());

							uint16_t ang = GloomMaths::CalcAngle(camera->x.GetInt(), camera->z.GetInt(), o.x.GetInt(), o.z.GetInt());

							ang += 16;
							ang -= o.data.ms.rot;
							ang >>= 5;
							ang &= 7;
							frametouse = ang | (((o.data.ms.frame >> 16) & 3) << 3);
						}
						else
						{
							frametouse = o.data.ms.frame >> 16;
						}

						auto scale = 2;
						auto shapewidth = (*s)[frametouse].w;
						auto shapeheight = (*s)[frametouse].h;

						int h = ((shapeheight * scale) << focshift) / iz;
						int w = ((shapewidth * scale) << focshift) / iz;

						if ((w > 0) && (h > 0))
						{

							Quick temp;

							Quick dx;
							Quick dy;
							Quick tx, ty;

							tx.SetInt(0);
							ty.SetInt(0);

							dx.SetInt(shapewidth);
							dy.SetInt(shapeheight);

							temp.SetInt(w);
							dx = dx / temp;

							temp.SetInt(h);
							dy = dy / temp;

							int32_t ystart = halfrenderheight - iy - h;

							if ((ix + halfrenderwidth + w / 2) > 0)
							{
								for (int32_t sx = ix + halfrenderwidth - w / 2; sx < (ix + halfrenderwidth + w / 2); sx++)
								{
									if (sx >= renderwidth) break;
									ty.SetInt(0);

									for (int32_t sy = ystart; sy < (ystart + h); sy++)
									{
										if ((sx >= 0) && (iz > zbuff[sx])) break;
										if (sy >= renderheight) break;

										if ((sx >= 0) && (sy >= 0))
										{
											auto col = (*s)[frametouse].data[ty.GetInt() + tx.GetInt()*shapeheight];

											if (col != 1)
											{
												uint32_t dimcol;

												ColourModify(0xFF & (col >> 16), 0xFF & (col >> 8), 0xFF & col, dimcol, o.rotz);
												surface[sx + sy*renderwidth] = dimcol;
											}
										}

										ty = ty + dy;
									}
									tx = tx + dx;
								}
							}
						}
					}
					else
					{
						debugVline(ix + halfrenderwidth, 0, halfrenderheight - iy, rendersurface, 0xFFFFFF);
					}
				}
			}
		}
	}
}

int16_t Renderer::CastColumn(int32_t x, int16_t& zone, Quick& t)
{
	// I'm not sure what Gloom is doing with its wall casting. Something to do with rotating them into the line of the cast?
	// I've rolled my own
	Quick z;

	z.SetInt(30000);
	int16_t hitwall = 0;

	for (auto w: walls)
	{
		if (w.valid)
		{
			if ((x >= w.wl_lsx) && (x <= w.wl_rsx))
			{
				Quick lx, lz, rx, rz, dx, dz, m, thisz;

				lx.SetInt(w.wl_lx);
				lz.SetInt(w.wl_lz);
				rx.SetInt(w.wl_rx);
				rz.SetInt(w.wl_rz);

				dx = rx - lx;
				dz = rz - lz;

				Quick divisor = (dx - castgrads[x] * dz);

				if (divisor.GetVal() != 0)
				{
					m = (castgrads[x] * lz - lx) / (dx - castgrads[x] * dz);
					thisz = lz + m*dz;

					// quick overflow check
					if (thisz.GetInt() < std::min(w.wl_lz, w.wl_rz))
					{
						thisz.SetInt(std::min(w.wl_lz, w.wl_rz));
					}

					if (thisz < z)
					{
						Quick len;

						len.SetInt(w.len);

						if (m.GetVal() < 0) m.SetVal(0);

						// check for transparent column
						int basetexture;
						Column* texcol = GetTexColumn(hitwall, m, basetexture);

						if (texcol && texcol->flag)
						{
							// transparent!
							MapObject o;

							o.isstrip = true;
							o.data.ts.column = texcol;
							o.data.ts.palette = basetexture / 20;
							o.rotx = x;
							o.rotz = thisz.GetInt();
							strips.push_back(o);
						}
						else
						{
							t = m;
							zone = hitwall;
							z = thisz;
						}
					}
				}
			}
		}

		hitwall++;
	}

	return z.GetInt();
}

void Renderer::Render(Camera* camera)
{
	SDL_LockSurface(rendersurface);

	std::fill(zbuff.begin(), zbuff.end(), 30000);
	strips.clear();

	for (size_t z = 0; z < walls.size(); z++)
	{
		Zone zone = gloommap->GetZones()[z];

		if (zone.ztype == Zone::ZT_WALL  && (zone.a | zone.b))
		{
			walls[z].valid = true;

			Quick x1, z1, x2, z2;
			Quick cammatrix[4];

			x1.SetInt(zone.x1);
			z1.SetInt(zone.z1);
			x2.SetInt(zone.x2);
			z2.SetInt(zone.z2);

			x1 = x1 - camera->x;
			z1 = z1 - camera->z;
			x2 = x2 - camera->x;
			z2 = z2 - camera->z;

			GloomMaths::GetCamRot(-camera->rot, cammatrix);

			walls[z].wl_lx = ((x1 * cammatrix[0]) + (z1 * cammatrix[1])).GetInt();
			walls[z].wl_lz = ((x1 * cammatrix[2]) + (z1 * cammatrix[3])).GetInt();
			walls[z].wl_rx = ((x2 * cammatrix[0]) + (z2 * cammatrix[1])).GetInt();
			walls[z].wl_rz = ((x2 * cammatrix[2]) + (z2 * cammatrix[3])).GetInt();
			walls[z].wl_nz = std::min(walls[z].wl_lz, walls[z].wl_rz);
			walls[z].wl_fz = std::max(walls[z].wl_lz, walls[z].wl_rz);

			// a vain attempt to stop z fighting on the doors
			if (zone.open)
			{
				walls[z].wl_rz += 12;
				walls[z].wl_lz += 12;
			}

			walls[z].len = zone.ln;

			// start culling. obvious Z check
			if (walls[z].wl_fz <= 0)
			{
				walls[z].valid = false;
			}
		}
		else
		{
			walls[z].valid = false;
		}
	}
		
	// back face cull

	for (size_t z = 0; z < walls.size(); z++)
	{
		if (walls[z].valid)
		{
			if ((((int32_t)walls[z].wl_lx*(int32_t)walls[z].wl_rz) - ((int32_t)walls[z].wl_rx*(int32_t)walls[z].wl_lz)) >= 0)
			{
				walls[z].valid = false;
			}
		}
	}

	// Z divide
	for (size_t z = 0; z < walls.size(); z++)
	{
		if (walls[z].valid)
		{
			if (walls[z].wl_lz > 0)
			{
				int32_t t = ((int32_t)walls[z].wl_lx << focshift) / walls[z].wl_lz;

				walls[z].wl_lsx = t;

				// I don't know how gloom handles overflows here. There may be something to do with "exshift", I can't figure out what that's for
				
				if ((t > 0) & (walls[z].wl_lsx < 0))
				{
					t = 0x4000;
				}
				if ((t < 0) & (walls[z].wl_lsx > 0))
				{
					t = -0x4000;
				}

				walls[z].wl_lsx = t + halfrenderwidth;
			}
			else
			{
				// uh oh
				walls[z].wl_lsx = OriginSide(walls[z].wl_rx, walls[z].wl_rz, walls[z].wl_lx, walls[z].wl_lz) ? -1 : renderwidth;
			}

			if (walls[z].wl_rz > 0)
			{
				int32_t t = ((int32_t)walls[z].wl_rx << focshift)  / walls[z].wl_rz;

				walls[z].wl_rsx = t;

				// I don't know how gloom handles overflows here. There may be something to do with "exshift", I can't figure out what that's for

				if ((t > 0) & (walls[z].wl_rsx < 0))
				{
					t = 0x4000;
				}
				if ((t < 0) & (walls[z].wl_rsx > 0))
				{
					t = -0x4000;
				}
				walls[z].wl_rsx = t + halfrenderwidth;
			}
			else
			{
				// uh oh
				walls[z].wl_rsx = OriginSide(walls[z].wl_lx, walls[z].wl_lz, walls[z].wl_rx, walls[z].wl_rz) ? -1 : renderwidth;
			}

			if (walls[z].wl_lsx == walls[z].wl_rsx)
			{
				walls[z].valid = false;
			}

			if (walls[z].wl_lsx > walls[z].wl_rsx)
			{
				std::swap(walls[z].wl_lsx, walls[z].wl_rsx);
				std::swap(walls[z].wl_lx, walls[z].wl_rx);
				std::swap(walls[z].wl_lz, walls[z].wl_rz);
			}
		}

		if (walls[z].wl_rsx < 0)
		{
			walls[z].valid = false;
		}
		if (walls[z].wl_lsx >= renderwidth)
		{
			walls[z].valid = false;
		}

		//tidy up
		if (walls[z].wl_lsx < 0) walls[z].wl_lsx = 0;
		if (walls[z].wl_rsx < 0) walls[z].wl_rsx = 0;
		if (walls[z].wl_lsx >= renderwidth) walls[z].wl_lsx = renderwidth - 1;
		if (walls[z].wl_rsx >= renderwidth) walls[z].wl_rsx = renderwidth - 1;
	}

	//ClipWalls();

	std::vector<int32_t> ceilend;
	std::vector<int32_t> floorstart;

	ceilend.resize(renderwidth);
	floorstart.resize(renderwidth);

	for (int32_t x = 0; x < renderwidth; x++)
	{
		int16_t hitzone;
		Quick texpos; 
		int16_t z = CastColumn(x, hitzone, texpos);

		if ((z>0) && (z<30000))
		{
			int32_t h = (256 << focshift) / z;
			int32_t ystart = halfrenderheight - ((256 - camera->y) << focshift) / z;

			ceilend[x] = ystart;
			floorstart[x] = ystart + h;

			int basetexture;
			Column* texcol = GetTexColumn(hitzone, texpos, basetexture);	

			if (texcol)
			{
				DrawColumn(x, ystart, h, texcol, z, basetexture / 20);
			}
			zbuff[x] = z;
			//debugVline(x, 120 - h/2, 120 + h/2, rendersurface, 0xFFFF0000 + 255 - z / 16);
		}
		else
		{
			ceilend[x] = halfrenderheight;
			floorstart[x] = halfrenderheight;
		}
	}

	DrawFlat(ceilend, floorstart, camera);
	DrawObjects(camera);

#if 1
	//DEBUG

	//DrawMap();
#endif

#if 1
	for (size_t z = 0; z < walls.size(); z++)
	{
		if (walls[z].valid)
		{
			//printf("%i: %i %i\n", z, walls[z].wl_lsx, walls[z].wl_rsx);

			//for (int x = walls[z].wl_lsx; x < walls[z].wl_rsx; x++)
			//{
			//	debugVline(x, 0, 25, rendersurface, 0xFFFF0000 + z * 342);
			//}
			//printf("%i: %i %i\n", z, walls[z].wl_lsx, walls[z].wl_rsx);
		}
	}
#endif

	SDL_UnlockSurface(rendersurface);
}

Column* Renderer::GetTexColumn(int hitzone, Quick texpos, int& basetexture)
{
	Quick scale;
	Column* result = nullptr;

	scale.SetInt(gloommap->GetZones()[hitzone].sc / 2);

	// scale is sometimes -ve? What? Possibly reflected texture? I've cobbled this together in a nasty way, don't understand the underlying logic
	if (gloommap->GetZones()[hitzone].sc < 0)
	{
		scale.SetInt(1);
	}

	// not sure how this, well, scales
	if (scale.GetInt() == 0) scale.SetInt(1);

	texpos = texpos*scale;

	auto textouse = texpos.GetInt();

	if (textouse < 0) textouse = 0;
	if (textouse > 7) textouse = 7;

	basetexture = gloommap->GetZones()[hitzone].t[textouse];
	int column = texpos.GetFrac() / (0x10000 / 64);

	// EMPIRICAL F-F-F-F-FUDGE

	if (gloommap->GetZones()[hitzone].sc < 0)
	{
		column /= -gloommap->GetZones()[hitzone].sc * 2;
	}

	Column** tc = gloommap->GetTexPointers();

	if (tc[basetexture])
	{
		result = tc[basetexture] + column;
	}

	return result;
}
