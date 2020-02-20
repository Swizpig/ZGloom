#pragma once
#include <SDL2/SDL.h>
#include "font.h"

class MenuScreen
{
public:
	enum MenuReturn
	{
		MENURET_PLAY,
		MENURET_QUIT,
		MENURET_NOTHING
	};

	MenuScreen();
	void Render(SDL_Surface* src, SDL_Surface* dest, Font& font);
	void Clock() { timer++; };
	MenuReturn Update(SDL_Event& tevent);

private:
	enum MENUSTATUS
	{
		MENUSTATUS_MAIN,
		MENUSTATUS_KEYCONFIG
	};

	MENUSTATUS status;
	int selection;
	int timer;
};

