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


	
	//Check if first 8 bytes match standard png type
	unsigned int i = 0;
	bool done = false;
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
	while (done == false) {
		unsigned int length = 0;
		unsigned char chunktype[4];
		unsigned char checksum[4];
		fread(&length, 4, 1, f); //read the chunk length (big endian int)

		printf("%s", "\nThe chunktype is: ");
		for (i=0; i<4; i++) { 	//read the chunktype (ASCII)
			fread(&chunktype[i],1,1,f);
			printf("%c", chunktype[i]);
		}

		printf("\nThe length of this chunk's data is: %d\n", length);

		if (checkChunk(chunktype, text_format) == true) { // DO tEXt stuff here
			printf("%s", "tEXt chunktype\n");
			
			//filler method for now, NEED TO CHANGE
			//tentative strategy: read byte by byte until 0x00
			//then read the rest based on length
			
			bool stop = false;
			unsigned int counter = 0;
			unsigned char key[length];
			
			while (stop == false) {
				fread(&key[counter],1,1,f);
				if (key[counter] == 0x00) {
					stop = true;
					printf("%s", ": "); //debug statement representing the colon in Key: Value
				} else { //this else statement is temporary, for debugging
					printf("%c", key[counter]); //this is the "key" in Key: Value
				}
				counter++;
			}
			unsigned int num_left = length - (counter);
			unsigned char value[num_left];
			for (i=0; i<num_left; i++) {
				fread(&value[i],1,1,f);
				printf("%c", value[i]); //this is the "value" in Key: Value
			}
			


			


		} else if (checkChunk(chunktype, ztxt_format) == true) { //DO zTXt stuff here
			printf("%s", "zTXt chunktype\n");
			
			//filler method for now, NEED TO CHANGE
			unsigned char junk[length];
			for (i=0; i<length; i++) {
				fread(&junk,1,1,f);
			}

		} else if (checkChunk(chunktype, time_format) == true) { //DO tIME stuff here
			//there should only be ONE tIME chunk in a valid PNG file
			printf("%s", "tIME chunktype\n");
			
			//filler method for now, NEED TO CHANGE
			unsigned char junk[length];
			for (i=0; i<length; i++) {
				fread(&junk,1,1,f);
			}

		} else if (checkChunk(chunktype, end_format) == true) { //ENDING stuff here
			printf("%s", "IEND chunktype\n");
			done = true;
		} else { //this chunk is irrelevant, pass it and get to the next chunk
			printf("%s", "irrelevant chunktype\n");

			//NOT SURE IF THIS IS DANGEROUS OR NOT HELP ME
			//lack of sleep atm
			//pretty sure there's a way to exploit this
			unsigned char junk[length];
			for (i=0; i<length; i++) {
				fread(&junk,1,1,f);
			}

		}

		for (i=0; i<4; i++) { //read the checksum (CRC-32???)
			fread(&checksum[i],1,1,f);
		}
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