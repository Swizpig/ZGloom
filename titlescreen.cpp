#include "titlescreen.h"

void TitleScreen::Render(SDL_Surface* src, SDL_Surface* dest, Font& font)
{
	SDL_BlitSurface(src, nullptr, dest, nullptr);
	bool flash = (timer / 5) & 1;

	if (status == TITLESTATUS_MAIN)
	{
		if (flash || (selection != 0)) font.PrintMessage("PLAY GLOOM", 150, dest, 1);
		if (flash || (selection != 1)) font.PrintMessage("ABOUT GLOOM", 170, dest, 1);
		if (flash || (selection != 2)) font.PrintMessage("EXIT GLOOM", 190, dest, 1);
	}
	else
	{
		font.PrintMessage("GLOOM", 30, dest, 1);

		font.PrintMessage("A BLACK MAGIC GAME", 50, dest, 1);

		font.PrintMessage("PROGRAMMED BY MARK SIBLY", 70, dest, 1);
		font.PrintMessage("GRAPHICS BY THE BUTLER BROTHERS", 80, dest, 1);
		font.PrintMessage("MUSIC BY KEV STANNARD", 90, dest, 1);
		font.PrintMessage("AUDIO BY US", 100, dest, 1);
		font.PrintMessage("PRODUCED BY US", 110, dest, 1);
		font.PrintMessage("DESIGNED BY US", 120, dest, 1);
		font.PrintMessage("GAME CODED IN DEVPAC2", 130, dest, 1);
		font.PrintMessage("UTILITIES CODED IN BLITZ BASIC 2", 140, dest, 1);
		font.PrintMessage("RENDERED IN DPAINT3 AND DPAINT4", 150, dest, 1);
		font.PrintMessage("DECRUNCHING CODE BY THOMAS SCHWARZ", 160, dest, 1);
	}
}

TitleScreen::TitleScreen()
{
	status = TITLESTATUS_MAIN;
	selection = 0;
	timer = 0;
}

TitleScreen::TitleReturn TitleScreen::Update(SDL_Event& tevent)
{
	if (tevent.type == SDL_KEYDOWN)
	{
		if (status == TITLESTATUS_MAIN)
		{
			switch (tevent.key.keysym.sym)
			{
			case SDLK_DOWN:
				selection++;
				if (selection == 3) selection = 2;
				break;
			case SDLK_UP:
				selection--;
				if (selection == -1) selection = 0;
				break;
			case SDLK_SPACE:
			case SDLK_RETURN:
			case SDLK_LCTRL:
				if (selection == 0) return TITLERET_PLAY;
				if (selection == 2) return TITLERET_QUIT;
				if (selection == 1) status = TITLESTATUS_ABOUT;
			default:
				break;
			}
		}
		else
		{
			status = TITLESTATUS_MAIN;
		}
	}

	return TITLERET_NOTHING;
}
