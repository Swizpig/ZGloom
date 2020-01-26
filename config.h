#pragma once

#include <string>
#include "objectgraphics.h"
#include "soundhandler.h"

namespace Config
{
	std::string GetScriptName();
	std::string GetPicsDir();
	std::string GetLevelDir();
	std::string GetMiscDir();
	std::string GetObjectFilename(ObjectGraphics::ObjectGraphicType i);
	std::string GetGoreFilename(ObjectGraphics::ObjectGraphicType i);
	std::string GetSoundFilename(SoundHandler::Sounds i);
	std::string GetMusicFilename(int i);
	void Init();
};
