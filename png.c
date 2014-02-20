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
	int i = 0;
	printf("hello world");
	unsigned char png_check[8];
	for (i=0; i<8; i++) {
		fread(&png_check[i],1,1, f);
	}

	for (i=0; i<8; i++) {
		printf("%d\n", png_check[i]);
	}

	//char file_name[]

    //printf(file_name)
    return -1;
}
