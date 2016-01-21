/* 
   RSB/BMP Conversion Code (C)2000-2002 Clint Kennedy
   02/12/2000 All Rights Reserved.
  
   PLEASE submit ANY useful changes to this code to the author as
   he will include your modifications in the primary distribution
   of RSB<->BMP.  This includes support for other RSB versions!

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   Distributed Exclusively By 3dretreat.com

   Any usage / modifications of this source code must include
   a reference to the original author.

   Code tested successfully with:
   MS Visual C++ 6.0 
   GCC/Mingw32-2.95.2

   For all GCC-based compilers, use
   -03            (Level 3 optimizations)
   -ffast-math    (fast math routines)
   -funroll-loops (unroll loops for speed)
   -mcpu=i586     (Pentium Instruction Scheduling, x86 compatible)

   Should work fine with any C compiler, on any platform.
   This code has been tested to be bug-free, as far as I can tell.

   Type Definitions & Function Prototypes
*/

/* Simple Data-Type Definitions */
typedef unsigned char RSBBYTE;             /*  8-bits */
typedef unsigned short int RSBWORD;        /* 16-bits */
typedef unsigned int RSBDWORD;             /* 32-bits */
typedef signed int RSBLONG;                /* 32-bits */

#define ERROR 0 

/* RS Bitmap Header Definition */
typedef struct {
        RSBDWORD version;                  /* currently version 1 */
        RSBDWORD width;                    /* width in pixels */
        RSBDWORD height;                   /* height in pixels */
        RSBDWORD redbits;                  /* # of R bits in 1 pixel */
        RSBDWORD greenbits;                /* # of G bits in 1 pixel */
        RSBDWORD bluebits;                 /* # of B bits in 1 pixel */
        RSBDWORD alphabits;                /* used in alpha blended 
										   textures, unused here  */
} RSBHEADER;

/* Windows / OS/2 Style Bitmap Header Definition
   (Bitmap File Header + Expanded Windows DIB Info Header)
   
   Newer implementations are backwards compatible with this
   information definition. The first 32 bits of any version
   header is guaranteed to be the size of the structure.
*/
typedef struct {
        RSBWORD signature;                        /* always 0x4D42 */
        RSBDWORD size;                            /* entire file size */
        RSBWORD reserved1;                        /* must be zero */
        RSBWORD reserved2;                        /* must be zero */
        RSBDWORD pixelsOffset;                    /* offset of DIB pixel bits */
        RSBDWORD infoStructSize;                  /* remaining size of info */
        RSBLONG width;                            /* width in pixels */
        RSBLONG height;                           /* height in pixels */
        RSBWORD planes;                           /* = 1 */
        RSBWORD bitDepth;                         /* must be 24 */
        RSBDWORD compression;                     /* must be BI_RGB */
        RSBDWORD imageSize;                       /* # of bytes in image */
        RSBLONG hResolution;                      /* horizontal resolution */
        RSBLONG vResolution;                      /* vertical resolution */
        RSBDWORD colorsUsed;                      /* # of colors used */
        RSBDWORD colorsImportant;                 /* # of important colors */

} BMPHEADER;

/* Function Prototypes */
int bmp_to_rsb(FILE *bmp, FILE *rsb);          /* 24bit BMP -> RS BMP */
int rsb_to_bmp(FILE *rsb, FILE *bmp);          /* RS BMP -> 24bit BMP */
int getRSBheader(RSBHEADER *rsbHeader, FILE *rsb); /* Get RSB info */
void writeBMPheader(BMPHEADER *bmpHeader, FILE *bmp); /* BMP Head */
int getBMPheader(BMPHEADER *bmpHeader, FILE *bmp); /* Get BMP info */
void writeRSBheader(RSBHEADER *rsbHeader, FILE *rsb); /* RSB Head */
