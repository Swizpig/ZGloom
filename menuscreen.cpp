#include "menuscreen.h"
#include "config.h"

void MenuScreen::Render(SDL_Surface* src, SDL_Surface* dest, Font& font)
{
	SDL_BlitSurface(src, nullptr, dest, nullptr);
	bool flash = (timer / 5) & 1;

	int scale = dest->h / 256;
	if (scale < 1) scale = 1;

	if (status == MENUSTATUS_MAIN)
	{
		int starty = 100 * scale;
		int yinc = 10 * scale;

		if (flash || (selection != MENU_MAIN_CONTINUE)) font.PrintMessage("CONTINUE", starty, dest, scale);
		starty += yinc*2;

		if (flash || (selection != MENU_MAIN_KEYCONF )) font.PrintMessage("CONFIGURE KEYS", starty, dest, scale);
		starty += yinc;

		if (flash || (selection != MENU_MAIN_SOUNDOPTIONS)) font.PrintMessage("SOUND OPTIONS", starty, dest, scale);
		starty += yinc;

		if (flash || (selection != MENU_MAIN_MOUSESENS))
		{
			std::string mousestring = "MOUSE SENSITIVITY: ";
			mousestring += std::to_string(Config::GetMouseSens());
			font.PrintMessage(mousestring, starty, dest, scale);
		}
		starty += yinc;

		if (flash || (selection != MENU_MAIN_BLOODSIZE))
		{
			std::string mousestring = "BLOOD SIZE: ";
			mousestring += std::to_string(Config::GetBlood());
			font.PrintMessage(mousestring, starty, dest, scale);
		}
		starty += yinc;

		if (flash || (selection != MENU_MAIN_QUIT)) font.PrintMessage("QUIT TO TITLE", starty, dest, scale);
		starty += yinc;
	}
	else if (status == MENUSTATUS_KEYCONFIG)
	{
		switch (selection)
		{
			case Config::KEY_UP:
				font.PrintMessage("PRESS KEY FOR FORWARD", 120 * scale, dest, scale);
				break;
			case Config::KEY_DOWN:
				font.PrintMessage("PRESS KEY FOR BACK", 120 * scale, dest, scale);
				break;
			case Config::KEY_LEFT:
				font.PrintMessage("PRESS KEY FOR ROTATE LEFT", 120 * scale, dest, scale);
				break;
			case Config::KEY_RIGHT:
				font.PrintMessage("PRESS KEY FOR ROTATE RIGHT", 120 * scale, dest, scale);
				break;
			case Config::KEY_SLEFT:
				font.PrintMessage("PRESS KEY FOR STRAFE LEFT", 120 * scale, dest, scale);
				break;
			case Config::KEY_SRIGHT:
				font.PrintMessage("PRESS KEY FOR STRAFE RIGHT", 120 * scale, dest, scale);
				break;
			case Config::KEY_STRAFEMOD:
				font.PrintMessage("PRESS KEY FOR STRAFE MODIFIER", 120 * scale, dest, scale);
				break;
			case Config::KEY_SHOOT:
				font.PrintMessage("PRESS KEY FOR SHOOT", 120 * scale, dest, scale);
				break;
		}
	}
	else if (status == MENUSTATUS_SOUNDOPTIONS)
	{
		int starty = 100 * scale;
		int yinc = 10 * scale;

		if (flash || (selection != MENU_SOUND_RETURN)) font.PrintMessage("RETURN", starty, dest, scale);
		starty += yinc * 2;

		if (flash || (selection != MENU_SOUND_SFXVOL))
		{
			std::string mousestring = "SFX VOLUME: ";
			mousestring += std::to_string(Config::GetSFXVol());
			font.PrintMessage(mousestring, starty, dest, scale);
		}
		starty += yinc;

		if (flash || (selection != MENU_SOUND_MUSVOL))
		{
			std::string mousestring = "MUSIC VOLUME: ";
			mousestring += std::to_string(Config::GetMusicVol());
			font.PrintMessage(mousestring, starty, dest, scale);
		}
		starty += yinc;
	}
}

MenuScreen::MenuScreen()
{
	status = MENUSTATUS_MAIN;
	selection = 0;
	timer = 0;
}

MenuScreen::MenuReturn MenuScreen::HandleMainMenu(SDL_Keycode sym)
{
	switch (sym)
	{
		case SDLK_DOWN:
			selection++;
			if (selection == MENU_MAIN_END) selection = MENU_MAIN_END - 1;
			break;
		case SDLK_UP:
			selection--;
			if (selection == -1) selection = 0;
			break;
		case SDLK_SPACE:
		case SDLK_RETURN:
		case SDLK_LCTRL:
			if (selection == MENU_MAIN_CONTINUE) return MENURET_PLAY;
			if (selection == MENU_MAIN_KEYCONF)
			{
				status = MENUSTATUS_KEYCONFIG;
				selection = Config::KEY_UP;
			}
			if (selection == MENU_MAIN_MOUSESENS)
			{
				int sens = Config::GetMouseSens() + 1;
				if (sens >= 10) sens = 0;
				Config::SetMouseSens(sens);
			}
			if (selection == MENU_MAIN_BLOODSIZE)
			{
				int sens = Config::GetBlood() + 1;
				if (sens >= 5) sens = 0;
				Config::SetBlood(sens);
			}
			if (selection == MENU_MAIN_SOUNDOPTIONS)
			{
				status = MENUSTATUS_SOUNDOPTIONS;
				selection = 0;
			}
			if (selection == MENU_MAIN_QUIT) return MENURET_QUIT;
		default:
			break;
	}

	return MENURET_NOTHING;
}

void MenuScreen::HandleKeyMenu(SDL_Keycode sym)
{
	Config::SetKey((Config::keyenum)selection, SDL_GetScancodeFromKey(sym));
	selection++;
	if (selection == Config::KEY_END)
	{
		selection = 0;
		status = MENUSTATUS_MAIN;
	}
}

void MenuScreen::HandleSoundMenu(SDL_Keycode sym)
{
	switch (sym)
	{
		case SDLK_DOWN:
			selection++;
			if (selection == MENU_SOUND_END) selection = MENU_SOUND_END - 1;
			break;
		case SDLK_UP:
			selection--;
			if (selection == -1) selection = 0;
			break;
		case SDLK_SPACE:
		case SDLK_RETURN:
		case SDLK_LCTRL:
			if (selection == MENU_SOUND_RETURN)
			{
				selection = 0;
				status = MENUSTATUS_MAIN;
			}
			if (selection == MENU_SOUND_MUSVOL)
			{
				int sens = Config::GetMusicVol() + 1;
				if (sens >= 10) sens = 0;
				Config::SetMusicVol(sens);
			}
			if (selection == MENU_SOUND_SFXVOL)
			{
				int sens = Config::GetSFXVol() + 1;
				if (sens >= 10) sens = 0;
				Config::SetSFXVol(sens);
			}

		default:
			break;
	}
}

MenuScreen::MenuReturn MenuScreen::Update(SDL_Event& tevent)
{
	if (tevent.type == SDL_KEYDOWN)
	{
		switch (status)
		{
		case MENUSTATUS_MAIN:
		{
			return HandleMainMenu(tevent.key.keysym.sym);
			break;
		}
		case MENUSTATUS_KEYCONFIG:
		{
			HandleKeyMenu(tevent.key.keysym.sym);
			break;
		}

		case MENUSTATUS_SOUNDOPTIONS:
		{
			HandleSoundMenu(tevent.key.keysym.sym);
		}

		default:
			break;
		}
	}

	return MENURET_NOTHING;
}
