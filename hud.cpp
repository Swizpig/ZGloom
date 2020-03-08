#include "hud.h"
#include "font.h"
#include "config.h"
#include "objectgraphics.h"
#include "gloommaths.h"

// ripped from PPM conversion
static const uint32_t wepraw[5][81*3] =
{ 
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 153, 0, 0, 153, 0, 0,
		153, 0, 0, 153, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		153, 0, 0, 153, 0, 0, 255, 119, 0, 255, 136, 34, 255, 136, 34,
		153, 0, 0, 153, 0, 0, 0, 0, 0, 0, 0, 0, 153, 0, 0,
		255, 136, 34, 238, 238, 102, 238, 238, 102, 238, 238, 102, 255, 136, 34,
		153, 0, 0, 0, 0, 0, 153, 0, 0, 153, 0, 0, 255, 136, 34,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 238, 238, 102, 255, 119, 0,
		153, 0, 0, 153, 0, 0, 255, 136, 34, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 238, 238, 102, 255, 119, 0, 153, 0, 0,
		153, 0, 0, 255, 119, 0, 238, 238, 102, 255, 255, 255, 238, 238, 102,
		255, 255, 255, 255, 136, 34, 153, 0, 0, 153, 0, 0, 153, 0, 0,
		153, 0, 0, 255, 136, 34, 238, 238, 102, 238, 238, 102, 255, 136, 34,
		255, 119, 0, 153, 0, 0, 0, 0, 0, 0, 0, 0, 153, 0, 0,
		153, 0, 0, 153, 0, 0, 255, 136, 34, 153, 0, 0, 153, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 153, 0, 0, 153, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0
	},
	{
		0, 0, 0, 0, 0, 0, 0, 68, 0, 0, 136, 102, 0, 136, 102,
		0, 136, 102, 0, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 102, 68, 136, 204, 136, 238, 238, 102, 238, 238, 102, 238, 238, 102,
		136, 204, 136, 0, 102, 68, 0, 0, 0, 0, 68, 0, 136, 204, 136,
		238, 238, 102, 238, 238, 102, 255, 255, 255, 238, 238, 102, 238, 238, 102,
		136, 204, 136, 0, 68, 0, 0, 102, 68, 136, 204, 136, 238, 238, 102,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 238, 238, 102, 238, 238, 102,
		0, 102, 68, 0, 102, 68, 238, 238, 102, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 238, 238, 102, 0, 102, 68,
		0, 102, 68, 238, 238, 102, 238, 238, 102, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 238, 238, 102, 238, 238, 102, 0, 102, 68, 0, 68, 0,
		136, 204, 136, 238, 238, 102, 238, 238, 102, 255, 255, 255, 238, 238, 102,
		238, 238, 102, 136, 204, 136, 0, 68, 0, 0, 0, 0, 0, 102, 68,
		136, 204, 136, 238, 238, 102, 238, 238, 102, 238, 238, 102, 136, 204, 136,
		0, 102, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 68, 0,
		0, 136, 102, 0, 136, 102, 0, 136, 102, 0, 68, 0, 0, 0, 0,
		0, 0, 0
	},
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 68, 0, 0, 68, 0,
		0, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 136, 102, 136, 204, 136, 221, 204, 255, 136, 204, 136,
		0, 102, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 136, 204, 136,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		0, 102, 68, 0, 0, 0, 0, 68, 0, 221, 204, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 136, 204, 136,
		0, 0, 0, 0, 68, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 136, 204, 136, 0, 68, 0,
		0, 0, 0, 221, 204, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 136, 204, 136, 0, 68, 0, 0, 0, 0,
		0, 136, 102, 238, 238, 102, 238, 238, 102, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 0, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 136, 102, 136, 204, 136, 221, 204, 255, 136, 204, 136, 0, 102, 68,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 68, 0, 0, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0
	},
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 102, 68,
		0, 102, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 136, 119, 238, 136, 204, 136, 136, 119, 238,
		85, 0, 153, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		136, 119, 238, 221, 204, 255, 255, 255, 255, 255, 255, 255, 221, 204, 255,
		85, 0, 153, 0, 0, 0, 0, 0, 0, 85, 51, 204, 221, 204, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 221, 204, 255,
		0, 102, 68, 0, 68, 0, 136, 119, 238, 221, 204, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 221, 204, 255, 85, 0, 153,
		0, 102, 68, 136, 119, 238, 221, 204, 255, 221, 204, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 221, 204, 255, 85, 0, 153, 0, 68, 0,
		136, 204, 136, 238, 238, 102, 255, 255, 255, 255, 255, 255, 221, 204, 255,
		221, 204, 255, 136, 119, 238, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		85, 0, 153, 136, 119, 238, 221, 204, 255, 221, 204, 255, 136, 119, 238,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 68, 0, 0, 102, 68, 0, 102, 68, 0, 0, 0,
		0, 0, 0
	},
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 85, 0, 153,
		85, 0, 153, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 85, 0, 153, 136, 119, 238, 221, 204, 255, 136, 119, 238,
		85, 17, 187, 0, 0, 0, 0, 0, 0, 0, 0, 0, 85, 51, 204,
		221, 204, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 221, 204, 255,
		85, 0, 153, 0, 0, 0, 0, 0, 0, 85, 51, 204, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 136, 119, 238,
		0, 0, 0, 0, 0, 0, 221, 204, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 221, 204, 255, 85, 0, 153,
		0, 0, 0, 136, 119, 238, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 136, 119, 238, 0, 0, 0, 0, 0, 0,
		85, 51, 204, 221, 204, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		221, 204, 255, 85, 0, 153, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		136, 119, 238, 221, 204, 255, 255, 255, 255, 221, 204, 255, 85, 51, 204,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		85, 0, 153, 85, 51, 204, 85, 0, 153, 0, 0, 0, 0, 0, 0,
		0, 0, 0
	}
};

