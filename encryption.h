/*******
 * Shannon
 * Developer: Becquerel Jones
 * Last Updated: September 14, 2019
 * OS: WSL Ubuntu on Windows 10
*****/

#ifndef _encryption_h_
#define _encryption_h_

#define KEY_0_LENGTH 5
#define KEY_1_LENGTH 11
#define KEY_2_LENGTH 12
#define MAX_OFFSET 413
#define HEADER_LENGTH 5

// Data string
struct dataString {
	uint64_t size;
	unsigned char* data;
};

// Encode transmission
int encode(FILE** files, uint64_t* filesizes, struct dataString* outfile);

// Initialize keys
int initKeys(struct dataString* sigcomp);

// Apply keys to raw signal
int applyKeys(struct dataString* sigcomp);

// Build final signal
int buildFinalSignal(struct dataString* finalsig, struct dataString* sigcomp);

// Hide signal in noise
int insertSignal(struct dataString* output, struct dataString* signal, struct dataString* noise, unsigned char* header);

#endif
