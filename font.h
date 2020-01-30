#pragma once
#include "crmfile.h"
#include <SDL2/SDL.h>
#include <string>

class Font
{
	public:
		
	bool Load(CrmFile& file);
	bool Load2(CrmFile& file);
	void SetPal(SDL_Surface* palsurface); 
	void Blit(int x, int y, int character, SDL_Surface* dest);
	void PrintMessage(std::string message,  int y, SDL_Surface* dest);
	void PrintMultiLineMessage(std::string message, int y, SDL_Surface* dest);
	
	private:

	static const int glyphs = 40;
	SDL_Surface* surfaces[glyphs];
	int w;
	int h;

	public:

	SDL_Palette* GetPalette(){
		return surfaces[0]->format->palette;
	};
};
