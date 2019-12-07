
#include "decrunchmania.h"


static void writew(void *data, unsigned short w);
static void writel(void *data, unsigned int l);
static unsigned short readw(void *data);
static unsigned int readl(void *data);
static int getbits();
static void readtab();
static void readit();
static void calccmptab();

static unsigned char *a0, *a1, *a2, *a3, *a4, *a5, *a6;
static unsigned short d0, d1, d2, d3, d4, d5, d6, d7;
static unsigned int d0l, d1l, d2l, d3l, d4l, d5l, d6l, d7l;

#define ocmptab     0
#define oaddtab     64
#define orealtab    128
#define oanzperbits 1182
#define obuflen     1248



void OverlapCheck(void *in)
{
	a0 = (unsigned char*)in;
	a1 = (unsigned char*)in + 14;

	a0 += 4;//skip "CRM2"

	d0l = d0 = 0;

	d0l = d0 = readw(a0); a0 += 2; //MinSecDist
	d1l = d1 = readl(a0); a0 += 4; //DestLen
	d2l = d2 = readl(a0); a0 += 4; //SrcLen

	a2 = a0 + d0l;
	if (a2 <= a1) goto NoCopy;

	a2 = a0;
	a0 = a1;

	a0 -= d0l;

	a3 = a0;
	d7 = d2;
	d7l = d2;
	
	d7 >>= 2;
	d7l >>= 2;

CopyLoop:
	*a3 = *a2;
	a2 += 4;
	a3 += 4;

	d7--;
	d7l--;

	if (d7l > 0) goto CopyLoop;

	*a3 = *a2;
	a2 += 4;
	a3 += 4; //in case of ...
NoCopy:
	a2 = a0;
	Decrunch(in, nullptr);
}


/*  decrunches data in place. must point to enough space to hold the
    decrunched data + the 14 byte header  */

void* Decrunch(void *in, void* out)
{
    unsigned char tabbs[1248] = {0};

    /*  Literal translation of the 68K assembly LZ-Huffman decruncher  */

	a0 = (unsigned char*)in + 6;
	a1 = (unsigned char*)out;

	d1 = d1l = readl(a0); a0 += 4;
	d2 = d2l = readl(a0); a0 += 4;
	a2 = a0;

    a6 = tabbs+2;

    a1 += d1l;
    a2 += d2l;
    
    d0 = readw(a2 -= 2);
    d6 = d6l = readl(a2 -= 4);
    d7 = d7l = 16;
    d7 -= d0;
    d6 = d6l >>= d7;
    d7 = d0;
    d7--;
    a5 = (unsigned char*)17;

bufloop:
    a0 = oanzperbits+a6;
    d2 = d2l = 16-1;

clear:
    writel(a0, 0); a0 += 4;
    d2--;
    if(d2 != 0xffff)
        goto clear;

    a0 = 32+oanzperbits+a6;
    a4 = orealtab+30+a6;
    d2 = d2l = 9;
    readtab();
    a0 = oanzperbits+a6;
    a4 = orealtab+a6;
    d2 = d2l = 4;
    readtab();

    a3 = oanzperbits+32+a6;
    a4 = ocmptab-2+a6;
    calccmptab();
    a3 = oanzperbits+a6;
    a4 = ocmptab+30+a6;
    calccmptab();

    d1 = d1l = 16;
    getbits();
    d5 = d0;
    a0 = ocmptab+32+a6;

decrloop:   
    a4 = a6;
    readit();
    d4 = readw(orealtab-2+a0+(int)d1);
    if(d4 < 0x8000)
        goto sequence;

    *(--a1) = d4;
    d5--;
    if(d5 != 0xffff)
        goto decrloop;

checknextbufloop:
    d1 = d1l = 1;
    if(getbits() != 0)
        goto bufloop;
    return a1;

sequence:
    a4 = a0;
    readit();
    d1 = readw(orealtab-32+a0+(int)d1);
    if(d1 == 0)
        goto sc3;
    d2 = d1;
    getbits();
    d0l = (d0l&0xffff0000) | d0; d0 = d0l |= (1<<d2);

sc2:
    a3 = a1+(int)d0;
    *(--a1) = *a3;

sloop:
    *(--a1) = *(--a3);
    d4--;
    if(d4 != 0xffff)
        goto sloop;
    *(--a1) = *(--a3);
    d5--;
    if(d5 != 0xffff)
        goto decrloop;
    goto checknextbufloop;

sc3:
    d1 = d1l = 1;
    getbits();
    goto sc2;
}




/*  returns size of decrunched data or 0 if it was not a valid file  */
unsigned int GetSize(void *data)
{
    if(data == 0)
        return 0;

    return readl(data) == 'CrM2' ? readl((unsigned char*)data+6) : 0;
}

/*  returns MinSecDist, headroom needed for some files */
unsigned int GetSecDist(void *data)
{
	if (data == 0)
		return 0;

	return readl(data) == 'CrM2' ? readw((unsigned char*)data + 4) : 0;
}


static void writew(void *data, unsigned short w)
{
	unsigned char *p = (unsigned char *)data;

    p[0] = w>>8;
    p[1] = w;
}

