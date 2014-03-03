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

	//read the first 8 bytes and check for read error
	if (fread(png_check, 1, SIZE_PNG_FORMAT, f) != SIZE_PNG_FORMAT) {
		return -1;
	}
	if (memcmp(png_check, png_format, SIZE_PNG_FORMAT)) {
		//Not a valid PNG file
		return -1;
	}
	for (i=0; i<8; i++) {
		if (png_check[i] != png_format[i]) {
			//Not a valid PNG file
			return -1;
		}
	}

	//moving on to chunks
	//The following code below should be in a loop, check chunks 1-by-1 until done
	while (done == false) {
		unsigned int length = 0;
		unsigned char len[4];
		unsigned char chunktype[4];
		unsigned char checksum[4];
		if (fread(&length, 4, 1, f) != 1) { //read the chunk length (big endian int)
				return -1;
		}
		unsigned char *int_to_char = (unsigned char *) &length;
		len[3] = int_to_char[0];
		len[2] = int_to_char[1];
		len[1] = int_to_char[2];
		len[0] = int_to_char[3];

		length = (unsigned int) *len;
		//printf("\nThe length of this chunk's data is now: %d\n", length);
		//printf("%s", "\nThe chunktype is: ");
		for (i=0; i<4; i++) { 	//read the chunktype (ASCII)
			if (fread(&chunktype[i],1,1,f) != 1) {
				return -1;
			}
		}

		if (checkChunk(chunktype, text_format) == true) { //------DO tEXt stuff here--------
			//printf("%s", "tEXt chunktype\n");
			bool stop = false;
			unsigned int counter = 0;
			unsigned char complete[length + 2]; //+3 from ':', ' ', '\0', and -1 from overwriting '\0'
			while (stop == false) {
				if (counter >= length || fread(&complete[counter],1,1,f) != 1) {
					return -1;
				}
				if (complete[(int)counter] == 0x00) {
					stop = true;
					//printf("%s", ": "); //debug statement representing the colon in Key: Value
					complete[counter] = ':';
					counter++;
					complete[counter] = ' ';
				} 
				/* else { //this else statement is temporary, for debugging
					printf("%c", complete[counter]); //this is the "key" in Key: Value
				}
				*/
				counter++;
			}
			for (i=counter; i<length + 1; i++) {
				if (fread(&complete[i],1,1,f) != 1) {
					return -1;
				}
				//printf("%c", complete[i]); //this is the "value" in Key: Value
			}
			complete[length + 1] = '\0';
			printf("%s\n", complete);
		} else if (checkChunk(chunktype, ztxt_format) == true) { //-----DO zTXt stuff here--------
			//printf("%s", "zTXt chunktype\n");
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
			//get compressed values
			unsigned int num_left = length - counter - 1;
			unsigned char compressedvalue[num_left];
			for (i=0; i<num_left; i++) {
				if (fread(&compressedvalue[i],1,1,f) != 1) {
					return -1;
				}
			}
			//malloc space for uncompressed
			unsigned long dst_len = num_left*2;
			unsigned char* dst;
			dst = (unsigned char *) malloc(dst_len * sizeof(unsigned char));
			unsigned char* src;
			src = compressedvalue;
			unsigned long src_len = num_left;
			if (dst == NULL) {
				return -1;
			}

			stop = false;
			while (stop == false) {
				size_t value = uncompress(dst, &dst_len, src, src_len);
				if (value == Z_DATA_ERROR || value == Z_MEM_ERROR) {
					return -1;
				} else if (value == Z_BUF_ERROR) {
					dst_len *= 2;
					dst = (unsigned char *) realloc(dst, dst_len * sizeof(unsigned char));
					if (dst == NULL) {
						return -1;
					}
				} else if (value == Z_OK) {
					stop = true;
				}
			}
			for (i=0; i<dst_len; i++) {
				printf("%c", dst[i]);
			}
			printf("%s", "\n");
			free(dst);
		} else if (checkChunk(chunktype, time_format) == true) { //DO tIME stuff here
			//there should only be ONE tIME chunk in a valid PNG file
			/*tentative strategy:
			1) Make sure "length" is at least 7 bytes (reject malformed)
			2) Grab the fields in order (year, month, day, hour, minute, second)
				Note: year is 2 bytes, everything else is 1 byte
			3) Construct the timestamp string
			4) E.g. Timestamp: 12/25/2004 2:39:2
			*/
			unsigned char data[7];
			unsigned int year, month, day, hour, minute, second;
			//tIMe chunk should only be 7 bytes long
			if (length != 7) {
				return -1;
			}
			//read tIMe data
			if (fread(&data,7,1,f) != 1) {
				return -1;
			}
			//fix endianness
			year = data[0]*256 + data[1];
			month = data[2];
			day = data[3];
			hour = data[4];
			minute = data[5];
			second = data[6];
			printf("Timestamp: %u/%u/%u %u:%u:%u\n", month,day,year,hour,minute,second);
		} else if (checkChunk(chunktype, end_format) == true) { //ENDING stuff here
			//printf("%s", "IEND chunktype\n");
			done = true;
			return 0;
		} else { //this chunk is irrelevant, pass it and get to the next chunk
			//printf("%s", "irrelevant chunktype\n");
			//pretty sure there's a way to exploit this
			unsigned char junk[1];
			for (i=0; i<length; i++) {
				if (fread(&junk[0],1,1,f) != 1) {
					return -1;
				}
			}
		}

		for (i=0; i<4; i++) { //read the checksum (CRC-32???)
			fread(&checksum[i],1,1,f);
		}
	}
    return 0;
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