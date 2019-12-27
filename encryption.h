#ifndef _encryption_h_
#define _encryption_h_

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

// Encode transmission
int encode(FILE **files, uint64_t *filesizes, struct datastring *outfile);

// Decode transmission
int decode(FILE *file, uint64_t filesize, struct datastring *outfile);

// Load Signal Parts
int loadSigParts(struct datastring *sigparts);

// Initialize header
int initHeader(struct datastring *sigparts);

// Initialize keys
int initKeys(struct datastring *sigparts);

// Apply keys to raw signal
int applyKeys(struct dataString *sigcomp);

// Build final signal
int buildFinalSignal(struct dataString *finalsig, struct dataString *sigcomp);

// Hide signal in noise
int insertSignal(struct dataString *output, struct dataString *signal, struct dataString *noise, unsigned char *header);

// Reconstitute data
int reconstitute(struct dataString data, struct dataString *recd);

// Get offset
int getOffset(struct dataString recd, uint64_t *offset);

// Get transmission length
int getLength(struct dataString sigcomp, uint64_t *length);

// Get keys
int getKeys(struct dataString sigcomp, struct dataString *keys);

// Get signal
int getSignal(struct dataString sigcomp, struct dataString *finalsig);

#endif
