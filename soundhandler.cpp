#include "crmfile.h"

namespace SoundHandler
{
	enum Sounds
	{
		SOUND_SHOOT	  ,
		SOUND_SHOOT2  ,
		SOUND_SHOOT3  ,
		SOUND_SHOOT4  ,
		SOUND_SHOOT5  ,
		SOUND_GRUNT	  ,
		SOUND_GRUNT2  ,
		SOUND_GRUNT3  ,
		SOUND_GRUNT4  ,
		SOUND_TOKEN	  ,
		SOUND_DOOR	  ,
		SOUND_FOOTSTEP,
		SOUND_DIE	  ,
		SOUND_SPLAT	  ,
		SOUND_TELEPORT,
		SOUND_GHOUL	  ,
		SOUND_LIZARD  ,
		SOUND_LIZHIT  ,
		SOUND_TROLLMAD,
		SOUND_TROLLHIT,
		SOUND_ROBOT	  ,
		SOUND_ROBODIE ,
		SOUND_DRAGON  ,
		SOUND_END
	};

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

	CrmFile sounddata[SOUND_END];

	void Init()
	{
		for (auto i = 0; i < SOUND_END; i++)
		{
			sounddata[i].Load(soundnames[i]);
		}
	}
}