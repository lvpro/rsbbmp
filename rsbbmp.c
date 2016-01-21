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

   TO-DO: Rework getBMPheader to differentiate between
		  different version headers
*/

#include <windows.h>    /* For MessageBox() Error Messages */
#include <stdio.h>
#include "rsbbmp.h"

int bmp_to_rsb(FILE *bmp, FILE *rsb)
{
	BMPHEADER bmpHeader;
	RSBHEADER rsbHeader;
	RSBBYTE *bmpPixel;
	RSBBYTE *dummyBuffer;
	RSBWORD *rsbPixel;
	RSBDWORD redbits = 5;
	RSBDWORD greenbits = 6;
	RSBDWORD bluebits = 5;
	RSBDWORD RowLength;
	RSBWORD tempWord, tempWord2;
	int pixels, padding;
	int rsbWord;    /* RSBPIXEL position */
        int i,j,k;      /* loop counters */

	/* Get BMP Header, Check For validity */
	if (getBMPheader(&bmpHeader, bmp) == ERROR)
		return ERROR;

	/* Otherwise we have a valid BMP Header */
	
	pixels = bmpHeader.width * bmpHeader.height;
	
	RowLength = ((bmpHeader.width * 24 + 31) & ~31) >> 3;
	padding = RowLength - (bmpHeader.width * 3);
	dummyBuffer = (RSBBYTE *) malloc(padding);

	/* Allocate space for BMP bytes */
        bmpPixel = (RSBBYTE *) malloc(pixels * 3);    /* 3 Bytes / Pixel */
        rsbPixel = (RSBWORD *) malloc(pixels * sizeof(RSBWORD)) ;

	if ((bmpPixel == NULL) || (rsbPixel == NULL))
	{
		MessageBox(NULL, "Memory Allocation Error!", "RSB <-> BMP", MB_ICONWARNING | MB_OK);
		return ERROR;
	}
	
	/* Seek To DIB Pixels */
	fseek(bmp, bmpHeader.pixelsOffset, SEEK_SET);
	
	/* Read In BMP Pixels */

	for(i=0; i<bmpHeader.height; i++)
	{
		j = i * bmpHeader.width * 3;
		k = j + (bmpHeader.width * 3);
		for(;j<k;j++)
                        fread(&bmpPixel[j], sizeof(RSBBYTE), 1, bmp);
		for(j=0;j<padding;j++)
                        fread(dummyBuffer, sizeof(RSBBYTE), 1, bmp);
	}
	
	for(i=0, rsbWord=0; i<k; i=i+3, rsbWord=rsbWord+1)
	{
		tempWord = tempWord2 = 0;
		tempWord = bmpPixel[i+2] >> (8-redbits);
		tempWord = tempWord << (16-redbits);
		tempWord2 = bmpPixel[i+1] >> (8-greenbits);
		tempWord2 = tempWord2 << (16-(redbits+greenbits));
		tempWord = tempWord | tempWord2;
		tempWord2 = bmpPixel[i] >> (8-bluebits);
		rsbPixel[rsbWord] = tempWord | tempWord2;
	}
	
	rsbHeader.version = 1;
	rsbHeader.width = bmpHeader.width;
	rsbHeader.height = bmpHeader.height;
	rsbHeader.redbits = redbits;
	rsbHeader.greenbits = greenbits;
	rsbHeader.bluebits = bluebits;
	rsbHeader.alphabits = 0;

	writeRSBheader(&rsbHeader, rsb);

	/* Write RSB Pixels */
        for(i=(rsbHeader.width * (rsbHeader.height - 1)); i>=0; i=i-rsbHeader.width)
        {
                j = i;
                k = i + (rsbHeader.width - 1);
                for(;j<=k;j++)
                        fwrite(&rsbPixel[j], sizeof(RSBWORD),1,rsb);
        }

    free(bmpPixel);
	free(rsbPixel);
	free(dummyBuffer);
	
	return 1;
}

