#ifndef PNG_H_GUARD
#define PNG_H_GUARD

int analyze_png(FILE *f);
typedef enum {
	false = 0,
	true
} bool;
bool checkChunk(unsigned char *test_chunk, const unsigned char *format );

#endif
