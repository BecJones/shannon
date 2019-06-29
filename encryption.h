#ifndef _encryption_h_
#define _encryption_h_

#define KEY_0_LENGTH 5
#define KEY_1_LENGTH 11
#define KEY_2_LENGTH 12
#define SIG_OFFSET 113
#define HEADER_LENGTH 5

// Encode transmission
int encode(FILE** files, uint64_t* filesizes);

// Initialize keys
int initKey(unsigned char** key);

// Data string
struct dataString {
	uint64_t size;
	unsigned char* data;
};

#endif