/* RSB->BMP Conversion Function
   Accepts: Input File Pointer, Output File Pointer
   Returns: 1 on success, ERROR on failure
*/
int rsb_to_bmp(FILE *rsb, FILE *bmp)
{
	RSBHEADER rsbHeader;
	BMPHEADER bmpHeader;
	RSBWORD *rsbPixel;
	RSBBYTE *bmpPixel;
	RSBBYTE pad = 0;
	RSBDWORD RowLength;
	int pixels, padding, rsbWord, bmpByte, i, j, k;

	if (getRSBheader(&rsbHeader, rsb) == ERROR)
		return ERROR;

	pixels = rsbHeader.width * rsbHeader.height;    
	RowLength = ((rsbHeader.width * 24 + 31) & ~31) >> 3;
	padding = RowLength - (rsbHeader.width * 3);

	rsbPixel = (RSBWORD *) malloc(pixels * sizeof(RSBWORD));
	bmpPixel = (RSBBYTE *) malloc(pixels * 3);

	/* Read 16-bit RSB Pixels Into Allocated Memory. 
	   Working On Data In Memory Will Be Faster Than
	   Doing It Byte-For-Byte From File I/O */

	for(i=0; i<pixels; i++)
	{
		if (fread(&rsbPixel[i], sizeof(RSBWORD), 1, rsb) == 0)
		{
			MessageBox(NULL,"Error Reading Pixel Data In Current File!", "RSB <->BMP", MB_OK | MB_ICONWARNING);
			free(rsbPixel); /* Free Allocated Memory */
			free(bmpPixel);
			return ERROR;
		}
	}

	/* At This Point RSB Data Was Successfully Read */
	/* Use somewhat byte-reversal to account for OS/2
	   PM coordinate system silliness */
	for(i=(rsbHeader.height-1), bmpByte=0; i>=0; i--)
	{
		/* For Speed, All Calculations Are Made With Bit Shifts */
		/* Note: ANSI states an unsigned integer is converted to
		   a shorter unsigned or signed integer by truncating the
		   high-order bits in assignment conversions */

		/* i = row
		   j = column max (calculated from row)
		   rsbWord = current RSB pixel      */

		rsbWord = i*rsbHeader.width;
		j = rsbWord + rsbHeader.width;
		
		/* Write Columns */
		for (; rsbWord<j; rsbWord++)
		{               
			/* Set Blue Byte Of Pixel */
			bmpPixel[bmpByte] = (rsbPixel[rsbWord] << (8 - rsbHeader.bluebits));

			/* Set Green Byte Of Pixel */
			bmpPixel[bmpByte+1] = ((rsbPixel[rsbWord] >> rsbHeader.bluebits) << (8-rsbHeader.greenbits));

			/* Set Red Byte Of Pixel */
			bmpPixel[bmpByte+2] = ((rsbPixel[rsbWord] >> (16 - rsbHeader.redbits)) << (8-rsbHeader.redbits));
			
			bmpByte = bmpByte + 3;  /* Advance BMP Byte Pointer */
		}
	}

	/* At This Point All Data Conversion Is Done, Write BMP */
	
	/* Set Header Values */
	bmpHeader.signature = 0x4D42;
	bmpHeader.size = (pixels * 3) + 54;
	bmpHeader.reserved1 = 0;
	bmpHeader.reserved2 = 0;
	bmpHeader.pixelsOffset = 54;
	bmpHeader.infoStructSize = 40;
	bmpHeader.width = rsbHeader.width;
	bmpHeader.height = rsbHeader.height;
	bmpHeader.planes = 1;
	bmpHeader.bitDepth = 24;
	bmpHeader.compression = 0;
	bmpHeader.imageSize = pixels * 3;
	bmpHeader.hResolution = bmpHeader.vResolution = 0;
	bmpHeader.colorsUsed = bmpHeader.colorsImportant = 0;
	
	writeBMPheader(&bmpHeader, bmp);        /* Write BMP Info Header */

	/* Write Pixels */

	pixels = pixels*3;     /* Adjust Counter In Bytes */

	for(i=0; i<bmpHeader.height; i++)
	{
		j = i * bmpHeader.width * 3;
		k = j + (bmpHeader.width * 3);
		for(; j<k; j++)
			fwrite(&bmpPixel[j], sizeof(RSBBYTE), 1, bmp);
		for(j=0; j<padding; j++)
			fwrite(&pad, sizeof(RSBBYTE), 1, bmp);
	}
	
	free(rsbPixel);         /* Free Allocated Memory */
	free(bmpPixel);

	return 1;
}

int getRSBheader(RSBHEADER *rsbHeader, FILE *rsb)
{
	if (fread(&rsbHeader->version, sizeof(RSBDWORD), 1, rsb) != 0)
	if (fread(&rsbHeader->width, sizeof(RSBDWORD), 1, rsb) != 0)
	if (fread(&rsbHeader->height, sizeof(RSBDWORD), 1, rsb) != 0)
	if (fread(&rsbHeader->redbits, sizeof(RSBDWORD), 1, rsb) != 0)
	if (fread(&rsbHeader->greenbits, sizeof(RSBDWORD), 1, rsb) != 0)
	if (fread(&rsbHeader->bluebits, sizeof(RSBDWORD), 1, rsb) != 0)
	if (fread(&rsbHeader->alphabits, sizeof(RSBDWORD), 1, rsb) != 0)
	{
		/* Check Version */
		if (rsbHeader->version != 1)
		{
			MessageBox(NULL, "RSB Version Not Supported!", "Current File...", MB_ICONWARNING | MB_OK);
			return ERROR;
		}

		/* Check Alpha Bits For Zero */
		if (rsbHeader->alphabits != 0)
		{
			MessageBox(NULL,"Alpha Blended Textures Not Supported!", "Current File...", MB_ICONWARNING | MB_OK);
			return ERROR;
		}
		
		return 1;  /* RSB info header read successfully, and valid */
	}

	MessageBox(NULL,"Error Reading RSB Input!", "Current File...", MB_ICONWARNING | MB_OK);
	return ERROR;
}

