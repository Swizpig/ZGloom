#include "menuscreen.h"
#include "config.h"

void MenuScreen::Render(SDL_Surface* src, SDL_Surface* dest, Font& font)
{
	SDL_BlitSurface(src, nullptr, dest, nullptr);
	bool flash = (timer / 5) & 1;

	if (status == MENUSTATUS_MAIN)
	{
		if (flash || (selection != 0)) font.PrintMessage("CONTINUE", 100, dest);
		if (flash || (selection != 1)) font.PrintMessage("CONFIGURE KEYS", 120, dest);
		if (flash || (selection != 2))
		{
			std::string mousestring = "MOUSE SENSITIVITY: ";
			mousestring += std::to_string(Config::GetMouseSens());
			font.PrintMessage(mousestring, 130, dest);
		}
		if (flash || (selection != 3)) font.PrintMessage("QUIT TO TITLE", 140, dest);
	}
	else if (status == MENUSTATUS_KEYCONFIG)
	{
		switch (selection)
		{
			case Config::KEY_UP:
				font.PrintMessage("PRESS KEY FOR FORWARD", 120, dest);
				break;
			case Config::KEY_DOWN:
				font.PrintMessage("PRESS KEY FOR BACK", 120, dest);
				break;
			case Config::KEY_LEFT:
				font.PrintMessage("PRESS KEY FOR ROTATE LEFT", 120, dest);
				break;
			case Config::KEY_RIGHT:
				font.PrintMessage("PRESS KEY FOR ROTATE RIGHT", 120, dest);
				break;
			case Config::KEY_SLEFT:
				font.PrintMessage("PRESS KEY FOR STRAFE LEFT", 120, dest);
				break;
			case Config::KEY_SRIGHT:
				font.PrintMessage("PRESS KEY FOR STRAFE RIGHT", 120, dest);
				break;
			case Config::KEY_STRAFEMOD:
				font.PrintMessage("PRESS KEY FOR STRAFE MODIFIER", 120, dest);
				break;
			case Config::KEY_SHOOT:
				font.PrintMessage("PRESS KEY FOR SHOOT", 120, dest);
				break;
		}
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
				if (selection == 4) selection = 3;
				break;
			case SDLK_UP:
				selection--;
				if (selection == -1) selection = 0;
				break;
			case SDLK_SPACE:
			case SDLK_RETURN:
			case SDLK_LCTRL:
				if (selection == 0) return MENURET_PLAY;
				if (selection == 1)
				{
					status = MENUSTATUS_KEYCONFIG;
					selection = Config::KEY_UP;
				}
				if (selection == 2)
				{
					int sens = Config::GetMouseSens() + 1;
					if (sens == 10) sens = 0;
					Config::SetMouseSens(sens);
				}
				if (selection == 3) return MENURET_QUIT;
			default:
				break;
			}
		}
		else if (status == MENUSTATUS_KEYCONFIG)
		{
			Config::SetKey((Config::keyenum)selection, SDL_GetScancodeFromKey(tevent.key.keysym.sym));
			selection++;
			if (selection == Config::KEY_END)
			{
				selection = 0;
				status = MENUSTATUS_MAIN;
			}

		}
	}

	return MENURET_NOTHING;
}
