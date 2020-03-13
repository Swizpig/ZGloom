#include "script.h"
#include "crmfile.h"
#include  "config.h"
#include <iostream>

Script::Script()
{
	CrmFile file;

	if (!file.Load(Config::GetScriptName().c_str()))
	{
		std::cout << "Could not load script!";
		return;
	}

	lines.clear();
	numlines = 0;
	std::string tempstring = "";

	for (uint32_t i = 0; i < file.size; i++)
	{
		char c = file.data[i];

		if (c != 0xa)
		{
			tempstring += c;
		}
		else
		{
			lines.push_back(tempstring);
			numlines++;
			tempstring = "";
		}
	}

	if (tempstring.length())
	{
		lines.push_back(tempstring);
		numlines++;
	}
}

Script::ScriptOp Script::NextLine(std::string& name)
{
	if (line == lines.size())
	{
		line = 0;
		return SOP_END;
	}

	while ((lines[line].length() == 0) ||(lines[line][0] == ';'))
	{
		line++;
		if (line == lines.size())
		{
			line = 0;
			return SOP_END;
		}
	}

	if (lines[line].substr(0, 5) == "pict_")
	{
		name = lines[line].substr(5, std::string::npos);
		line++;
		return SOP_SETPICT;
	}
	if (lines[line].substr(0, 5) == "tile_")
	{
		name = lines[line].substr(5, std::string::npos);
		line++;
		return SOP_LOADFLAT;
	}
	if (lines[line].substr(0, 5) == "play_")
	{
		name = lines[line].substr(5, std::string::npos);
		line++;
		return SOP_PLAY;
	}
	if (lines[line].substr(0, 5) == "draw_")
	{
		line++;
		return SOP_DRAW;
	}
	if (lines[line].substr(0, 5) == "wait_")
	{
		line++;
		return SOP_WAIT;
	}
	if (lines[line].substr(0, 5) == "text_")
	{
		name = lines[line].substr(5, std::string::npos);
		line++;
		return SOP_TEXT;
	}
	if (lines[line].substr(0, 5) == "song_")
	{
		name = lines[line].substr(5, std::string::npos);
		line++;
		return SOP_SONG;
	}

	line++;
	if (line == lines.size())
	{
		line = 0;
		return SOP_END;
	}
	return SOP_NOP;
}

void Script::GetLevelNames(std::vector<std::string>& names)
{
	for (auto& s : lines)
	{
		if (s.length() > 5)
		{
			if (s.substr(0, 5) == "text_")
			{
				names.push_back(s.substr(5, std::string::npos));
			}
		}
	}
}
