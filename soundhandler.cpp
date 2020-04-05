#include "crmfile.h"
#include "soundhandler.h"
#include "config.h"

#include <string>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

namespace SoundHandler
{
	static CrmFile sounddata[SOUND_END];
	static Mix_Chunk* sdlsounds[SOUND_END];

	static uint8_t wavheader[] = "RIFF    WAVEfmt ";

	uint8_t* CreateWAV(uint8_t* data, uint32_t filelength)
	{
		uint32_t period = (uint32_t)(data[0]) << 8 | data[1];
		uint32_t length = (uint32_t)(data[2]) << 8 | data[3];

		// hello Paula
		period = 3546895 / period;
		length *= 2;

		// 8bit killer has some corrupt samples?

		if ((length + 4) > filelength)
		{
			length = filelength - 4;
		}

		//signed to offset binary

		for (uint32_t i = 4; i < (length + 4); i++)
		{
			data[i] ^= 0x80;
		}

		uint8_t* wavbuffer = new uint8_t[length + 44];

		std::copy(wavheader, wavheader + 16, wavbuffer);

		//chunksize
		wavbuffer[4] = ((36 + length) >> 0) & 0xff;
		wavbuffer[5] = ((36 + length) >> 8) & 0xff;
		wavbuffer[6] = ((36 + length) >> 16) & 0xff;
		wavbuffer[7] = ((36 + length) >> 24) & 0xff;

		wavbuffer[16] = (16 >> 0) & 0xff;
		wavbuffer[17] = (16 >> 8) & 0xff;
		wavbuffer[18] = (16 >> 16) & 0xff;
		wavbuffer[19] = (16 >> 24) & 0xff;

		// format, PCM
		wavbuffer[20] = 1;
		wavbuffer[21] = 0;

		//mono
		wavbuffer[22] = 1;
		wavbuffer[23] = 0;

		// samplerate
		wavbuffer[24] = (period >> 0) & 0xff;
		wavbuffer[25] = (period >> 8) & 0xff;
		wavbuffer[26] = (period >> 16) & 0xff;
		wavbuffer[27] = (period >> 24) & 0xff;

		// byterate, same thing as mono and 8bps
		wavbuffer[28] = (period >> 0) & 0xff;
		wavbuffer[29] = (period >> 8) & 0xff;
		wavbuffer[30] = (period >> 16) & 0xff;
		wavbuffer[31] = (period >> 24) & 0xff;

		//blockalign
		wavbuffer[32] = (1 >> 0) & 0xff;
		wavbuffer[33] = (1 >> 8) & 0xff;

		//bps
		wavbuffer[34] = (8 >> 0) & 0xff;
		wavbuffer[35] = (8 >> 8) & 0xff;

		//
		wavbuffer[36] = 'd';
		wavbuffer[37] = 'a';
		wavbuffer[38] = 't';
		wavbuffer[39] = 'a';

		wavbuffer[40] = ((length) >> 0) & 0xff;
		wavbuffer[41] = ((length) >> 8) & 0xff;
		wavbuffer[42] = ((length) >> 16) & 0xff;
		wavbuffer[43] = ((length) >> 24) & 0xff;

		std::copy(data + 4, data+length+4, wavbuffer + 44);

		return wavbuffer;
	}

	void Quit()
	{
		Mix_CloseAudio();
		//is this strictly needed? I've not loaded any libs, Mix_init was not used.
		Mix_Quit();
	}

	void Init()
	{

		if (Mix_OpenAudio(22050, AUDIO_S16LSB, 2, 1024))
		{
			std::cout << "openaudio error" << Mix_GetError() << std::endl;
			return;
		}

		Mix_AllocateChannels(16);

		for (auto i = 0; i < SOUND_END; i++)
		{
			sounddata[i].Load(Config::GetSoundFilename((SoundHandler::Sounds)i).c_str());

			if (sounddata[i].data)
			{
				uint8_t* wavdata = CreateWAV(sounddata[i].data, sounddata[i].size);

#if 0
				std::string fname = Config::GetSoundFilename((SoundHandler::Sounds)i);

				fname += ".wav";

				FILE* fdebug = fopen(fname.c_str(), "wb");
				fwrite(wavdata, 1, sounddata[i].size - 4 + 44, fdebug);
				fclose(fdebug);
#endif

				SDL_RWops* rwop = SDL_RWFromMem(wavdata, sounddata[i].size - 4 + 44);

				sdlsounds[i] = Mix_LoadWAV_RW(rwop, 0);

				SDL_RWclose(rwop);

				delete[] wavdata;
			}
		}
	}

	void Play(int i)
	{
		Mix_PlayChannel(-1, sdlsounds[i], 0);
	}
}
