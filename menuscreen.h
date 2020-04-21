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
		MENUSTATUS_KEYCONFIG,
		MENUSTATUS_SOUNDOPTIONS,
	};

	enum MENU_MAIN
	{
		MENU_MAIN_CONTINUE,
		MENU_MAIN_KEYCONF,
		MENU_MAIN_SOUNDOPTIONS,
		MENU_MAIN_MOUSESENS,
		MENU_MAIN_BLOODSIZE,
		MENU_MAIN_QUIT,
		MENU_MAIN_END
	};

	enum MENU_SOUND
	{
		MENU_SOUND_RETURN,
		MENU_SOUND_SFXVOL,
		MENU_SOUND_MUSVOL,
		MENU_SOUND_END
	};

	MENUSTATUS status;
	int selection;
	int timer;

	MenuReturn HandleMainMenu(SDL_Keycode sym);
	void HandleKeyMenu(SDL_Keycode sym);
	void HandleSoundMenu(SDL_Keycode sym);
};

