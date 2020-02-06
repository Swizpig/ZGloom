#include "config.h"
#include "objectgraphics.h"
#include "soundhandler.h"
#include <string>

#define ZOMBIE_MASSACRE 0

namespace Config
{
	std::string GetScriptName()
	{
#if ZOMBIE_MASSACRE
		std::string result = "stuf/stages";
		return result;
#else
		std::string result = "misc/script";
		return result;
#endif
	}

	std::string GetMiscDir()
	{
#if ZOMBIE_MASSACRE
		std::string result = "stuf/";
		return result;
#else
		std::string result = "misc/";
		return result;
#endif
	}

	std::string GetPicsDir()
	{
#if ZOMBIE_MASSACRE
		std::string result = "pixs/";
		return result;
#else
		std::string result = "pics/";
		return result;
#endif
	}

	std::string GetLevelDir()
	{
#if ZOMBIE_MASSACRE
		std::string result = "lvls/";
		return result;
#else
		std::string result = "maps/";
		return result;
#endif
	}

	std::string objectfilenames[ObjectGraphics::OGT_END];
	std::string soundfilenames[SoundHandler::SOUND_END];

	std::string GetObjectFilename(ObjectGraphics::ObjectGraphicType i)
	{
		return objectfilenames[i];
	}

	std::string GetGoreFilename(ObjectGraphics::ObjectGraphicType i)
	{
		return objectfilenames[i] + "2";
	}

	std::string GetSoundFilename(SoundHandler::Sounds i)
	{
		return soundfilenames[i];
	}

	std::string GetMusicFilename(int i)
	{
		std::string result;
#if ZOMBIE_MASSACRE
		if (i == 0) result = "musi/meda"; else result = "musi/medb";
#else
		if (i == 0) result = "sfxs/med1"; else result = "sfxs/med2";
#endif

		return result;
	}

