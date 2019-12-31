#ifndef _encryption_h_
#define _encryption_h_

#include <stdint.h>

#define KEY_0_LENGTH 5
#define KEY_1_LENGTH 11
#define KEY_2_LENGTH 12

#define MIN_OFFSET 214
#define MAX_OFFSET 413

#define HEADER_LENGTH 5

// Data string struct
struct datastring {
	uint64_t size;
	unsigned char *data;
};

// Initialize Header
int initHeader(struct datastring *sigparts);

// Initialize Keys
int initKeys(struct datastring *sigparts);

// Initialize Size
int initSize(struct datastring *sigparts);

// Load Signal Parts
int loadSigParts(struct datastring *sigparts);

// Get Offset
int getOffset(struct datastring *sigparts, uint64_t *offset,
		struct datastring data);

// Get Keys
int getKeys(struct datastring *sigparts, uint64_t *offset,
		struct datastring data);

// Get Size
int getSize(struct datastring *sigparts, uint64_t *offset,
		struct datastring data);

// Get Signal
int getSignal(struct datastring *sigparts, uint64_t *offset,
		struct datastring data);

// Apply Keys
int applyKeys(struct datastring *sigparts);

// Assemble Signal
int assembleSignal(struct datastring *finalsig, struct datastring *sigparts);

// Disassemble Signal
int disassembleSignal(struct datastring *sigparts, struct datastring data);

// Deconstitute Signal
int deconstitute(struct datastring *noise, struct datastring signal,
		struct datastring header);

// Reconstitute Signal
int reconstitute(struct datastring *data, struct datastring raw);

// Encode Transmission
int encode(FILE **files, uint64_t *filesizes, struct datastring *outfile);

// Decode Transmission
int decode(FILE *file, uint64_t filesize, struct datastring *outfile);

#endif
