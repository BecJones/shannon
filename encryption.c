/*******
 * Shannon; Encryption
 * Becquerel Jones
 * January 23, 2020
 * Debian 10: Buster
 * Vim
*****/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "encryption.h"


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
	//
	// keyInd: combined key byte index
	// keyNum: combined length of all keys
	// tmp: does this substring look like the header sequence? If so, what is the start index?
	// random: header byte index
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


// Get Offset
//
// Returns:    0 if successful
// 	      -1 if header not found
// 	      -2 if out of memory
int getOffset(struct datastring *sigparts, uint64_t *offset,
		struct datastring data) {
	// Variables
	uint64_t dataIndex; // Data index
	uint8_t headerIndex; // Header index
	int res; // Result flag

	// Allocate memory for header
	sigparts[3].size = HEADER_LENGTH;
	sigparts[3].data = malloc(sigparts[3].size * sizeof(*(sigparts[3].data)));
	if(errno == ENOMEM) {
		return -2;
	}

	// Generate header
	if((res = initHeader(sigparts)) < 0) {
		return res;
	}

	// Loop through data
	for(dataIndex = 0; dataIndex < data.size - (sigparts[3].size - 1); dataIndex = dataIndex + 1) {
		res = 0;

		// Check for header substring
		for(headerIndex = 0; headerIndex < sigparts[3].size; headerIndex = headerIndex + 1) {
			if(data.data[dataIndex + headerIndex] != sigparts[3].data[headerIndex]) {
				res = 1;
				break;
			}
		}

		// No difference between substring and header; header found
		if(res == 0) {
			*offset = dataIndex;
			return 0;
		}
	}

	// Header not found in data
	return -1;
} // Get Offset


// Get Keys
//
// Returns:    0 if successful
// 	      -2 if out of memory
int getKeys(struct datastring *sigparts, uint64_t *offset,
		struct datastring data) {
	// Variables
	uint64_t loopStart; // Loop start
	uint64_t dataIndex; // Data index
	uint8_t keyIndex; // Key index

	// Allocate space for keys
	sigparts[0].size = KEY_0_LENGTH;
	sigparts[1].size = KEY_1_LENGTH;
	sigparts[2].size = KEY_2_LENGTH;
	for(keyIndex = 0; keyIndex < 3; keyIndex = keyIndex + 1) {
		sigparts[keyIndex].data =
			malloc(sigparts[keyIndex].size * sizeof(*(sigparts[keyIndex].data)));
		if(errno == ENOMEM) {
			return -2;
		}
	}

	// Load key 0
	loopStart = *offset - (KEY_0_LENGTH + KEY_1_LENGTH + KEY_2_LENGTH);
	for(keyIndex = 0; keyIndex < KEY_0_LENGTH; keyIndex = keyIndex + 1) {
		sigparts[0].data[keyIndex] = data.data[loopStart + keyIndex];
	}

	// Load key 1
	loopStart = loopStart + KEY_0_LENGTH;
	for(keyIndex = 0; keyIndex < KEY_1_LENGTH; keyIndex = keyIndex + 1) {
		sigparts[1].data[keyIndex] = data.data[loopStart + keyIndex];
	}

	// Load key 2
	loopStart = loopStart + KEY_1_LENGTH;
	for(keyIndex = 0; keyIndex < KEY_2_LENGTH; keyIndex = keyIndex + 1) {
		sigparts[2].data[keyIndex] = data.data[loopStart + keyIndex];
	}

	return 0;
} // Get Keys


// Get Size
//
// Returns:    0 if successful
// 	      -2 if out of memory
int getSize(struct datastring *sigparts, uint64_t *offset,
		struct datastring data) {
	// Variables
	uint64_t loopStart; // Loop start
	uint8_t sizeIndex; // Size index

	// Allocate space for size
	sigparts[4].size = sizeof(sigparts[4].size);
	sigparts[4].data = malloc(sigparts[4].size * sizeof(*(sigparts[4].data)));
	if(errno == ENOMEM) {
		return -2;
	}

	// Load size
	for(sizeIndex = 0; sizeIndex < sigparts[4].size; sizeIndex = sizeIndex + 1) {
		sigparts[4].data[sizeIndex] = data.data[*offset + HEADER_LENGTH + sizeIndex];
	}

	return 0;
} // Get Size


