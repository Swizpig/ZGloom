#pragma once

#include <string>
#include "objectgraphics.h"
#include "soundhandler.h"

namespace Config
{
	enum keyenum
	{
		KEY_UP,
		KEY_DOWN,
		KEY_LEFT,
		KEY_RIGHT,
		KEY_SLEFT,
		KEY_SRIGHT,
		KEY_STRAFEMOD,
		KEY_SHOOT,
		KEY_END
	};

	void SetZM(bool zm);
	std::string GetScriptName();
	std::string GetPicsDir();
	std::string GetLevelDir();
	std::string GetMiscDir();
	std::string GetObjectFilename(ObjectGraphics::ObjectGraphicType i);
	std::string GetGoreFilename(ObjectGraphics::ObjectGraphicType i);
	std::string GetSoundFilename(SoundHandler::Sounds i);
	std::string GetMusicFilename(int i);
	std::string GetMusicDir();
	int GetKey(keyenum k);
	void SetKey(keyenum k, int newval);
	void GetRenderSizes(int &rw, int &rh, int &ww, int& wh);
	int32_t GetFocalLength();
	int GetMouseSens();
	void SetMouseSens(int sens);
	int GetBlood();
	void SetBlood(int b);
	void SetDebug(bool b);
	bool GetDebug();
	void SetFPS(uint32_t f);
	uint32_t GetFPS();
	void SetFullscreen(bool f);
	bool GetFullscreen();

	bool GetMT();
	bool GetVSync();

	void Init();
	void Save();
};
