/*******
 * Shannon; Encryption
 * Becquerel Jones
 * December 27, 2019
 * Debian 10: Buster
 * Vim
*****/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include "encryption.h"

// Encode
//
// Returns:    0 if successful
// 	      -1 if output file too small
// 	      -2 if out of memory
int encode(FILE **files, uint64_t *filesizes, struct datastring *outfile) {
	// Variables
	int res; // Return flag
	unsigned char i; // Iteration index

	// Signal parts
	//
	// 0: Key 0
	// 1: Key 1
	// 2: Key 2
	// 3: Header
	// 4: Message Length (filesizes[0])
	// 5: Message (files[0])
	struct datastring *sigparts = malloc(6 * sizeof(*sigparts));
	
	struct datastring finalsig; // Container for final signal at the end

	// Assign sizes to signal parts and output file
	sigparts[0].size = KEY_0_LENGTH;
	sigparts[1].size = KEY_1_LENGTH;
	sigparts[2].size = KEY_2_LENGTH;
	sigparts[3].size = HEADER_LENGTH;
	sigparts[4].size = sizeof(filesizes[0]);
	sigparts[5].size = filesizes[0];

	outfile->size = filesizes[1];

	// Make sure output file is big enough
	if(outfile->size / 8 < sigparts[0].size + sigparts[1].size +
			sigparts[2].size + sigparts[3].size +
			sigparts[4].size + sigparts[5].size + (MAX_OFFSET / 8)) {
		return -1;
	}

	// Allocate memory for signal parts and output file
	for(i = 0; i < 6; i = i + 1) {
		sigparts[i].data = malloc(sigparts[i].size);
		if(errno == ENOMEM) {
			return -2;
		}
	}

	outfile->data = malloc(outfile->size);
	if(errno == ENOMEM) {
		return -2;
	}

	// Read files
	fread(sigparts[5].data, 1, sigparts[5].size, files[0]);
	fread(outfile->data, 1, outfile->size, files[1]);

	// Load signal parts
	if((res = loadSigParts(sigparts)) < 0) {
		return res;
	}

	// Apply keys to signal
	if((res = applyKeys(sigparts)) < 0) {
		return res;
	}

	// Concatenate final signal
	if((res = buildFinalSignal(&finalsig, sigcomp))) {
		return res;
	}

	// Hide signal in noise
	if((res = insertSignal(outfile, &finalsig))) {
		return res;
	}

	// Cleanup
	for(i = 0; i < 6; i = i + 1) {
		free(sigparts[i].data);
	}
	free(finalsig.data);
	free(sigparts);

	return 0;
} // Encode


// Load Signal Parts
//
// Returns:    0 if successful
// 	      -2 if out of memory
int loadSigParts(struct datastring *sigparts) {
	// Variables
	int res; // Result flag

	// Initialize header
	if((res = initHeader(sigparts)) < 0) {
		return res;
	}

	// Initialize keys
	if((res = initKeys(sigparts)) < 0) {
		return res;
	}

	// Initialize size
	if((res = initSize(sigparts)) < 0) {
		return res;
	}

	return 0;
} // Load Signal Parts


// Initialize Header
int initHeader(struct datastring *sigparts) {
	// Variables
	unsigned int index; // Iteration index

	// Generate header
	for(index = 0; index < sigparts[3].size; index = index + 1) {

		// All 1, except index bit
		sigparts[3].data[index] = 0xFF ^ (0x01 << index);
	}

	return 0;
} // Initialize Header


