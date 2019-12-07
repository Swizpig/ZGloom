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

		enum ScriptOp
		{
			SOP_LOADFLAT,
			SOP_LOADMAP,
			SOP_SETPICT,
			SOP_DRAW,
			SOP_TEXT,
			SOP_WAIT,
			SOP_PLAY,
			SOP_NOP
		};

		ScriptOp NextLine(std::string& name);

	private:
		uint32_t line = 0;
};