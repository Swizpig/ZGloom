// zgloom.cpp : Defines the entry point for the console application.
//

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "xmp/include/xmp.h"

#include "config.h"
#include "gloommap.h"
#include "script.h"
#include "crmfile.h"
#include "iffhandler.h"
#include "renderer.h"
#include "objectgraphics.h"
#include <iostream>
#include "gamelogic.h"
#include "soundhandler.h"
#include "font.h"
#include "titlescreen.h"
#include "menuscreen.h"
#include "hud.h"

Uint32 my_callbackfunc(Uint32 interval, void *param)
{
	SDL_Event event;
	SDL_UserEvent userevent;

	/* In this example, our callback pushes an SDL_USEREVENT event
	into the queue, and causes our callback to be called again at the
	same interval: */

	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = NULL;
	userevent.data2 = NULL;

	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);
	return(interval);
}

static void fill_audio(void *udata, Uint8 *stream, int len)
{
	auto res = xmp_play_buffer((xmp_context)udata, stream, len, 0);
}

void LoadPic(std::string name, SDL_Surface* render8)
{
	std::vector<uint8_t> pic;
	CrmFile picfile;
	CrmFile palfile;

	picfile.Load(name.c_str());
	palfile.Load((name+".pal").c_str());

	SDL_FillRect(render8, nullptr, 0);

	// is this some sort of weird AGA/ECS backwards compatible palette encoding? 4 MSBs, then LSBs?
	// Update: Yes, yes it is. 
	for (uint32_t c = 0; c < palfile.size / 4; c++)
	{
		SDL_Color col;
		col.a = 0xFF;
		col.r = palfile.data[c * 4 + 0] & 0xf;
		col.g = palfile.data[c * 4 + 1] >> 4;
		col.b = palfile.data[c * 4 + 1] & 0xF;

		col.r <<= 4;
		col.g <<= 4;
		col.b <<= 4;

		col.r |= palfile.data[c * 4 + 2] & 0xf;
		col.g |= palfile.data[c * 4 + 3] >> 4;
		col.b |= palfile.data[c * 4 + 3] & 0xF;

		SDL_SetPaletteColors(render8->format->palette, &col, c, 1);
	}

	uint32_t width = 0;

	IffHandler::DecodeIff(picfile.data, pic, width);

	if (width == render8->w)
	{
		if (pic.size() > (size_t)(render8->w * render8->h))
		{
			pic.resize(render8->w * render8->h);
		}
		std::copy(pic.begin(), pic.begin() + pic.size(), (uint8_t*)(render8->pixels));
	}
	else
	{
		// gloom 3 has some odd-sized intermission pictures. Do a line-by-line copy.

		uint32_t p = 0;
		uint32_t y = 0;

		if (pic.size() > (width * render8->h))
		{
			pic.resize(width * render8->h);
		}

		while (p < pic.size())
		{
			std::copy(pic.begin() + p, pic.begin() + p + render8->w, (uint8_t*)(render8->pixels) + y*render8->pitch);

			p += width;
			y++;
		}
	}
}

enum GameState
{
	STATE_PLAYING,
	STATE_PARSING,
	STATE_WAITING,
	STATE_MENU,
	STATE_TITLE
};


