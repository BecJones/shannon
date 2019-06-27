#ifndef _encryption_h_
#define _encryption_h_

// Encode transmission
int encode(FILE** files);

// Data string
struct dataString {
	int size;
	char* data;
};

#endif
