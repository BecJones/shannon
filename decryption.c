/*******
 * Shannon
 * Developer: Becquerel Jones
 * Last Updated: October 20, 2019
 * OS: WSL Ubuntu on Windows 10
*****/

#include "shannon.h"

// Decode transmission
int decode(FILE* file, uint64_t filesize, struct dataString* outfile) {
	// Result in case of operation failure
	int res;

	// File data
	struct dataString data;

	// Reconstituted data
	// LSB of each byte in data, mashed into a string
	struct dataString recd;

	// Header index
	uint64_t hindex;

	// Allocate memory for file contents
	data.size = filesize;
	data.data = malloc(data.size);

	// Read file
	fread(data.data, sizeof(*(data.data)), data.size, file);

	printf("\nTransmission size: %lu\n", data.size);

	// Reconstitute data
	if((res = reconstitute(data, &recd))) {
		return res;
	}

	// Find header in file data
	if((res = getOffset(recd, &hindex))) {
		return res;
	}

	// Cleanup
	free(data.data);
	free(recd.data);

	return 0;
}

// Reconstitute data from raw file input
int reconstitute(struct dataString data, struct dataString* recd) {
	// Allocate reconstituted data size
	if(data.size % 8 == 0) {
		recd->size = data.size / 8;
	} else {
		recd->size = (data.size / 8) + 1;
	}
	recd->data = malloc(recd->size);

	// Index variables
	uint64_t i = 0;
	uint64_t bytenum = 0;
	int bitnum = 0;

	for(bytenum = 0; bytenum < data.size; bytenum = bytenum + 1) {
		bitnum = bytenum % 8;
		if(bitnum == 0) {
			recd->data[i] = 0x00;
		}
		recd->data[i] = recd->data[i] |
			((data.data[bytenum] & 0x01) << bitnum);
		if(bitnum == 7) {
			i = i + 1;
		}
	}

	return 0;
}

// Get offset from start of file to start of header
int getOffset(struct dataString recd, uint64_t *offset) {
	// Variables
	unsigned char found;
	uint64_t i;
	int j;
	struct dataString header;
	header.size = HEADER_LENGTH;
	header.data = malloc(header.size);

	// Generate header
	for(i = 0; i < header.size; i = i + 1) {
		header.data[i] = 0xFF - i;
	}

	// Find header
	for(i = header.size; i < recd.size; i = i + 1) {
		found = 0x01;
		for(j = 0; j < header.size; j = j + 1) {
			if(recd.data[i - header.size + j] != header.data[j]) {
				found = 0x00;
			}
		}
		if(found) {
			*offset = i;
			i = recd.size;
		}
	}

	if(!found) {
		printf("ERROR: No Header Found\n");
		return 8;
	}

	// Cleanup
	free(header.data);

	return 0;
}
