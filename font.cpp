#include "font.h"

static uint16_t Get16(const uint8_t* p)
{
	return (static_cast<uint16_t>(p[0])) << 8 | static_cast<uint16_t>(p[1]);
}

static uint32_t Get32(const uint8_t* p)
{
	return (static_cast<uint16_t>(p[0])) << 24 | (static_cast<uint16_t>(p[1]) << 16) | (static_cast<uint16_t>(p[2])) << 8 | (static_cast<uint16_t>(p[3]) << 0);
}

void Font::SetPal(SDL_Surface* palsurface)
{
	for (int i = 0; i < glyphs; i++)
	{
		SDL_SetPaletteColors(surfaces[i]->format->palette, palsurface->format->palette->colors, 0, palsurface->format->palette->ncolors);
	}
}

bool Font::Load(CrmFile& file)
{
	bool G2mode = false;
	w = 6;
	h = 8;

	for (int i = 0; i < glyphs; i++)
	{
		uint32_t pos = Get32(file.data + 4 + 4*i);

		uint32_t shapepos = pos + Get32(file.data + pos);
		uint32_t maskpos = pos + 8;

		pos += 4;
		uint32_t modulo = Get16(file.data + pos);

		pos += 2;
		uint32_t widthheight = Get16(file.data + pos);

		uint32_t width = (widthheight & 0x3F) * 16;
		uint32_t height = (widthheight >> 6);

		width = modulo * 8;

		surfaces[i] = SDL_CreateRGBSurface(0, width, height / (G2mode?1:7), 8, 0, 0, 0, 0);
		SDL_SetColorKey(surfaces[i], 1, 0);

		//printf("%i %i %i %i\n", shapepos, maskpos, width, height);

		// assume always 7 bitplanes?

		for (uint32_t y = 0; y < height / (G2mode ? 1 : 7); y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				for (uint32_t plane = 0; plane < 7; plane++)
				{
					uint8_t thebyte = file.data[maskpos + (x + (y * 7 + plane)*width) / 8];

					if (plane == 0)
					{
						((char*)surfaces[i]->pixels)[x+y*surfaces[i]->pitch] = 0;
					}

					if (thebyte & (1 << (7 - (x % 8))))
					{
						((char*)surfaces[i]->pixels)[x+y*surfaces[i]->pitch] |= (1<<plane);
					}
				}
			}

			// now the palette

			uint32_t pos = Get32(file.data);
			int colnum = 0;
			while (pos < file.size)
			{
				SDL_Color col;
				col.a = 0xFF;
				col.r = file.data[pos + 0] & 0xf;
				col.g = file.data[pos + 1] >> 4;
				col.b = file.data[pos + 1] & 0xF;

				col.r <<= 4;
				col.g <<= 4;
				col.b <<= 4;

				SDL_SetPaletteColors(surfaces[i]->format->palette, &col, colnum, 1);

				colnum++;
				pos += 2;
			}
		}
	}
	return true;
}

void Font::Blit(int x, int y, int character, SDL_Surface* dest)
{
	SDL_Rect srcrect, dstrect;

	srcrect.w = w;
	srcrect.h = h;
	srcrect.x = 0;
	srcrect.y = 0;

	dstrect.w = w;
	dstrect.h = h;
	dstrect.x = x;
	dstrect.y = y;

	SDL_BlitSurface(surfaces[character], &srcrect, dest, &dstrect);
}

void Font::PrintMessage(std::string message, int y, SDL_Surface* dest)
{
	int xstart = dest->w/2 - message.length()*w / 2;

	for (auto c : message)
	{
		if (c != ' ')
		{
			if ((c >= '0') && (c <= '9'))
			{
				Blit(xstart, y, c - '0', dest);
			}
			else if ((c >= 'a') && (c <= 'z'))
			{
				Blit(xstart, y, c - 'a' + 10, dest);
			}
			else if ((c >= 'A') && (c <= 'Z'))
			{
				Blit(xstart, y, c - 'A' + 10, dest);
			}
			else if (c == '!')
			{
				Blit(xstart, y, 36, dest);
			}
			else if (c == '.')
			{
				Blit(xstart, y, 37, dest);
			}
			else if (c == ':')
			{
				Blit(xstart, y, 37, dest);
			}
		}

		xstart += w;
	}
}