// Initialize Keys
// Returns:     0 if successful
// 	       -2 if out of memory
int initKeys(struct datastring *sigparts) {
	// Variables
	unsigned int keyNum; // Key being initialized
	unsigned int keyInd; // Iteration index
	unsigned int random; // Random number
	unsigned int tmp; // General temp storage
	unsigned char *testStr; // Test string for later

	// Seed random number generation
	srand(time(0));

	// Generate keys
	for(keyNum = 0; keyNum < 3; keyNum = keyNum + 1) {
		for(keyInd = 0; keyInd < sigparts[keyNum].size; keyInd = keyInd + 1) {
			tmp = keyInd % sizeof(random);
			if(tmp == 0) {
				random = rand();
			}
			sigparts[keyNum].data[keyInd] = (random >> (8 * tmp)) & 0xFF;
		}
	}

	// Ensure that no keys include header sequence
	keyNum = sigparts[0].size + sigparts[1].size + sigparts[2].size;
	testStr = malloc(keyNum);
	if(errno == ENOMEM) {
		return -2;
	}

	// Calvinball; using all the variables as iterators now
	// Loop through all keys
	for(keyInd = 0; keyInd < keyNum - (sigparts[3].size - 1); keyInd = keyInd + 1) {
		tmp = 0;

		// Check for substring that looks like header
		for(random = 0; random < sigparts[3].size; random = random + 1) {
			if(testStr[keyInd + random] != sigparts[3].data[random]) {
				tmp = 1;
				break;
			}
		}

		// If this chunk of keys includes the header
		// modify the appropriate key
		if(tmp == 0) {
			if(keyInd < sigparts[0].size) {
				tmp = keyInd;
				random = 0;
			} else if(keyInd < sigparts[1].size) {
				tmp = keyInd - sigparts[0].size;
				random = 1;
			} else {
				tmp = keyInd -
					(sigparts[0].size + sigparts[1].size);
				random = 2;
			}
			sigparts[random].data[tmp] =
				sigparts[random].data[tmp] - 1;
		}
	}

	// Clean up
	free(testStr);

	return 0;
} // Initialize Keys


// Initialize Size
int initSize(struct datastring *sigparts) {
	// Variables
	unsigned int index;

	// Load size of message into appropriate string
	for(index = 0; index < sizeof(sigparts[4].size); index = index + 1) {
		sigparts[4].data[index] = (sigparts[5].size >> (index * 8)) & 0xFF;
	}

	return 0;
} // Initialize Size


// Apply Keys
int applyKeys(struct datastring *sigparts) {
	// Variables
	uint64_t dataIndex;
	unsigned int keyIndex;

	// Loop through all data
	for(dataIndex = 0; dataIndex < sigcomp[5].size; dataIndex = dataIndex + 1) {

		// XOR each byte of data with the next byte of each key
		for(keyIndex = 0; keyIndex < 3; keyIndex = keyIndex + 1) {
			sigcomp[5].data[dataIndex] = sigcomp[5].data[dataIndex] ^ sigcomp[keyIndex].data[dataIndex % sigcomp[keyIndex].size];
		}
	}

	return 0;
} // Apply Keys


// Build final signal
int buildFinalSignal(struct datastring *finalsig, struct datastring *sigparts) {
	int i;
	uint64_t size = 0;

	for(i = 0; i < 6; i = i + 1) {
		size = size + sigcomp[i].size;
	}
	finalsig->size = size;
	finalsig->data = malloc(size);
	for(i = 0; i < 6; i = i + 1) {
		strcat(finalsig->data, sigcomp[i].data);
	}

	return 0;
}

// Hide signal in noise
int insertSignal(struct dataString* output, struct dataString* signal, struct dataString* noise, unsigned char* header) {
	int offset;
	uint64_t bytenum;
	int bitnum;
	uint64_t i;
	int 
	unsigned char* headercheck;
	unsigned char currentbit;

	offset = (rand() % 300) + 113;
	
	// Ensure that the header sequence doesn't appear by chance before the signal
	headercheck = malloc(offset / 8);
	bytenum = 0;
	headercheck[0] = 0x00;
	for(i = 0; i < (offset / 8) * 8; i = i + 1) {
		headercheck[bytenum] = headercheck[bytenum] |
			(noise->data[i] & 0x01);
		headercheck[bytenum] = headercheck[bytenum] << 1;
		if(i % 8 == 7) {
			if(bytenum > 3) {
				if(headercheck[bytenum - 4] == header[0] &&
						headercheck[bytenum - 3] == header[1] &&
						headercheck[bytenum - 2] == header[2] &&
						headercheck[bytenum - 1] == header[3] &&
						headercheck[bytenum] == header[4]) {
					noise->data[i] = noise->data[i] ^ 0x01;
				}
			}
			bytenum = bytenum + 1;
			headercheck[bytenum] = 0x00;
		}
	}

	// Hide the signal
	bytenum = 0;
	bitnum = 0;
	strcpy(output->data, noise->data);

	for(bytenum = 0; bytenum < signal->size; bytenum = bytenum + 1) {
		for(bitnum = 0; bitnum < 8; bitnum = bitnum + 1) {
			i = offset + ((bytenum * 8) + bitnum);
			output->data[i] = (output->data[i] & 0xFE) |
				((signal->data[bytenum] >> bitnum) & 0x01);
		}
	}
	
	// Cleanup
	free(headercheck);

	return 0;
}
