#pragma once
#include <SDL2/SDL.h>
#include "font.h"

class TitleScreen
{
	public:
		enum TitleReturn
		{
			TITLERET_PLAY,
			TITLERET_QUIT,
			TITLERET_NOTHING
		};

		TitleScreen();
		void Render(SDL_Surface* src, SDL_Surface* dest, Font& font);
		void Clock() { timer++; };
		TitleReturn Update(SDL_Event& tevent);

	private:
		enum TITLESTATUS
		{
			TITLESTATUS_MAIN,
			TITLESTATUS_ABOUT
		};

		TITLESTATUS status;
		int selection;
		int timer;

};