Hud::Hud()
{
	healthbar = SDL_CreateRGBSurface(0, 2 + 25 * 2, 9, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	healthbaron = SDL_CreateRGBSurface(0, 2 + 25 * 2, 9, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	weaponbar = SDL_CreateRGBSurface(0, 2 + 25 * 2, 9, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	for (int x = 0; x < (25 * 2 + 2); x++)
	{
		for (int y = 0; y < 9; y++)
		{
			uint32_t* pix = (uint32_t*)(healthbar->pixels) + x + y*(healthbar->pitch / 4);
			uint32_t* hpix = (uint32_t*)(healthbaron->pixels) + x + y*(healthbar->pitch / 4);
			uint32_t* wpix = (uint32_t*)(weaponbar->pixels) + x + y*(weaponbar->pitch / 4);

			if ((y == 0) || (x == 0))
			{
				*pix = 0xFF008866;
				*hpix = 0xFF008866;
			}
			else if ((y == 8) || (x == (2 * 25 + 1)))
			{
				*pix = 0xFF004400;
				*hpix = 0xFF004400;
			}
			else if ((y == 1) || (y == 7))
			{
				*pix = (x & 1) ? 0xff5511bb : 0xff550099;
				*hpix = (x & 1) ? 0xffbb0000 : 0xff880000;
			}
			else
			{
				*pix = (x & 1) ? 0xff5533CC : 0xff5511bb;
				*hpix = (x & 1) ? 0xffff0000 : 0xffbb0000;
			}

			*wpix = *pix;
		}
	}

	for (int i = 0; i < 5; i++)
	{
		weaponsprites[i] = SDL_CreateRGBSurface(0, 9, 9, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

		for (int y = 0; y < 9; y++)
		{
			for (int x = 0; x < 9; x++)
			{
				uint32_t pix = 0xFF000000;

				pix |= wepraw[i][3 * (x + y * 9) + 0] << 16;
				pix |= wepraw[i][3 * (x + y * 9) + 1] << 8;
				pix |= wepraw[i][3 * (x + y * 9) + 2] << 0;
				if (pix == 0xFF000000) pix = 0;

				*((uint32_t*)(weaponsprites[i]->pixels) + x + y*weaponsprites[i]->pitch / 4) = pix;
			}
		}
	}


	messages.push_back("dummy");
	messages.push_back("health bonus!");
	messages.push_back("weapon boosted to full!");
	messages.push_back("weapon boost!");
	messages.push_back("ultra mega overkill!!!");
	messages.push_back("mega weapon boost!");
	messages.push_back("new weapon!");
	messages.push_back("invisibility!");
	messages.push_back("hyper!");
	messages.push_back("bouncy bullets!");
	messages.push_back("got the thermo glasses!");
	messages.push_back("mega weapon out...'");
	messages.push_back("thermo glasses out...");
	messages.push_back("invisibility out...");
	messages.push_back("hyper out...");

	CrmFile gundata;

	ObjectGraphics::LoadGraphic((Config::GetMiscDir() + "gun.bin").c_str(), gunshapes);

#if 0
	for (size_t i = 0; i < gunshapes.size(); i++)
	{
		std::string name = "gunshapes";

		gunshapes[i].DumpDebug((name + std::to_string(i) + ".ppm").c_str());
	}
#endif

	gunsurfaces.resize(gunshapes.size());
	gunsurfacesblend.resize(gunshapes.size());

	for (size_t i = 0; i < gunshapes.size(); i++)
	{
		// Do I need to worry about endianness here?
		std::vector<uint32_t> tempdata;

		// get alpha right
		tempdata = gunshapes[i].data;

		for (auto &i : tempdata)
		{
			if (i == 1)
			{
				i = 0;
			}
			else
			{
				i |= 0xFF000000;
			}
		}

		gunsurfaces[i] = SDL_CreateRGBSurface(0, gunshapes[i].w, gunshapes[i].h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		gunsurfacesblend[i] = SDL_CreateRGBSurface(0, gunshapes[i].w, gunshapes[i].h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

		for (uint32_t y = 0; y < gunshapes[i].h; y++)
		{
			for (uint32_t x = 0; x < gunshapes[i].w; x++)
			{
				((uint32_t*)gunsurfaces[i]->pixels)[x + gunsurfaces[i]->pitch / 4 * y] = tempdata[y + x*gunshapes[i].h];
				((uint32_t*)gunsurfacesblend[i]->pixels)[x + gunsurfaces[i]->pitch / 4 * y] = tempdata[y + x*gunshapes[i].h]?0xFF808080:0xFFFFFFFF;
			}
		}
	}
}

void Hud::Render(SDL_Surface* surface, MapObject& player, Font& font)
{
	SDL_Rect dstrect;
	SDL_Rect srcrect;
	dstrect.x = 10;
	dstrect.y = 2;
	dstrect.w = 25 * 2 + 2;
	dstrect.h = 8;

	srcrect.x = 0;
	srcrect.y = 0;
	srcrect.w = player.data.ms.hitpoints * 2 + 1;
	srcrect.h = 9;

	int scale = surface->h / 256;

	if (scale < 1) scale = 1;

	dstrect.x *= scale;
	dstrect.y *= scale;
	dstrect.w *= scale;
	dstrect.h *= scale;

	SDL_BlitScaled(healthbar, NULL, surface, &dstrect);
	dstrect.w = srcrect.w*scale;
	SDL_BlitScaled(healthbaron, &srcrect, surface, &dstrect);

	dstrect.y += 12*scale;

	srcrect.x = 0;
	srcrect.y = 0;
	srcrect.w = player.data.ms.mega ? player.data.ms.mega / 50 : 0;
	srcrect.h = 9;

	dstrect.w = (25 * 2 + 2)*scale;
	SDL_BlitScaled(weaponbar, NULL, surface, &dstrect);
	dstrect.w = srcrect.w*scale;
	SDL_BlitScaled(healthbaron, &srcrect, surface, &dstrect);

	for (int i = 0; i < (6 - player.data.ms.reload); i++)
	{
		dstrect.x = 11 + 40 - 10 * i;
		dstrect.w = weaponsprites[player.data.ms.weapon]->w * scale;
		dstrect.h = weaponsprites[player.data.ms.weapon]->h * scale;

		dstrect.x *= scale;
		SDL_BlitScaled(weaponsprites[player.data.ms.weapon], NULL, surface, &dstrect);
	}

	if (player.data.ms.messtimer < 0)
	{
		font.PrintMessage(messages[player.data.ms.mess], 40, surface, scale);
	}

	//GUN. This is largely guesswork, it's not in the available gloom deluxe source as far as I can tell. There's not even a reference to gun.bin

	if (gunshapes.size())
	{
		dstrect.x = surface->w / 2 - gunshapes[0].xh * scale;
		dstrect.y = surface->h - gunshapes[0].h*scale;
		dstrect.w = gunshapes[0].w*scale;
		dstrect.h = gunshapes[0].h*scale;

		if (player.data.ms.fired)
		{
			SDL_Rect bullrect;

			// G3 does not have fire anims?
			if (gunshapes.size() > 2)
			{
				int wepshape = 2 + (player.data.ms.weapon + 1) / 2;
				bullrect.x = (surface->w - gunshapes[wepshape].w*scale) / 2;
				bullrect.y = surface->h - gunshapes[wepshape].h*scale;
				bullrect.w = gunshapes[wepshape].w*scale;
				bullrect.h = gunshapes[wepshape].h*scale;

				int zoom = GloomMaths::RndW() & 3;

				bullrect.x -= zoom*scale;
				bullrect.y -= zoom*scale;
				bullrect.w += 2*zoom*scale;
				bullrect.h += 2*zoom*scale;

				SDL_BlitScaled(gunsurfaces[wepshape], NULL, surface, &bullrect);
			}


			dstrect.y += 10 * scale;
		}
		else if (player.data.ms.bounce)
		{
			int16_t camrots[4];
			uint8_t ang = (player.data.ms.bounce>>1) * 0xff;

			GloomMaths::GetCamRotRaw(ang, camrots);

			int32_t xoffset = camrots[1];

			xoffset <<= 5;

			xoffset >>= 16;

			dstrect.x += xoffset*scale;

			int32_t yoffset = xoffset/2;

			if (yoffset < 0) yoffset = -yoffset;

			dstrect.y -= yoffset*scale;
		}

		dstrect.y += 25 * scale;

		if (player.data.ms.invisible)
		{
			SDL_SetSurfaceBlendMode(gunsurfacesblend[player.data.ms.fired ? 1 : 0], SDL_BLENDMODE_MOD);
			SDL_BlitScaled(gunsurfacesblend[player.data.ms.fired ? 1 : 0], NULL, surface, &dstrect);
		}
		else
		{
			SDL_BlitScaled(gunsurfaces[player.data.ms.fired ? 1 : 0], NULL, surface, &dstrect);
		}
	}

	if (Config::GetDebug())
	{
		std::string pos = "position ";
		std::string fps = "FPS ";

		pos += std::to_string(player.x.GetInt());
		pos += ' ';
		pos += std::to_string(player.z.GetInt());
		pos += ' ';
		pos += std::to_string(player.data.ms.rotquick.GetInt());

		fps += std::to_string(Config::GetFPS());

		font.PrintMessage(pos, 0, surface, 1);
		font.PrintMessage(fps,10, surface, 1);
	}
}