// Get Signal
//
// Returns:    0 if successful
// 	      -2 if out of memory
int getSignal(struct datastring *sigparts, uint64_t *offset,
		struct datastring data) {
	// Variables
	uint64_t index; // Index

	// Initialize signal size
	sigparts[5].size = 0;

	// Load signal size from size part
	for(index = 0; index < sigparts[4].size; index = index + 1) {
		sigparts[5].size = (sigparts[5].size | sigparts[4].data[index]) << 8;
	}

	// Allocate memory for signal
	sigparts[5].data = malloc(sigparts[5].size);
	if(errno == ENOMEM) {
		return -2;
	}

	// Load signal from data
	for(index = 0; index < sigparts[5].size; index = index + 1) {
		sigparts[5].data[index] =
			data.data[index + *offset + sigparts[3].size + sigparts[4].size];
	}

	return 0;
} // Get Signal


// Apply Keys
int applyKeys(struct datastring *sigparts) {
	// Variables
	uint64_t dataIndex;
	unsigned int keyIndex;

	// Loop through all data
	for(dataIndex = 0; dataIndex < sigparts[5].size; dataIndex = dataIndex + 1) {

		// XOR each byte of data with the next byte of each key
		for(keyIndex = 0; keyIndex < 3; keyIndex = keyIndex + 1) {
			sigparts[5].data[dataIndex] = sigparts[5].data[dataIndex] ^ 
				sigparts[keyIndex].data[dataIndex % sigparts[keyIndex].size];
		}
	}

	return 0;
} // Apply Keys


// Assemble Signal
//
// Returns:    0 if successful
// 	      -2 if out of memory
int assembleSignal(struct datastring *finalsig, struct datastring *sigparts) {
	// Variables
	unsigned char partindex; // Part index
	uint64_t dataindex; // Data index
	uint64_t totalsize = 0; // Size sum

	// Sum sizes of parts; finalsig size to sum
	for(partindex = 0; partindex < 6; partindex = partindex + 1) {
		totalsize = totalsize + sigparts[partindex].size;
	}
	finalsig->size = totalsize;

	// Allocate total memory
	finalsig->data = malloc(finalsig->size * sizeof(*(finalsig->data)));
	if(errno == ENOMEM) {
		return -2;
	}

	// Copy all parts to finalsig
	//
	// totalsize: finalsig data index
	totalsize = 0;
	for(partindex = 0; partindex < 6; partindex = partindex + 1) {
		for(dataindex = 0; dataindex < sigparts[partindex].size; dataindex = dataindex + 1) {
			finalsig->data[totalsize] = sigparts[partindex].data[dataindex];
			totalsize = totalsize + 1;
		}
	}

	return 0;
} // Assemble Signal


// Disassemble Signal
int disassembleSignal(struct datastring *sigparts, struct datastring data) {
	// Variables
	int res; // Result flag
	uint64_t *offset = malloc(sizeof(*offset)); // Header offset from start of file
	if(errno == ENOMEM) {
		return -2;
	}

	if((res = getOffset(sigparts, offset, data)) < 0) { // Get offset and header
		return res;
	}

	if((res = getKeys(sigparts, offset, data)) < 0) { // Get keys
		return res;
	}

	if((res = getSize(sigparts, offset, data)) < 0) { // Get signal size
		return res;
	}

	if((res = getSignal(sigparts, offset, data)) < 0) { // Get signal
		return res;
	}

	return 0;
} // Disassemble Signal


