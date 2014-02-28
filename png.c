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
	const unsigned char png_format[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
	const unsigned char text_format[4] = {0x74, 0x45, 0x58, 0x74};
	const unsigned char ztxt_format[4] = {0x7A, 0x54, 0x58, 0x74};
	const unsigned char time_format[4] = {0x74, 0x49, 0x4D, 0x45};
	const unsigned char end_format[4] = {0x49, 0x45, 0x4e, 0x44};

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
	//The following code below should be in a loop, check chunks 1-by-1 until done

	unsigned int length = 0;
	unsigned char chunktype[4];
	fread(&length, 4, 1, f);
	for (i=0; i<4; i++) {
		fread(&chunktype[i],1,1,f);
		printf("%c\n", chunktype[i]);
	}
	printf("\n%d\n", length);

	if (checkChunk(chunktype, text_format) == true) { // DO tEXt stuff here
		printf("%s", "tEXt chunktype\n");

	} else if (checkChunk(chunktype, ztxt_format) == true) { //DO zTXt stuff here
		printf("%s", "zTXt chunktype\n");

	} else if (checkChunk(chunktype, time_format) == true) { //DO tIME stuff here
		//there should only be ONE tIME chunk in a valid PNG file
		printf("%s", "tIME chunktype\n");

	} else if (checkChunk(chunktype, end_format) == true) { //ENDING stuff here
		printf("%s", "IEND chunktype\n");

	} else { //this chunk is irrelevant, pass it and get to the next chunk
		printf("%s", "irrelevant chunktype\n");

	}


	

    return -1;
}

bool checkChunk(unsigned char *test_chunk, const unsigned char *format) {
	int i = 0;
		for (i = 0; i<4; i++) {
			if (test_chunk[i] != format[i]) {
				return false;
			}
		}
		return true;
	}