int main(int argc, char* argv[])
{
	/* AUTODETECT ZM FIRST!*/
	if (FILE* file = fopen("stuf/stages", "r"))
	{
		fclose(file);
		Config::SetZM(true);
	}
	Config::Init();

	GloomMap gmap;
	Script script;
	TitleScreen titlescreen;
	MenuScreen menuscreen;
	GameState state = STATE_TITLE;

	xmp_context ctx;

	ctx = xmp_create_context();

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	int renderwidth = 320;
	int renderheight = 256;

	int windowwidth = 960;
	int windowheight = 768;


	CrmFile titlemusic;
	CrmFile intermissionmusic;
	CrmFile titlepic;

	titlemusic.Load(Config::GetMusicFilename(0).c_str());
	intermissionmusic.Load(Config::GetMusicFilename(1).c_str());

	if (Mix_OpenAudio(22050, AUDIO_S16LSB, 2, 1024))
	{
		std::cout << "openaudio error" << Mix_GetError() << std::endl;
		return -1;
	}

	SoundHandler::Init();

	SDL_Window* win = SDL_CreateWindow("ZGloom", 100, 100, windowwidth, windowheight, SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN*/);
	if (win == nullptr)
	{
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC*/);
	if (ren == nullptr)
	{
		std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Texture* rendertex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, renderwidth, renderheight);
	if (rendertex == nullptr)
	{
		std::cout << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_ShowCursor(SDL_DISABLE);

	SDL_Surface* fontsurface = SDL_CreateRGBSurface(0, 320, 256, 8, 0, 0, 0, 0);
	SDL_Surface* render8 = SDL_CreateRGBSurface(0, 320, 256, 8, 0, 0, 0, 0);
	SDL_Surface* intermissionscreen = SDL_CreateRGBSurface(0, 320, 256, 8, 0, 0, 0, 0);
	SDL_Surface* titlebitmap = SDL_CreateRGBSurface(0, 320, 256, 8, 0, 0, 0, 0);
	SDL_Surface* render32 = SDL_CreateRGBSurface(0, renderwidth, renderheight, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	ObjectGraphics objgraphics;
	Renderer renderer;
	GameLogic logic;
	Camera cam;
	Hud hud;

	logic.Init(&objgraphics);
	SDL_AddTimer(1000 / 25, my_callbackfunc, NULL);

	SDL_Event sEvent;

	bool notdone = true;

#if 1
	Font smallfont, bigfont;
	CrmFile fontfile;
	fontfile.Load((Config::GetMiscDir() + "bigfont2.bin").c_str());
	if (fontfile.data)
	{
		bigfont.Load2(fontfile);
		smallfont.Load2(fontfile);
	}
	else
	{
		fontfile.Load((Config::GetMiscDir() + "smallfont.bin").c_str());
		if (fontfile.data)smallfont.Load(fontfile);
		fontfile.Load((Config::GetMiscDir() + "bigfont.bin").c_str());
		if (fontfile.data)bigfont.Load(fontfile);
	}
#endif

	titlepic.Load((Config::GetPicsDir() + "title").c_str());

	if (titlepic.data)
	{
		LoadPic(Config::GetPicsDir() + "title", titlebitmap);
	}
	else
	{
		LoadPic(Config::GetPicsDir() + "blackmagic", titlebitmap);
	}

	if (titlemusic.data)
	{
		if (xmp_load_module_from_memory(ctx, titlemusic.data, titlemusic.size))
		{
			std::cout << "music error";
		}

		if (xmp_start_player(ctx, 22050, 0))
		{
			std::cout << "music error";
		}
		Mix_HookMusic(fill_audio, ctx);
	}

	std::string intermissiontext;

	bool intermissionmusplaying = false;
	bool fullscreen = false;
	bool printscreen = false;
	int screennum = 0;

	Mix_Volume(-1, 32);

	while (notdone)
	{
		if (state == STATE_PARSING)
		{
			std::string scriptstring;
			Script::ScriptOp sop;

			sop = script.NextLine(scriptstring);

			switch (sop)
			{
				case Script::SOP_SETPICT:
				{
					scriptstring.insert(0, Config::GetPicsDir());
					LoadPic(scriptstring, intermissionscreen);
					SDL_SetPaletteColors(render8->format->palette, intermissionscreen->format->palette->colors, 0, 256);
					break;
				}
				case Script::SOP_LOADFLAT:
				{
					//improve this, only supports 9 flats
					gmap.SetFlat(scriptstring[0] - '0');
					break;
				}
				case Script::SOP_TEXT:
				{
					 intermissiontext = scriptstring;
					 break;
				}
				case Script::SOP_DRAW:
				{
					if (intermissionmusic.data)
					{
						if (xmp_load_module_from_memory(ctx, intermissionmusic.data, intermissionmusic.size))
						{
							std::cout << "music error";
						}

						if (xmp_start_player(ctx, 22050, 0))
						{
							std::cout << "music error";
						}
						Mix_HookMusic(fill_audio, ctx);
						intermissionmusplaying = true;
					}
					break;
				}
				case Script::SOP_WAIT:
				{
					state = STATE_WAITING;


					SDL_SetPaletteColors(render8->format->palette, smallfont.GetPalette()->colors, 0, 16);
					SDL_BlitSurface(intermissionscreen, NULL, render8, NULL);
					smallfont.PrintMultiLineMessage(intermissiontext, 220, render8);
					break;
				}
				case Script::SOP_PLAY:
				{
					cam.x.SetInt(0);
					cam.y = 120;
					cam.z.SetInt(0);
					cam.rotquick.SetInt(0);
					scriptstring.insert(0, Config::GetLevelDir());
					gmap.Load(scriptstring.c_str(), &objgraphics);
					//gmap.Load("maps/map1_4", &objgraphics);
					renderer.Init(render32, &gmap, &objgraphics);
					logic.InitLevel(&gmap, &cam, &objgraphics);
					state = STATE_PLAYING;
					break;
				}
				case Script::SOP_END:
				{
					state = STATE_TITLE;
					if (intermissionmusic.data && intermissionmusplaying)
					{
						Mix_HookMusic(nullptr, nullptr);
						xmp_end_player(ctx);
						xmp_release_module(ctx);
						intermissionmusplaying = false;
					}
					if (titlemusic.data)
					{
						if (xmp_load_module_from_memory(ctx, titlemusic.data, titlemusic.size))
						{
							std::cout << "music error";
						}

						if (xmp_start_player(ctx, 22050, 0))
						{
							std::cout << "music error";
						}
						Mix_HookMusic(fill_audio, ctx);
					}
					break;
				}
			}
		}

		if (state == STATE_TITLE)
		{
			SDL_SetPaletteColors(render8->format->palette, titlebitmap->format->palette->colors, 0, 256);
			titlescreen.Render(titlebitmap, render8, smallfont);
		}

		while (SDL_PollEvent(&sEvent))
		{
			if (sEvent.type == SDL_WINDOWEVENT)
			{
				if (sEvent.window.event == SDL_WINDOWEVENT_CLOSE)
				{
					notdone = false;
				}
			}

			if ((sEvent.type == SDL_KEYDOWN) && (sEvent.key.keysym.sym == SDLK_SPACE ||
				sEvent.key.keysym.sym == SDLK_RETURN ||
			   sEvent.key.keysym.sym == SDLK_LCTRL))
			{
				if (state == STATE_WAITING)
				{
					state = STATE_PARSING;
					if (intermissionmusic.data)
					{
						Mix_HookMusic(nullptr, nullptr);
						xmp_end_player(ctx);
						xmp_release_module(ctx);
						intermissionmusplaying = false;
					}
				}
			}

			if (sEvent.type == SDL_KEYDOWN)
			{
				if (state == STATE_TITLE)
				{
					switch (titlescreen.Update(sEvent))
					{
						case TitleScreen::TITLERET_PLAY:
							state = STATE_PARSING;
							logic.Init(&objgraphics);
							if (titlemusic.data)
							{
								Mix_HookMusic(nullptr, nullptr);
								xmp_end_player(ctx);
								xmp_release_module(ctx);
							}
							break;
						case TitleScreen::TITLERET_QUIT:
							notdone = false;
							break;
						default:
							break;
					}
				}
				if (state == STATE_MENU)
				{
					switch (menuscreen.Update(sEvent))
					{
						case MenuScreen::MENURET_PLAY:
							state = STATE_PLAYING;
							break;
						case MenuScreen::MENURET_QUIT:
							script.Reset();
							state = STATE_TITLE;
							if (titlemusic.data)
							{
								if (xmp_load_module_from_memory(ctx, titlemusic.data, titlemusic.size))
								{
									std::cout << "music error";
								}

								if (xmp_start_player(ctx, 22050, 0))
								{
									std::cout << "music error";
								}
								Mix_HookMusic(fill_audio, ctx);
							}
							break;
						default:
							break;
					}
				}
				if ((state == STATE_PLAYING) && (sEvent.key.keysym.sym == SDLK_ESCAPE))
				{
					state = STATE_MENU;
				}
			}

			if ((sEvent.type == SDL_KEYDOWN) && sEvent.key.keysym.sym == SDLK_F12)
			{
				if (!fullscreen)
				{
					SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN);
				}
				else
				{
					SDL_SetWindowFullscreen(win, 0);
				}

				fullscreen = !fullscreen;
			}

			if ((sEvent.type == SDL_KEYDOWN) && sEvent.key.keysym.sym == SDLK_PRINTSCREEN)
			{
				printscreen = true;
			}

			if (sEvent.type == SDL_USEREVENT)
			{
				if (state == STATE_PLAYING)
				{
					if (logic.Update(&cam))
					{
						state = STATE_PARSING;
					}
				}
				if (state == STATE_TITLE)
				{
					titlescreen.Clock();
				}
				if (state == STATE_MENU)
				{
					menuscreen.Clock();
				}
			}
		}

		SDL_FillRect(render32, NULL, 0);

		if (state == STATE_PLAYING)
		{
			renderer.SetTeleEffect(logic.GetTeleEffect());
			renderer.SetPlayerHit(logic.GetPlayerHit());
			renderer.Render(&cam);
			MapObject pobj = logic.GetPlayerObj();
			hud.Render(render32, pobj, smallfont);
		}
		if (state == STATE_MENU)
		{
			renderer.Render(&cam);
			menuscreen.Render(render32, render32, smallfont);
		}

		if ((state == STATE_WAITING) || (state == STATE_TITLE))
		{
			SDL_BlitSurface(render8, NULL, render32, NULL);
		}

		if (printscreen)
		{
			std::string filename("img");

			filename += std::to_string(screennum);
			filename += ".bmp";
			screennum++;

			SDL_SaveBMP(render32, filename.c_str());
			printscreen = false;
		}
		SDL_UpdateTexture(rendertex, NULL, render32->pixels, render32->pitch);
		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, rendertex, NULL, NULL);
		SDL_RenderPresent(ren);
	}

	xmp_free_context(ctx);

	SDL_FreeSurface(render8);
	SDL_FreeSurface(render32);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}

