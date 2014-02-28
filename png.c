#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zlib.h>
#include "png.h"

/*
 * Analyze a PNG file.
 * If it is a PNG file, print out all relevant metadata and return 0.
 * If it isn't a PNG file, return -1 and print nothing.
 */
int analyze_png(FILE *f) {
    /* YOU WRITE THIS PART */
	//Check if first 8 bytes match standard png type
	unsigned int i = 0;
	unsigned char png_check[8];
	unsigned char png_format[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
	for (i=0; i<8; i++) {
		/*
		size_t fread ( void * ptr, size_t size, size_t count, FILE * stream );
		Reads an array of count elements, each one with a size of size bytes, 
		from the stream and stores them in the block of memory specified by ptr.

		The position indicator of the stream is advanced by the total amount of 
		bytes read.

		The total amount of bytes read if successful is (size*count).
		*/
		fread(&png_check[i],1,1,f);
	}

	for (i=0; i<8; i++) {
		printf("%x\n", png_check[i]);
		//printf("%d\n", png_check[i] == png_format[i]);
		if (png_check[i] != png_format[i]) {
			//Not a valid PNG file
			return -1;
		}
	}

	//moving on to chunks
	unsigned int length = 0;
	unsigned char chunktype[4];
	fread(&length, 4, 1, f);
	for (i=0; i<4; i++) {
		fread(&chunktype[i],1,1,f);
		printf("%x\n", chunktype[i]);
	}
	printf("\n%d\n", length);
	//printf("%x", (const char *) png_check);
	//char file_name[]

    //printf(file_name)
    return -1;
}
