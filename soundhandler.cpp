#include "crmfile.h"
#include "soundhandler.h"

#include <string>
#include "SDL.h"
#include "SDL_mixer.h"

namespace SoundHandler
{
	char* soundnames[] =
	{
		"sfxs/shoot.bin",
		"sfxs/shoot2.bin",
		"sfxs/shoot3.bin",
		"sfxs/shoot4.bin",
		"sfxs/shoot5.bin",
		"sfxs/grunt.bin",
		"sfxs/grunt2.bin",
		"sfxs/grunt3.bin",
		"sfxs/grunt4.bin",
		"sfxs/token.bin",
		"sfxs/door.bin",
		"sfxs/footstep.bin",
		"sfxs/die.bin",
		"sfxs/splat.bin",
		"sfxs/teleport.bin",
		"sfxs/ghoul.bin",
		"sfxs/lizard.bin",
		"sfxs/lizhit.bin",
		"sfxs/trollmad.bin",
		"sfxs/trollhit.bin",
		"sfxs/robot.bin",
		"sfxs/robodie.bin",
		"sfxs/dragon.bin"
	};

	static CrmFile sounddata[SOUND_END];
	static Mix_Chunk* sdlsounds[SOUND_END];

	static uint8_t wavheader[] = "RIFF    WAVEfmt ";

	uint8_t* CreateWAV(uint8_t* data)
	{
		uint32_t period = (uint32_t)(data[0]) << 8 | data[1];
		uint32_t length = (uint32_t)(data[2]) << 8 | data[3];

		// hello Paula
		period = 3546895 / period;
		length *= 2;

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

	void Init()
	{
		Mix_AllocateChannels(16);

		for (auto i = 0; i < SOUND_END; i++)
		{
			sounddata[i].Load(soundnames[i]);

			if (sounddata[i].data)
			{
				uint8_t* wavdata = CreateWAV(sounddata[i].data);

#if 1
				std::string fname = soundnames[i];

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