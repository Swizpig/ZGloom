#pragma once


#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include "gloommap.h"
#include "font.h"
#include "objectgraphics.h"

class Hud
{
	public:
		Hud();
		void Render(SDL_Surface* surface, MapObject& player, Font& font);

		enum MESSAGE
		{
			MESSAGES_DUMMY,
			MESSAGES_HEALTH_BONUS,
			MESSAGES_WEAPON_BOOST_FULL,
			MESSAGES_WEAPON_BOOST,
			MESSAGES_ULTRA_MEGA_OVERKILL,
			MESSAGES_MEGA_WEAPON_BOOST,
			MESSAGES_NEW_WEAPON,
			MESSAGES_INVISIBILITY,
			MESSAGES_HYPER,
			MESSAGES_BOUNCY,
			MESSAGES_THERMO,
			MESSAGES_MEGA_WEAPON_OUT,
			MESSAGES_THERMO_OUT,
			MESSAGES_INVISIBILITY_OUT,
			MESSAGES_HYPER_OUT
		};

	private:
		SDL_Surface* healthbar;
		SDL_Surface* healthbaron;
		SDL_Surface* weaponbar;
		SDL_Surface* weaponsprites[5];

		std::vector<Shape> gunshapes;
		std::vector<SDL_Surface*> gunsurfaces;
		std::vector<SDL_Surface*> gunsurfacesblend;

		std::vector<std::string> messages;
		
};
