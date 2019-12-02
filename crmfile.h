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
			FILE *file;
			int filesize;
			uint8_t *indata;
			char header[14];

			/*  open input file  */
			file = fopen(name, "rb");

			if (file == 0)
			{
				//printf("Could not open input file\n");
				return false;
			}

			/*  read header and get file size  */
			fread(header, 14, 1, file);
			fseek(file, 0, SEEK_END);
			filesize = ftell(file);

			/*  check if file is valid  */
			if (filesize > 14)
				size = GetSize(header);
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

			if (indata == 0)
			{
				//printf("Out of memory\n");
				fclose(file);
				return false;
			}

			/*  read file  */
			rewind(file);
			fread(indata, filesize, 1, file);
			fclose(file);

			/*  decrunch!  */
			Decrunch(indata);

			std::copy(indata + 14, indata+14+size, data);
			free(indata);

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