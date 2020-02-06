#pragma once


#include <SDL2/SDL.h>

class Hud
{
	public:
		Hud();
		void Render(SDL_Surface* surface, int weapon, int reload, int health);

	private:
		SDL_Surface* healthbar;
		SDL_Surface* healthbaron;
		SDL_Surface* weaponbar;
		SDL_Surface* weaponsprites[5];
};
