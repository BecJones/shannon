/*******
 * Shannon
 * Developer: Becquerel Jones
 * Last Updated: September 22, 2019
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



	return 0;
}

// Get offset from start of file to start of header
int getOffset(struct dataString recd, uint64_t* offset) {
	return 0;
}