	void Init()
	{
#if ZOMBIE_MASSACRE
		// some of this is guesswork, need to check
		objectfilenames[ObjectGraphics::OGT_TOKENS] = "char/pwrups";
		objectfilenames[ObjectGraphics::OGT_MARINE] = "char/troopr";
		objectfilenames[ObjectGraphics::OGT_BALDY] = "char/zombi";
		objectfilenames[ObjectGraphics::OGT_TERRA] = "char/fatzo";
		objectfilenames[ObjectGraphics::OGT_PHANTOM] = "char/zomboid";
		objectfilenames[ObjectGraphics::OGT_GHOUL] = "char/ghost";
		objectfilenames[ObjectGraphics::OGT_DRAGON] = "char/zombie";
		objectfilenames[ObjectGraphics::OGT_LIZARD] = "char/skinny";
		objectfilenames[ObjectGraphics::OGT_DEMON] = "char/zocom";
		objectfilenames[ObjectGraphics::OGT_DEATHHEAD] = "char/dows-head";
		objectfilenames[ObjectGraphics::OGT_TROLL] = "char/james";


		//double check these
		soundfilenames[SoundHandler::SOUND_SHOOT] = "musi/shoot.bin";
		soundfilenames[SoundHandler::SOUND_SHOOT2] = "musi/shoot2.bin";
		soundfilenames[SoundHandler::SOUND_SHOOT3] = "musi/shoot3.bin";
		soundfilenames[SoundHandler::SOUND_SHOOT4] = "musi/shoot4.bin";
		soundfilenames[SoundHandler::SOUND_SHOOT5] = "musi/shoot5.bin";
		soundfilenames[SoundHandler::SOUND_GRUNT] = "musi/groan.bin";
		soundfilenames[SoundHandler::SOUND_GRUNT2] = "musi/groan2.bin";
		soundfilenames[SoundHandler::SOUND_GRUNT3] = "musi/groan3.bin";
		soundfilenames[SoundHandler::SOUND_GRUNT4] = "musi/groan4.bin";
		soundfilenames[SoundHandler::SOUND_TOKEN] = "musi/pwrup.bin";
		soundfilenames[SoundHandler::SOUND_DOOR] = "musi/door.bin";
		soundfilenames[SoundHandler::SOUND_FOOTSTEP] = "musi/footstep.bin";
		soundfilenames[SoundHandler::SOUND_DIE] = "musi/die.bin";
		soundfilenames[SoundHandler::SOUND_SPLAT] = "musi/splat.bin";
		soundfilenames[SoundHandler::SOUND_TELEPORT] = "musi/teleport.bin";
		soundfilenames[SoundHandler::SOUND_GHOUL] = "musi/ghost.bin";
		soundfilenames[SoundHandler::SOUND_LIZARD] = "musi/skinny.bin";
		soundfilenames[SoundHandler::SOUND_LIZHIT] = "musi/skihit.bin";
		soundfilenames[SoundHandler::SOUND_TROLLMAD] = "musi/jamesmad.bin";
		soundfilenames[SoundHandler::SOUND_TROLLHIT] = "musi/jameshit.bin";
		soundfilenames[SoundHandler::SOUND_ROBOT] = "musi/fatzo.bin";
		soundfilenames[SoundHandler::SOUND_ROBODIE] = "musi/fatzdie.bin";
		soundfilenames[SoundHandler::SOUND_DRAGON] = "musi/zombie.bin";
#else
		objectfilenames[ObjectGraphics::OGT_TOKENS] = "objs/tokens";
		objectfilenames[ObjectGraphics::OGT_MARINE] = "objs/marine";
		objectfilenames[ObjectGraphics::OGT_BALDY] = "objs/baldy";
		objectfilenames[ObjectGraphics::OGT_TERRA] = "objs/terra";
		objectfilenames[ObjectGraphics::OGT_PHANTOM] = "objs/phantom";
		objectfilenames[ObjectGraphics::OGT_GHOUL] = "objs/ghoul";
		objectfilenames[ObjectGraphics::OGT_DRAGON] = "objs/dragon";
		objectfilenames[ObjectGraphics::OGT_LIZARD] = "objs/lizard";
		objectfilenames[ObjectGraphics::OGT_DEMON] = "objs/demon";
		objectfilenames[ObjectGraphics::OGT_DEATHHEAD] = "objs/deathhead";
		objectfilenames[ObjectGraphics::OGT_TROLL] = "objs/troll";


		soundfilenames[SoundHandler::SOUND_SHOOT] = "sfxs/shoot.bin";
		soundfilenames[SoundHandler::SOUND_SHOOT2] = "sfxs/shoot2.bin";
		soundfilenames[SoundHandler::SOUND_SHOOT3] = "sfxs/shoot3.bin";
		soundfilenames[SoundHandler::SOUND_SHOOT4] = "sfxs/shoot4.bin";
		soundfilenames[SoundHandler::SOUND_SHOOT5] = "sfxs/shoot5.bin";
		soundfilenames[SoundHandler::SOUND_GRUNT] = "sfxs/grunt.bin";
		soundfilenames[SoundHandler::SOUND_GRUNT2] = "sfxs/grunt2.bin";
		soundfilenames[SoundHandler::SOUND_GRUNT3] = "sfxs/grunt3.bin";
		soundfilenames[SoundHandler::SOUND_GRUNT4] = "sfxs/grunt4.bin";
		soundfilenames[SoundHandler::SOUND_TOKEN] = "sfxs/token.bin";
		soundfilenames[SoundHandler::SOUND_DOOR] = "sfxs/door.bin";
		soundfilenames[SoundHandler::SOUND_FOOTSTEP] = "sfxs/footstep.bin";
		soundfilenames[SoundHandler::SOUND_DIE] = "sfxs/die.bin";
		soundfilenames[SoundHandler::SOUND_SPLAT] = "sfxs/splat.bin";
		soundfilenames[SoundHandler::SOUND_TELEPORT] = "sfxs/teleport.bin";
		soundfilenames[SoundHandler::SOUND_GHOUL] = "sfxs/ghoul.bin";
		soundfilenames[SoundHandler::SOUND_LIZARD] = "sfxs/lizard.bin";
		soundfilenames[SoundHandler::SOUND_LIZHIT] = "sfxs/lizhit.bin";
		soundfilenames[SoundHandler::SOUND_TROLLMAD] = "sfxs/trollmad.bin";
		soundfilenames[SoundHandler::SOUND_TROLLHIT] = "sfxs/trollhit.bin";
		soundfilenames[SoundHandler::SOUND_ROBOT] = "sfxs/robot.bin";
		soundfilenames[SoundHandler::SOUND_ROBODIE] = "sfxs/robodie.bin";
		soundfilenames[SoundHandler::SOUND_DRAGON] = "sfxs/dragon.bin";
#endif
	}
}