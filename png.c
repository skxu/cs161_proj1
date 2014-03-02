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

	const size_t SIZE_PNG_FORMAT = sizeof(png_format);

		/*
		size_t fread ( void * ptr, size_t size, size_t count, FILE * stream );
		Reads an array of count elements, each one with a size of size bytes, 
		from the stream and stores them in the block of memory specified by ptr.

		The position indicator of the stream is advanced by the total amount of 
		bytes read.

		The total amount of bytes read if successful is (size*count).
		*/


	//read the first 8 bytes and check for read error
	if (fread(png_check, 1, SIZE_PNG_FORMAT, f) != SIZE_PNG_FORMAT) {
		return -1;
	}

	if (memcmp(png_check, png_format, SIZE_PNG_FORMAT)) {
		//Not a valid PNG file
		return -1;
	}
//***** combine these checks?
	

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

		if (fread(&length, 4, 1, f) != 1) { //read the chunk length (big endian int)
			return -1;
		}

//***** temporarily force big endiannness
		length = length >> 24;

		printf("%s", "\nThe chunktype is: ");
		for (i=0; i<4; i++) { 	//read the chunktype (ASCII)
			if (fread(&chunktype[i],1,1,f) != 1) {
				return -1;
			}
			printf("%c", chunktype[i]);
		}

		printf("\nThe length of this chunk's data is: %d\n", length);

//***** switch statement here imo for chunktypes
		if (checkChunk(chunktype, text_format) == true) { // DO tEXt stuff here
			printf("%s", "tEXt chunktype\n");
			
			//filler method for now, NEED TO CHANGE
			//tentative strategy: read byte by byte until 0x00
			//then read the rest based on length
			
			bool stop = false;
			unsigned int counter = 0;
			unsigned char key[length];
			
			while (stop == false) {
				if (counter == length || fread(&key[counter],1,1,f) != 1) {
					return -1;
				}
				if (key[counter] == 0x00) {
					stop = true;
					printf("%s", ": "); //debug statement representing the colon in Key: Value
				} else { //this else statement is temporary, for debugging
					printf("%c", key[counter]); //this is the "key" in Key: Value
				}
				counter++;
			}
//***** below could be stored in back of "key" array if i starts at counter and goes to length
			unsigned int num_left = length - counter;
			unsigned char value[num_left];
			for (i=0; i<num_left; i++) {
				if (fread(&value[i],1,1,f) != 1) {
					return -1;
				}
				printf("%c", value[i]); //this is the "value" in Key: Value
			}
			


		} else if (checkChunk(chunktype, ztxt_format) == true) { //DO zTXt stuff here
			printf("%s", "zTXt chunktype\n");
			
			bool stop = false;
			unsigned int counter = 0;
			unsigned char key[length];

			//read key
			while (stop == false) {
				if (counter == length || fread(&key[counter],1,1,f) != 1) {
					return -1;
				}
				if (key[counter] == 0x00) {
					stop = true;
					printf("%s", ": ");
				} else {
					printf("%c", key[counter]);
				}
				counter++;
			}
			unsigned char ctype[1];
			//read compressiontype
			if (fread(&ctype[0],1,1,f) != 1 || ctype[0] != 0x00) {
				return -1;
			}

			unsigned char junk[length];
			for (i=0; i<length; i++) {
				fread(&junk,1,1,f);
			}

		} else if (checkChunk(chunktype, time_format) == true) { //DO tIME stuff here
			//there should only be ONE tIME chunk in a valid PNG file
			printf("%s", "tIME chunktype\n");
			
			/*tentative strategy:
			1) Make sure "length" is at least 7 bytes (reject malformed)
			2) Grab the fields in order (year, month, day, hour, minute, second)
				Note: year is 2 bytes, everything else is 1 byte
			3) Construct the timestamp string
			4) E.g. Timestamp: 12/25/2004 2:39:2
			*/	
			unsigned int year, month, day, hour, minute, second;
			
			if (length != 7) {
				printf("%s", "what2");
			}
			
			if ((fread(&year,2,1,f) != 1) || (fread(&month,1,1,f) != 1) || (fread(&day,1,1,f) != 1) || (fread(&hour,1,1,f) != 1) || (fread(&minute,1,1,f) != 1) || (fread(&second,1,1,f) != 1)) {
				printf("%s", "what");
			}
			year = year >> 16;
			month = month >> 24;
			day = day >> 24;
			hour = hour >> 24;
			minute = minute >> 24;
			second = second >> 24;
			printf("%u/%u/%u %u:%u:%u", month,day,year,hour,minute,second);
			
		} else if (checkChunk(chunktype, end_format) == true) { //ENDING stuff here
			printf("%s", "IEND chunktype\n");
			done = true;
			return 0;

		} else { //this chunk is irrelevant, pass it and get to the next chunk
			printf("%s", "irrelevant chunktype\n");

			//pretty sure there's a way to exploit this
			unsigned char junk;
			for (i=0; i<length; i++) {
				if (fread(&junk,1,1,f) != 1) {
					return -1;
				}
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