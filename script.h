#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Script
{
	public:
		Script();
		uint32_t numlines;
		std::vector<std::string> lines;
		void Reset()
		{
			line = 0;
		};

		enum ScriptOp
		{
			SOP_LOADFLAT,
			SOP_LOADMAP,
			SOP_SETPICT,
			SOP_DRAW,
			SOP_TEXT,
			SOP_WAIT,
			SOP_PLAY,
			SOP_SONG,
			SOP_END,
			SOP_NOP
		};

		ScriptOp NextLine(std::string& name);
		void GetLevelNames(std::vector<std::string>& names);

	private:
		uint32_t line = 0;
};