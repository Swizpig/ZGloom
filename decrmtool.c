
/*
    decrmtool.c

    Robert Leffmann 2-Nov-2009
    robert.leffmann@gmail.com
*/


#include <stdio.h>
#include <stdlib.h>

#include "decrunchmania.h"


int main(int nargs, char **argp)
{
    FILE *file;
    int filesize, datasize;
    char *indata;
    char header[14];

    /*  check for correct usage  */
    if(nargs != 2  &&  nargs != 3)
    {
        printf("Usage: decrunch <input> [output]\n");
        return 0;
    }
    
    /*  open input file  */
    file = fopen(argp[1], "rb");
    if(file == 0)
    {
        printf("Could not open input file\n");
        return 0;
    }

    /*  read header and get file size  */
    fread(header, 14, 1, file);
    fseek(file, 0, SEEK_END);
    filesize = ftell(file);

    /*  check if file is valid  */
    if(filesize > 14)
        datasize = GetSize(header);
    if(filesize <= 14  ||  datasize == 0)
    {
        printf("Not a valid CrM2 file\n");
        fclose(file);
        return 0;
    }

    /*  allocate memory to hold header and decrunched data  */
    indata = malloc(datasize+14);
    if(indata == 0)
    {
        printf("Out of memory\n");
        fclose(file);
        return 0;
    }

    /*  read file  */
    rewind(file);
    fread(indata, filesize, 1, file);
    fclose(file);

    /*  decrunch!  */
    Decrunch(indata);

    /*  open output file  */
    file = fopen(argp[nargs == 2 ? 1 : 2], "wb");
    if(file == 0)
    {
        printf("Could not create output file\n");
        free(indata);
        return 0;
    }

    /*  save decrunched data and clean up  */
    fwrite(indata+14, datasize, 1, file);
    fclose(file);
    free(indata);

    return 0;
}
