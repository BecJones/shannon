#ifndef _encryption_h_
#define _encryption_h_

// Encode transmission
int encode(FILE** files, unsigned long* filesizes);

// Data string
struct dataString {
	unsigned long size;
	char* data;
};

#endif
