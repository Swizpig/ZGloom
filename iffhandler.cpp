#include <cstdint>
#include <vector>

/// handles a 'trimmed', ie. most of the headers gone, IFF file

static uint16_t Get16(const uint8_t* p)
{
	return (static_cast<uint16_t>(p[0])) << 8 | static_cast<uint16_t>(p[1]);
}

static uint32_t Get32(const uint8_t* p)
{
	return (static_cast<uint16_t>(p[0])) << 24 | (static_cast<uint16_t>(p[1]) << 16) | (static_cast<uint16_t>(p[2])) << 8 | (static_cast<uint16_t>(p[3]) << 0);
}

namespace IffHandler
{
	void DecodeIff(uint8_t* indata, std::vector<uint8_t>& outdata, uint32_t& outwidth)
	{
		uint16_t width  = Get16(indata + 0);
		uint16_t height = Get16(indata + 2);
		uint16_t depth  = Get16(indata + 4);

		uint16_t widthinbytes = (width + 7)/8;

		uint8_t* p = indata + 6 + 6;

		std::vector<uint8_t> row;
		row.resize(widthinbytes);
		outdata.resize(width*height);

		// assuming always byterun1 compressed?

		for (auto y = 0; y < height; y++)
		{
			for (auto d = 0; d < depth; d++)
			{
				uint16_t xpos = 0;

				while (xpos < widthinbytes)
				{
					uint8_t control = *p++;
					
					if (control > 128)
					{
						//run data
						uint16_t repeat = 257 - static_cast<uint16_t>(control);
						uint8_t data = *p++;

						if ((xpos + repeat) > widthinbytes)
						{
							repeat = widthinbytes - xpos;
						}

						std::fill(row.begin() + xpos, row.begin() + xpos + repeat, data);

						xpos += repeat;
					}
					else if (control == 128)
					{
						break;
					}
					else
					{
						// raw copy
						uint8_t datatocopy = control + 1;

						std::copy(p, p + datatocopy, row.begin() + xpos);

						xpos += datatocopy;
						p += datatocopy;
					}
				}

				//planar to chunky. How times change.

				for (auto x = 0; x < width; x++)
				{
					if (d == 0)
					{
						outdata[x + y*width] = (row[x / 8] >> (7 - (x % 8))) & 1;
					}
					else
					{
						outdata[x + y*width] |= ((row[x / 8] >> (7 - (x % 8))) & 1) << d;
					}
				}
			}
		}

		outwidth = width;
	}

}
