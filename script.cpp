#include "script.h"
#include "crmfile.h"
#include <iostream>

Script::Script()
{
	CrmFile file;

	if (!file.Load("misc/script"))
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
