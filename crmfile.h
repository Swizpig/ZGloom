#pragma once 

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <algorithm>

#include "decrunchmania.h"

class CrmFile
{
	public:
		uint32_t size;
		uint8_t* data;

		bool Load(const char* name)
		{
			if (data) free(data);

			FILE *file;
			int filesize;
			int headroom;
			uint8_t *indata;
			uint8_t *outdata;
			char header[14];

			/*  open input file  */
			file = fopen(name, "rb");

			if (file == 0)
			{
				printf("Could not open input file %s\n", name);
				return false;
			}

			/*  read header and get file size  */
			fread(header, 14, 1, file);
			fseek(file, 0, SEEK_END);
			filesize = ftell(file);

			/*  check if file is valid  */
			if (filesize > 14)
			{
				headroom = GetSecDist(header);
				size = GetSize(header);
			}
			if (filesize <= 14 || size == 0)
			{
				//printf("Not a valid CrM2 file\n");
				// assume unpacked
				fseek(file, 0, SEEK_SET);
				data = static_cast<uint8_t*>(malloc(filesize));
				fread(data, filesize, 1, file);
				size = filesize;
				fclose(file);
				return true;
			}

			/*  allocate memory to hold header and decrunched data  */
			indata = static_cast<uint8_t*>(malloc(size + 14));
			data = static_cast<uint8_t*>(malloc(size));
			outdata = static_cast<uint8_t*>(malloc(size + headroom));

			if (indata == 0)
			{
				//printf("Out of memory\n");
				fclose(file);
				return false;
			}

			/*  read file  */
			rewind(file);
			fread(indata, 1, filesize, file);
			fclose(file);

			/*  decrunch!  */

			// in place decrunch is broken for some files with minsecdist.
			//OverlapCheck(indata);
			Decrunch(indata, outdata);

			std::copy(outdata, outdata+size, data);
			free(indata);
			free(outdata);

			return true;
		}

		CrmFile()
		{
			size = 0;
			data = nullptr;
		}

		~CrmFile()
		{
			if (data) free(data);
		}
};