void writeBMPheader(BMPHEADER *bmpHeader, FILE *bmp)
{
	fwrite(&bmpHeader->signature, sizeof(RSBWORD), 1, bmp);
	fwrite(&bmpHeader->size, sizeof(RSBDWORD), 1, bmp);
	fwrite(&bmpHeader->reserved1, sizeof(RSBWORD), 1, bmp);
	fwrite(&bmpHeader->reserved2, sizeof(RSBWORD), 1, bmp);
	fwrite(&bmpHeader->pixelsOffset, sizeof(RSBDWORD), 1, bmp);
	fwrite(&bmpHeader->infoStructSize, sizeof(RSBDWORD), 1, bmp);
	fwrite(&bmpHeader->width, sizeof(RSBLONG), 1, bmp);
	fwrite(&bmpHeader->height, sizeof(RSBLONG), 1, bmp);
	fwrite(&bmpHeader->planes, sizeof(RSBWORD), 1, bmp);
	fwrite(&bmpHeader->bitDepth, sizeof(RSBWORD), 1, bmp);
	fwrite(&bmpHeader->compression, sizeof(RSBDWORD), 1, bmp);
	fwrite(&bmpHeader->imageSize, sizeof(RSBDWORD), 1, bmp);
	fwrite(&bmpHeader->hResolution, sizeof(RSBLONG), 1, bmp);
	fwrite(&bmpHeader->vResolution, sizeof(RSBLONG), 1, bmp);
	fwrite(&bmpHeader->colorsUsed, sizeof(RSBDWORD), 1, bmp);
	fwrite(&bmpHeader->colorsImportant, sizeof(RSBDWORD), 1, bmp);
}

int getBMPheader(BMPHEADER *bmpHeader, FILE *bmp)
{
	/* So So Error Checking. I'm sure if the user tried hard 
	   enough, he/she could trick the code into proceeding. 
	   Junk would output.  Oh well. */
	
	if (fread(&bmpHeader->signature, sizeof(RSBWORD), 1, bmp) != 0)
	if (fread(&bmpHeader->size, sizeof(RSBDWORD), 1, bmp) != 0)
	if (fread(&bmpHeader->reserved1, sizeof(RSBWORD), 1, bmp) != 0)
	if (fread(&bmpHeader->reserved2, sizeof(RSBWORD), 1, bmp) != 0)
	if (fread(&bmpHeader->pixelsOffset, sizeof(RSBDWORD), 1, bmp) != 0)
	if (fread(&bmpHeader->infoStructSize, sizeof(RSBDWORD), 1, bmp) != 0)
	if (fread(&bmpHeader->width, sizeof(RSBLONG), 1, bmp) != 0)
	if (fread(&bmpHeader->height, sizeof(RSBLONG), 1, bmp) != 0)
	if (fread(&bmpHeader->planes, sizeof(RSBWORD), 1, bmp) != 0)
	if (fread(&bmpHeader->bitDepth, sizeof(RSBWORD), 1, bmp) != 0)
	if (fread(&bmpHeader->compression, sizeof(RSBDWORD), 1, bmp) != 0)
	if (fread(&bmpHeader->imageSize, sizeof(RSBDWORD), 1, bmp) != 0)
	if (fread(&bmpHeader->hResolution, sizeof(RSBLONG), 1, bmp) != 0)
	if (fread(&bmpHeader->vResolution, sizeof(RSBLONG), 1, bmp) != 0)
	if (fread(&bmpHeader->colorsUsed, sizeof(RSBDWORD), 1, bmp) != 0)
	if (fread(&bmpHeader->colorsImportant, sizeof(RSBDWORD), 1, bmp) != 0)
	{
		/* Check For A Valid Bitmap */
		/* BitMap Signature Present? */
		if (bmpHeader->signature != 0x4D42)
		{
			MessageBox(NULL,"Bitmap Signature Not Present!", "Current File...", MB_ICONWARNING | MB_OK);
			return ERROR;
		}
		/* 24-bit Bitmap? */
		if (bmpHeader->bitDepth != 24)
		{
			MessageBox(NULL,"Only 24-bit Bitmaps Are Supported At This Time!", "Current File...", MB_ICONWARNING | MB_OK);
			return ERROR;
		}
		
		if (bmpHeader->compression != 0)
		{
			MessageBox(NULL,"Compression Not Supported!", "Current File...", MB_ICONWARNING | MB_OK);
			return ERROR;
		}
		
		return 1;       /* Successful! */
	}

	MessageBox(NULL,"Error Reading BMP Input!", "Current File...", MB_ICONWARNING | MB_OK);
	return ERROR;
}

void writeRSBheader(RSBHEADER *rsbHeader, FILE *rsb)
{
	fwrite(&rsbHeader->version, sizeof(RSBDWORD), 1, rsb);
	fwrite(&rsbHeader->width, sizeof(RSBDWORD), 1, rsb);
	fwrite(&rsbHeader->height, sizeof(RSBDWORD), 1, rsb);
	fwrite(&rsbHeader->redbits, sizeof(RSBDWORD), 1, rsb);
	fwrite(&rsbHeader->greenbits, sizeof(RSBDWORD), 1, rsb);
	fwrite(&rsbHeader->bluebits, sizeof(RSBDWORD), 1, rsb);
	fwrite(&rsbHeader->alphabits, sizeof(RSBDWORD), 1, rsb);
}       
