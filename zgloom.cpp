// zgloom.cpp : Defines the entry point for the console application.
//

#include "SDL.h"

#include "gloommap.h"
#include "script.h"
#include "crmfile.h"
#include "iffhandler.h"
#include "renderer.h"

#include <iostream>

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


int main(int argc, char* argv[])
{
	GloomMap gmap;
	Script script;
	CrmFile blackmagic;
	CrmFile blackmagicpal;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}

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

	std::vector<uint8_t> pic;
	blackmagic.Load("pics/gothic");
	blackmagicpal.Load("pics/gothic.pal");

	// is this some sort of weird AGA/ECS backwards compatible palette encoding? 4 MSBs, then LSBs?
	for (uint32_t c = 0; c < blackmagicpal.size / 4; c++)
	{
		SDL_Color col;
		col.a = 0xFF;
		col.r = blackmagicpal.data[c * 4 + 0]&0xf;
		col.g = blackmagicpal.data[c * 4 + 1]>>4;
		col.b = blackmagicpal.data[c * 4 + 1]&0xF;

		col.r <<= 4;
		col.g <<= 4;
		col.b <<= 4;

		col.r |= blackmagicpal.data[c * 4 + 2] & 0xf;
		col.g |= blackmagicpal.data[c * 4 + 3] >> 4;
		col.b |= blackmagicpal.data[c * 4 + 3] & 0xF;

		SDL_SetPaletteColors(render8->format->palette, &col, c, 1);
	}
	IffHandler::DecodeIff(blackmagic.data, pic);

	gmap.Load("maps/map1_1");
	gmap.SetFlat(1);
	gmap.DumpDebug();

	Renderer renderer;

	renderer.Init(render32, &gmap);

	SDL_AddTimer(1000 / 50, my_callbackfunc, NULL);

	std::copy(pic.begin(), pic.begin()+320*240, (uint8_t*)(render8->pixels));

	SDL_Event sEvent;

	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	bool done = true;

	Camera cam;

	cam.x.SetInt(0x1000);
	cam.z.SetInt(0x1000);
	cam.rot = 0;

	cam.y = 120;
	int16_t camdir = 1;

	while (done)
	{
		while (SDL_PollEvent(&sEvent))
		{
			if (sEvent.type == SDL_WINDOWEVENT)
			{
				if (sEvent.window.event == SDL_WINDOWEVENT_CLOSE)
				{
					done = false;
				}
			}

			Quick inc;

			inc.SetVal(0x70000);

			Quick camrots[4];

			GloomMaths::GetCamRot(cam.rot, camrots);			

			if (sEvent.type == SDL_USEREVENT)
			{
				//printf("t\n");
				if (keystate[SDL_SCANCODE_UP])
				{
					// U 
					cam.x = cam.x + camrots[1]*inc;
					cam.z = cam.z + camrots[0]*inc;
					cam.y += camdir;
				}
				if (keystate[SDL_SCANCODE_DOWN])
				{
					// D
					cam.x = cam.x - camrots[1] * inc;
					cam.z = cam.z - camrots[0] * inc;
					cam.y += camdir;
				}
				if (keystate[SDL_SCANCODE_LEFT])
				{
					//L
					//cam.x = cam.x - inc;
					cam.rot++;
				}
				if (keystate[SDL_SCANCODE_RIGHT])
				{
					//R
					//cam.x = cam.x + inc;
					cam.rot--;
				}

				if (cam.y > 136) camdir = -1;
				if (cam.y < 120) camdir =  1;
			}
		}

		SDL_FillRect(render32, NULL, 0);
		renderer.Render(&cam);

		//sRenderer.Draw();
		//SDL_BlitSurface(render8, NULL, render32, NULL);
		SDL_UpdateTexture(rendertex, NULL, render32->pixels, render32->pitch);
		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, rendertex, NULL, NULL);
		SDL_RenderPresent(ren);
	}

	SDL_FreeSurface(render8);
	SDL_FreeSurface(render32);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}