// Deconstitute Signal
//
// Returns:    0 if successful
// 	      -2 if out of memory
int deconstitute(struct datastring *noise, struct datastring signal,
		struct datastring header) {
	// Variables
	unsigned int offset; // Offset from beginning of file
	uint64_t noisebyte; // Noise byte index
	uint64_t signalbyte = 0; // Signal byte index
	unsigned char signalbit = 0; // Signal bit index
	struct datastring headercheck; // For checking to ensure that header doesn't show up prematurely later

	// Pick random offset from beginning of file
	srand(time(0));
	offset = (rand() % (MAX_OFFSET - MIN_OFFSET)) + MIN_OFFSET;

	// Deconstitute signal into noise
	// Loop through noise, byte by byte, starting at offset
	for(noisebyte = offset; signalbyte < signal.size; noisebyte = noisebyte + 1) {

		// Force noise byte's lsb to signal byte's current bit
		noise->data[noisebyte] = (noise->data[noisebyte] & 0xFE) |
			((signal.data[signalbyte] >> signalbit) & 0x01);

		// Increment signal bit
		signalbit = signalbit + 1;

		// When signal byte is done, increment signal byte and reset signal bit
		if(signalbit >= 8) {
			signalbit = 0;
			signalbyte = signalbyte + 1;
		}
	}
	
	// Ensure that the header sequence doesn't appear by chance before the signal
	// Keys are before header and longer than header; it's okay to overshoot by up to a byte
	// It's less okay to read a false positive header
	headercheck.size = (offset / 8) + 1;
	headercheck.data = malloc(headercheck.size);
	if(errno == ENOMEM) {
		return -2;
	}

	// Reconstitute noise file prior to signal
	signalbyte = 0;
	signalbit = 0;
	for(noisebyte = 0; noisebyte < headercheck.size * 8; noisebyte = noisebyte + 1) {

		// Clear new "signal" byte
		if(signalbit == 0) {
			headercheck.data[signalbyte] = 0x00;
		}

		// Force next bit of "signal" byte to lsb of current noise byte
		headercheck.data[signalbyte] = headercheck.data[signalbyte] |
			((noise->data[noisebyte] & 0x01) << signalbit);

		// Increment signal bit
		signalbit = signalbit + 1;

		// When signal byte is done, increment signal byte and reset signal bit
		if(signalbit >= 8) {
			signalbit = 0;
			signalbyte = signalbyte + 1;
		}
	}

	// Check "signal" for fake headers
	//
	// signalbyte: headercheck index
	// signalbit: header found/header start index
	// noisebyte: header index
	for(signalbyte = 0; signalbyte < headercheck.size - (header.size - 1); signalbyte = signalbyte + 1) {
		signalbit = 0;

		// Check substring for header
		for(noisebyte = 0; noisebyte < header.size; noisebyte = noisebyte + 1) {
			if(headercheck.data[signalbyte + noisebyte] != header.data[noisebyte]) {
				signalbit = 1;
				break;
			}
		}

		// No difference from header; flip first bit of first byte of substring in noise file
		if(signalbit == 0) {
			noise->data[signalbyte * 8] = noise->data[signalbyte * 8] ^ 0x01;

			// Start over
			signalbyte = 0;
		}
	}

	// Cleanup
	free(headercheck.data);

	return 0;
} // Deconstitute Signal


// Reconstitute Signal
//
// Returns:    0 if successful
// 	      -2 if out of memory
int reconstitute(struct datastring *data, struct datastring raw) {
	// Variables
	unsigned char offset;
	uint64_t byteindex;

	// Allocate memory for reconstituted signal
	data->size = raw.size / 8;
	data->data = malloc(data->size * sizeof(*(data->data)));
	if(errno == ENOMEM) {
		return -2;
	}

	// Consilidate LSBs from raw into data
	for(byteindex = 0; byteindex < data->size; byteindex = byteindex + 1) {
		// Clear any garbage memory from current byte
		data->data[byteindex] = 0;
		
		// Load in LSBs from next 8 bytes of raw input
		for(offset = 0; offset < 8; offset = offset + 1) {
			data->data[byteindex] = data->data[byteindex] | 
					(raw.data[(byteindex * 8) + offset] & 0x01) << offset;
		}
	}

	return 0;
} // Reconstitute Signal


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
	if(errno == ENOMEM) {
		return -2;
	}
	
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

	// Assemble final signal
	if((res = assembleSignal(&finalsig, sigparts)) < 0) {
		return res;
	}

	// Deconstitute signal in noise
	if((res = deconstitute(outfile, finalsig, sigparts[3])) < 0) {
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


// Decode
//
// Returns:    0 if successful
// 	      -2 if out of memory
int decode(FILE *file, uint64_t filesize, struct datastring *outfile) {

	return 0;
} // Decode