static void writel(void *data, unsigned int l)
{
	unsigned char *p = (unsigned char *)data;

    p[0] = l>>24;
    p[1] = l>>16;
    p[2] = l>>8;
    p[3] = l;
}

static unsigned short readw(void *data)
{
	unsigned char *p = (unsigned char *)data;

    return (unsigned short)p[0]<<8 | p[1];
}

static unsigned int readl(void *data)
{
	unsigned char *p = (unsigned char *)data;

    return (unsigned int)p[0]<<24 | p[1]<<16 | p[2]<<8 | p[3];
}




static int getbits()
{
    static const unsigned short anddata[] =
    {   1, 3, 7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff, 0x3ff,
        0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff};

    short d7t;
    

    d0 = d6;
    d6l = (d6l&0xffff0000) | d6; d6 = d6l >>= d1;
    d7t = d7;
    d7 -= d1;
    if(d7t >= (short)d1)
        goto gbnoloop;
    d7 += (unsigned short)a5;
    d3 = d3l = 0;
    d3 = readw(a2 -= 2);
    d3l = (d3l&0xffff0000) | d3; d3 = d3l <<= d7;
    d6l = (d6l&0xffff0000) | d6; d6 = d6l |= d3l;
    d7--;
    
gbnoloop:
    d1 += d1;
    d0 &= anddata[(d1/2)-1];

    return d0;
}




static void readtab()
{
    unsigned int x, d1t, d2t, d3t, d4t, d5t;
    unsigned char *a3t = a3;

    d1t = d1l = (d1l&0xffff0000) | d1;
    d2t = d2l = (d2l&0xffff0000) | d2;
    d3t = d3l = (d3l&0xffff0000) | d3;
    d4t = d4l = (d4l&0xffff0000) | d4;
    d5t = d5l = (d5l&0xffff0000) | d5;

    d1 = d1l = 4;
    getbits();
    d5 = d0;
    d5--;
    d4 = d4l = 0;
    a3 = 0;

rtlop:
    d4++;
    d1 = d4;
    if((signed short)d1 > (signed short)d2)
        d1 = d2;
    getbits();
    writew(a0, d0); a0 += 2;
    a3 += (int)d0;
    d5--;
    if(d5 != 0xffff)
        goto rtlop;
    
    d5 = (unsigned short)a3;
    d5--;

rtlp2:
    d1 = d2;
    getbits();
    x = d0&1; d0 >>= 1;
    d0 &= 0xff;
    d0 |= d0 & 0x80 ? 0xff00 : 0;
    d0 += d0+x;
    writew(a4, d0); a4 += 2;
    d5--;
    if(d5 != 0xffff)
        goto rtlp2;

    d1 = d1l = d1t;
    d2 = d2l = d2t;
    d3 = d3l = d3t;
    d4 = d4l = d4t;
    d5 = d5l = d5t;
    a3 = a3t;
}




static void calccmptab()
{
    unsigned int d0t, d1t, d2t, d3t, d4t, d5t, d6t, d7t;
    
    d0t = d0l = (d0l&0xffff0000) | d0;
    d1t = d1l = (d1l&0xffff0000) | d1;
    d2t = d2l = (d2l&0xffff0000) | d2;
    d3t = d3l = (d3l&0xffff0000) | d3;
    d4t = d4l = (d4l&0xffff0000) | d4;
    d5t = d5l = (d5l&0xffff0000) | d5;
    d6t = d6l = (d6l&0xffff0000) | d6;
    d7t = d7l = (d7l&0xffff0000) | d7;

    writew(a4, 0); a4 += 2;
    d7 = d7l = 15-1;
    d4 = d4l = 0xffffffff;
    d2 = d3 = d2l = d3l = 0;
    
cclop:
    d6 = readw(a3); a3 += 2;
    writew(64+a4, d3);
    d0 = readw(-2+a4);
    d0 += d0;
    writew(64+a4, readw(64+a4)-d0);
    d3 += d6;
    d2 += d6;
    writew(a4, d2); a4 += 2;
    d2 += d2;
    d7--;
    if(d7 != 0xffff)
        goto cclop;

    d0 = d0l = d0t;
    d1 = d1l = d1t;
    d2 = d2l = d2t;
    d3 = d3l = d3t;
    d4 = d4l = d4t;
    d5 = d5l = d5t;
    d6 = d6l = d6t;
    d7 = d7l = d7t;
}




static void readit()
{
    int x;

    d1 = d1l = 0;
    d7--;
    if(d7 != 0xffff)
        goto rionebit;

rinextword:
    d7 = d7l = 15;
    x = d6&1;
    d6l = (d6l&0xffff0000) | d6; d6 = d6l >>= 1;
    d1 += d1 + x;
    d6l = (d6l&0xffff) | (unsigned int)readw(a2 -= 2)<<16;
    a4 += 2; if(d1 < readw(a4-2))
        goto rifini;
    d7 = d7l = 14;

rionebit:
    x = d6&1;
    d6l = (d6l&0xffff0000) | d6; d6 = d6l >>= 1;
    d1 += d1+x;
    a4 += 2; if(d1 < readw(a4-2))
        goto rifini;
    d7--;
    if(d7 != 0xffff)
        goto rionebit;
    goto rinextword;

rifini:
    d1 += readw(62+a4);
    d1 += d1;
}
