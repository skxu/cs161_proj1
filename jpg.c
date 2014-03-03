#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jpg.h"

/*
 * Analyze a JPG file that contains Exif data.
 * If it is a JPG file, print out all relevant metadata and return 0.
 * If it isn't a JPG file, return -1 and print nothing.
 */
int analyze_jpg(FILE *f) {
		/*
		size_t fread ( void * ptr, size_t size, size_t count, FILE * stream );
		Reads an array of count elements, each one with a size of size bytes, 
		from the stream and stores them in the block of memory specified by ptr.

		The position indicator of the stream is advanced by the total amount of 
		bytes read.

		The total amount of bytes read if successful is (size*count).
		*/
	int i;
	unsigned char marker[2];
	static const unsigned char TIFF_FORMAT[6] = {0x45, 0x78, 0x69, 0x66, 0x00, 0x00};
	static const unsigned int SIZE_TIFF_FORMAT = sizeof(TIFF_FORMAT);
	static const unsigned char ASCII_FORM[8] = {0x41, 0x53, 0x43, 0x49, 0x49, 0x00, 0x00, 0x00};
		
	if (fread(marker, 1, 2, f) != 2) {
		return -1;
	}

	if (marker[0] != 0xff || marker[1] != 0xd8) { // first chunk is Start of Image
		return -1;
	}

	if (fread(marker, 1, 2, f) != 2) { // get next chunk ready
		return -1;
	}

	while (1) {
		if (marker[0] != 0xff) { //invalid marker
			return -1;
		}

		if (marker[1] == 0xd9) { // end of image chunk
			return 0;
		}

		if (marker[1] >= 0xd0 && marker[1] <= 0xda) { //super chunks
			while (1) {
				if (fread(&marker[0], 1, 1, f) != 1) {
					return -1;
				}
				if (marker[0] == 0xff) {
					if (fread(&marker[1], 1, 1, f) != 1) {
						return -1;
					}

					if (marker[1] == 0x00) { // regular data
						continue;
					} else { // end of super chunk
						break;
					}
				}
			}


		} else { //regular chunks
			unsigned int length = 0;
			unsigned char *length_array = (unsigned char*) &length;

			for (i = 1; i >= 0; i--) {
				if (fread(length_array + i, 1, 1, f) != 1) {
					return -1;
				}
			}

			unsigned char data;
			
			if (length < 2) {
				return -1;
			}

			if (marker[1] == 0xe1) { //APP1 chunk
				if (length < 18) {
					return -1;
				}
				unsigned int data_size = length - 10;

				unsigned char tiff_format[SIZE_TIFF_FORMAT];
				if (fread(tiff_format, 1, SIZE_TIFF_FORMAT, f) != SIZE_TIFF_FORMAT) {
					return -1;
				}
				if (memcmp(tiff_format, TIFF_FORMAT, SIZE_TIFF_FORMAT)) {
					return -1;
				}

				unsigned char tiff_header[8];
				if (fread(tiff_header, 1, 8, f) != 8) {
					return -1;
				}

				// endianness
				if (tiff_header[0] != 0x49 || tiff_header[1] != 0x49) {
					return -1;
				}

				// magic string
				if (tiff_header[2] != 0x2a || tiff_header[3] != 0x00) {
					return -1;
				}

				long int header_end = ftell(f);
				unsigned long header_offset = 8;

				unsigned long offset = (unsigned long) (((unsigned int*) tiff_header)[1]);
				if (offset < 8 || offset >= data_size) {
					return -1;
				}
				unsigned int j;
				for (j = 8; j < offset; j++) {  // move to the 0th IFD
					if (fread(&data, 1, 1, f) != 1) {
						return -1;
					}
				}

				unsigned int num_tag_structures = 0;
				if (fread(&num_tag_structures, 2, 1, f) != 1) {
					return -1;
				}

				offset += 2;

				unsigned int exif_ptr = 0;
				unsigned char exif_rdy = 0; // set to true when exif ptr is found

				//tag structures, 0th IFD
				for (j = 0; j < num_tag_structures; j++) {
					unsigned int tagid = 0;
					if (fread(&tagid, 2, 1, f) != 1) {
						return -1;
					}

					unsigned int datatype = 0;
					if (fread(&datatype, 2, 1, f) != 1) {
						return -1;
					}

					unsigned int count = 0;
					if (fread(&count, 4, 1, f) != 1) {
						return -1;
					}

					unsigned int o_o_v = 0;
					if (fread(&o_o_v, 4, 1, f) != 1) {
						return -1;
					}

					if (offset + 12 < offset) {
						return -1;
					}
					offset += 12;

					if (offset > data_size) {
						return -1;
					}

					if (tagid == 0x8769) {  // exif tag structure
						exif_ptr = o_o_v;
						exif_rdy = 1;
					}

					long int next_header_pos = ftell(f);
					unsigned long old_offset = offset;

					unsigned int data_byte_size = 0;
					unsigned int print_string = 0; // set to true if should print a string
					// tag type
					switch (datatype) {
						case 0x0001:
							data_byte_size = 1;
							break;
						case 0x0002:
							data_byte_size = 1;
							print_string = 1;
							switch (tagid) {
								case 0x010d:
									printf("DocumentName: ");
									break;
								case 0x010e:
									printf("ImageDescription: ");
									break;
								case 0x010f:
									printf("Make: ");
									break;
								case 0x0110:
									printf("Model: ");
									break;
								case 0x0131:
									printf("Software: ");
									break;
								case 0x0132:
									printf("DateTime: ");
									break;
								case 0x013b:
									printf("Artist: ");
									break;
								case 0x013c:
									printf("HostComputer: ");
									break;
								case 0x8298:
									printf("Copyright: ");
									break;
								case 0xa004:
									printf("RelatedSoundFile: ");
									break;
								case 0x9003:
									printf("DateTimeOriginal: ");
									break;
								case 0x9004:
									printf("DateTimeDigitized: ");
									break;
								case 0xa420:
									printf("ImageUniqueID: ");
									break;
								default:
									print_string = 0;
									break;
							}
							break;
						case 0x0003:
							data_byte_size = 2;
							break;
						case 0x0004:
							data_byte_size = 4;
							break;
						case 0x0005:
							data_byte_size = 8;
							break;
						case 0x0007:
							data_byte_size = 1;
							switch (tagid) {
								case 0x927c:
									print_string = 1;
									printf("MakerNote: ");
									break;
								case 0x9286:
									// UserComment, handle this later
									break;
								default:
									break;
							}
							break;
						case 0x0008:
							data_byte_size = 2;
							break;
						case 0x0009:
							data_byte_size = 4;
							break;
						case 0x000a:
							data_byte_size = 8;
							break;
						case 0x000b:
							data_byte_size = 4;
							break;
						case 0x000c:
							data_byte_size = 8;
							break;
						default:
							return -1;
					}

					unsigned long data_num_bytes = (unsigned long) count * data_byte_size;

					if (data_num_bytes < 5) { // data fits in o_o_v field
						if (print_string) {
							printf("%.*s\n", (unsigned int) data_num_bytes, (unsigned char*) &o_o_v);
						}
						continue;
					}

					fseek(f, header_end, SEEK_SET);
					offset = header_offset;

					while (offset < o_o_v) {  // move stream to beginning of list
						if (fread(&data, 1, 1, f) != 1) {
							return -1;
						}
						if (offset + 1 < offset) {
							return -1;
						}
						offset += 1;
					}

					if (offset + data_num_bytes < offset) {
						return -1;
					}
					unsigned long end = offset + data_num_bytes; // end of list

					if (datatype == 0x0007 && tagid == 0x9286 && data_num_bytes >= 8) { // UserComment
						unsigned char char_set[8];
						if (fread(char_set, 1, 8, f) != 8) {
							return -1;
						}
						if (offset + 8 < offset) {
							return -1;
						}
						offset += 8;
						if (!memcmp(char_set, ASCII_FORM, 8)) {
							print_string = 1;
							printf("UserComment: ");
						}
					}

					if (print_string) {
						while (offset < end) {
							if (fread(&data, 1, 1, f) != 1) {
								return -1;
							}
							if (data == 0x00) {
								break;
							}
							printf("%c", data);
							if (offset + 1 < offset) {
								return -1;
							}
							offset++;
						}
						printf("\n");
					}

					fseek(f, next_header_pos, SEEK_SET);
					offset = old_offset;
				}

				if (offset > data_size) {
					return -1;
				}

				if (!exif_rdy) {
					while (offset < data_size) {
						if (fread(&data, 1, 1, f) != 1) {
							return -1;
						}
						if (offset + 1 < offset) {
							return -1;
						}
						offset++;
					}

					if (fread(marker, 1, 2, f) != 2) { // get next chunk ready
						return -1;
					}

					return 0;
				}

				// scan to exif IFD
				while (offset < exif_ptr) {
					if (fread(&data, 1, 1, f) != 1) {
						return -1;
					}
					if (offset + 1 < offset) {
						return -1;
					}
					offset++;
				}

				num_tag_structures = 0;
				if (fread(&num_tag_structures, 2, 1, f) != 1) {
					return -1;
				}

				offset += 2;

				//tag structures, exif IFD
				for (j = 0; j < num_tag_structures; j++) {
					unsigned int tagid = 0;
					if (fread(&tagid, 2, 1, f) != 1) {
						return -1;
					}

					unsigned int datatype = 0;
					if (fread(&datatype, 2, 1, f) != 1) {
						return -1;
					}

					unsigned int count = 0;
					if (fread(&count, 4, 1, f) != 1) {
						return -1;
					}

					unsigned int o_o_v = 0;
					if (fread(&o_o_v, 4, 1, f) != 1) {
						return -1;
					}

					if (offset + 12 < offset) {
						return -1;
					}
					offset += 12;

					if (offset > data_size) {
						return -1;
					}

					long int next_header_pos = ftell(f);
					unsigned long old_offset = offset;

					unsigned int data_byte_size = 0;
					unsigned int print_string = 0; // set to true if should print a string
					// tag type
					switch (datatype) {
						case 0x0001:
							data_byte_size = 1;
							break;
						case 0x0002:
							data_byte_size = 1;
							print_string = 1;
							switch (tagid) {
								case 0x010d:
									printf("DocumentName: ");
									break;
								case 0x010e:
									printf("ImageDescription: ");
									break;
								case 0x010f:
									printf("Make: ");
									break;
								case 0x0110:
									printf("Model: ");
									break;
								case 0x0131:
									printf("Software: ");
									break;
								case 0x0132:
									printf("DateTime: ");
									break;
								case 0x013b:
									printf("Artist: ");
									break;
								case 0x013c:
									printf("HostComputer: ");
									break;
								case 0x8298:
									printf("Copyright: ");
									break;
								case 0xa004:
									printf("RelatedSoundFile: ");
									break;
								case 0x9003:
									printf("DateTimeOriginal: ");
									break;
								case 0x9004:
									printf("DateTimeDigitized: ");
									break;
								case 0xa420:
									printf("ImageUniqueID: ");
									break;
								default:
									print_string = 0;
									break;
							}
							break;
						case 0x0003:
							data_byte_size = 2;
							break;
						case 0x0004:
							data_byte_size = 4;
							break;
						case 0x0005:
							data_byte_size = 8;
							break;
						case 0x0007:
							data_byte_size = 1;
							switch (tagid) {
								case 0x927c:
									print_string = 1;
									printf("MakerNote: ");
									break;
								case 0x9286:
									// UserComment, handle this later
									break;
								default:
									break;
							}
							break;
						case 0x0008:
							data_byte_size = 2;
							break;
						case 0x0009:
							data_byte_size = 4;
							break;
						case 0x000a:
							data_byte_size = 8;
							break;
						case 0x000b:
							data_byte_size = 4;
							break;
						case 0x000c:
							data_byte_size = 8;
							break;
						default:
							return -1;
					}

					unsigned long data_num_bytes = (unsigned long) count * data_byte_size;

					if (data_num_bytes < 5) { // data fits in o_o_v field
						if (print_string) {
							printf("%.*s\n", (unsigned int) data_num_bytes, (unsigned char*) &o_o_v);
						}
						continue;
					}

					fseek(f, header_end, SEEK_SET);
					offset = header_offset;

					while (offset < o_o_v) {  // move stream to beginning of list
						if (fread(&data, 1, 1, f) != 1) {
							return -1;
						}
						if (offset + 1 < offset) {
							return -1;
						}
						offset += 1;
					}

					if (offset + data_num_bytes < offset) {
						return -1;
					}
					unsigned long end = offset + data_num_bytes; // end of list

					if (datatype == 0x0007 && tagid == 0x9286 && data_num_bytes >= 8) { // UserComment
						unsigned char char_set[8];
						if (fread(char_set, 1, 8, f) != 8) {
							return -1;
						}
						if (offset + 8 < offset) {
							return -1;
						}
						offset += 8;
						if (!memcmp(char_set, ASCII_FORM, 8)) {
							print_string = 1;
							printf("UserComment: ");
						}
					}

					if (print_string) {
						while (offset < end) {
							if (fread(&data, 1, 1, f) != 1) {
								return -1;
							}
							if (data == 0x00) {
								break;
							}
							printf("%c", data);
							if (offset + 1 < offset) {
								return -1;
							}
							offset++;
						}
						printf("\n");
					}

					fseek(f, next_header_pos, SEEK_SET);
					offset = old_offset;
				}



				// scan to end of data
				if (offset > data_size) {
					return -1;
				}

				while (offset < data_size) {
					if (fread(&data, 1, 1, f) != 1) {
						return -1;
					}
					if (offset + 1 < offset) {
						return -1;
					}
					offset++;
				}

				return 0;

			} else { // other

				for (; length > 2; length--) {
					if (fread(&data, 1, 1, f) != 1) {
						return -1;
					}
				}
			}

			if (fread(marker, 1, 2, f) != 2) { // get next chunk ready
				return -1;
			}
		}
	}
	return -1;
}
