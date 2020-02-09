#include "menuscreen.h"

void MenuScreen::Render(SDL_Surface* src, SDL_Surface* dest, Font& font)
{
	SDL_BlitSurface(src, nullptr, dest, nullptr);
	bool flash = (timer / 5) & 1;

	if (status == MENUSTATUS_MAIN)
	{
		if (flash || (selection != 0)) font.PrintMessage("CONTINUE", 100, dest);
		if (flash || (selection != 1)) font.PrintMessage("QUIT TO TITLE", 120, dest);
	}
}

MenuScreen::MenuScreen()
{
	status = MENUSTATUS_MAIN;
	selection = 0;
	timer = 0;
}

MenuScreen::MenuReturn MenuScreen::Update(SDL_Event& tevent)
{
	if (tevent.type == SDL_KEYDOWN)
	{
		if (status == MENUSTATUS_MAIN)
		{
			switch (tevent.key.keysym.sym)
			{
			case SDLK_DOWN:
				selection++;
				if (selection == 2) selection = 1;
				break;
			case SDLK_UP:
				selection--;
				if (selection == -1) selection = 0;
				break;
			case SDLK_SPACE:
			case SDLK_RETURN:
			case SDLK_LCTRL:
				if (selection == 0) return MENURET_PLAY;
				if (selection == 1) return MENURET_QUIT;
			default:
				break;
			}
		}
		else
		{
			status = MENUSTATUS_MAIN;
		}
	}

	return MENURET_NOTHING;
}
