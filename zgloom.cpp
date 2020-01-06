// zgloom.cpp : Defines the entry point for the console application.
//

#include "SDL.h"
#include "xmp/include/xmp.h"

#include "gloommap.h"
#include "script.h"
#include "crmfile.h"
#include "iffhandler.h"
#include "renderer.h"
#include "objectgraphics.h"
#include <iostream>
#include "gamelogic.h"

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
static int playingaudio;

static void fill_audio(void *udata, Uint8 *stream, int len)
{
	if (xmp_play_buffer((xmp_context)udata, stream, len, 0) < 0)
		playingaudio = 0;
}

void LoadPic(std::string name, SDL_Surface* render8)
{
	std::vector<uint8_t> pic;
	CrmFile picfile;
	CrmFile palfile;

	picfile.Load(name.c_str());
	palfile.Load((name+".pal").c_str());

	// is this some sort of weird AGA/ECS backwards compatible palette encoding? 4 MSBs, then LSBs?
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
	IffHandler::DecodeIff(picfile.data, pic);

	std::copy(pic.begin(), pic.begin() + 320 * 240, (uint8_t*)(render8->pixels));
}


int main(int argc, char* argv[])
{
	GloomMap gmap;
	Script script;

	xmp_context ctx;

	ctx = xmp_create_context();

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}


	CrmFile titlemusic;
	CrmFile intermissionmusic;

	titlemusic.Load("sfxs/med1");
	intermissionmusic.Load("sfxs/med2");

	SDL_Window* win = SDL_CreateWindow("ZGloom", 100, 100, 800, 600, SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN*/);
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

	SDL_Texture* rendertex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 320, 240);
	if (rendertex == nullptr)
	{
		std::cout << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Surface* render8 = SDL_CreateRGBSurface(0, 320, 240, 8, 0, 0, 0, 0);
	SDL_Surface* render32 = SDL_CreateRGBSurface(0, 320, 240, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	ObjectGraphics objgraphics;
	Renderer renderer;
	GameLogic logic;
	Camera cam;

	SDL_AddTimer(1000 / 25, my_callbackfunc, NULL);

	SDL_Event sEvent;

	bool done = true;

	bool showscreen = false;
	bool waiting = false;
	bool playing = false;

	while (done)
	{
		if ((playing == false) && (waiting == false))
		{
			std::string scriptstring;
			Script::ScriptOp sop;

			sop = script.NextLine(scriptstring);

			switch (sop)
			{
				case Script::SOP_SETPICT:
				{
					scriptstring.insert(0, "pics/");
					LoadPic(scriptstring, render8);
					break;
				}
				case Script::SOP_LOADFLAT:
				{
					//improve this, only supports 9 flats
					gmap.SetFlat(scriptstring[0] - '0');
					break;
				}
				case Script::SOP_DRAW:
				{
					showscreen = true;

					if (intermissionmusic.data)
					{
						SDL_AudioSpec a;

						a.freq = 44100;
						a.format = AUDIO_S16;
						a.channels = 2;
						a.samples = 2048;
						a.callback = fill_audio;
						a.userdata = ctx;

						if (SDL_OpenAudio(&a, NULL) < 0) {
							std::cout << "openaudio error" << SDL_GetError() << std::endl;
							return -1;
						}

						xmp_load_module_from_memory(ctx, intermissionmusic.data, intermissionmusic.size);

						xmp_start_player(ctx, 44100, 0);

						SDL_PauseAudio(0);
						playingaudio = 1;
					}
					break;
				}
				case Script::SOP_WAIT:
				{
					waiting = true;
					break;
				}
				case Script::SOP_PLAY:
				{
					cam.x.SetInt(0);
					cam.y = 120;
					cam.z.SetInt(0);
					cam.rot = 0;
					scriptstring.insert(0, "maps/");
					gmap.Load(scriptstring.c_str(), &objgraphics);
					//gmap.Load("maps/map1_4", &objgraphics);
					renderer.Init(render32, &gmap, &objgraphics);
					logic.Init(&gmap, &cam);
					showscreen = false;
					playing = true;

				}
			}
		}

		while (SDL_PollEvent(&sEvent))
		{
			if (sEvent.type == SDL_WINDOWEVENT)
			{
				if (sEvent.window.event == SDL_WINDOWEVENT_CLOSE)
				{
					done = false;
				}
			}

			if ((sEvent.type == SDL_KEYDOWN) || (sEvent.type == SDL_MOUSEBUTTONDOWN))
			{
				if (waiting)
				{
					waiting = false;
					if (intermissionmusic.data)
					{
						playingaudio = 0;
						xmp_end_player(ctx);
						xmp_release_module(ctx);
						SDL_CloseAudio();
					}
				}
			}

			if (sEvent.type == SDL_USEREVENT)
			{
				if (playing)
				{
					if (logic.Update(&cam))
					{
						playing = false;
					}
				}
			}
		}

		SDL_FillRect(render32, NULL, 0);

		if (playing) renderer.Render(&cam);
		if (showscreen)
		{
			SDL_BlitSurface(render8, NULL, render32, NULL);
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

