#include "shannon.h"

// Encode transmission
int encode(FILE** files, unsigned long* filesizes) {
	// Initialize variables
	int res;
	unsigned char** key = malloc(3 * sizeof(*key));
	struct dataString* data = malloc(2 * sizeof(*data));

	// Allocate memory for file contents
	data[0].size = filesizes[0];
	data[0].data = malloc(data[0].size * sizeof(*(data[0].data)));
	data[1].size = filesizes[1];
	data[1].data = malloc(data[1].size * sizeof(*(data[1].data)));

	// Ensure that noise file is large enough to contain signal file contents
	if(data[1].size / 8 < data[0].size +
			KEY_0_LENGTH + KEY_1_LENGTH + KEY_2_LENGTH +
			HEADER_LENGTH + sizeof(data[0].size) + SIG_OFFSET) {
		return 6;
	}

	// Read files
	fread(data[0].data, sizeof(*(data[0].data)), data[0].size, files[0]);
	fread(data[1].data, sizeof(*(data[1].data)), data[1].size, files[1]);

	printf("\nSignal size: %lu\nNoise size: %lu\n",
			data[0].size, data[1].size);
	
	// Allocate memory for keys
	key[0] = malloc(KEY_0_LENGTH * sizeof(*(key[0])));
	key[1] = malloc(KEY_1_LENGTH * sizeof(*(key[1])));
	key[2] = malloc(KEY_2_LENGTH * sizeof(*(key[2])));

	//Generate keys
	if((res = initKey(key))) {
		return res;
	}

	// Cleanup
	free(key[0]);
	free(key[1]);
	free(key[2]);
	free(key);
	free(data[0].data);
	free(data[1].data);
	free(data);

	return 0;
}

// Initialize keys
int initKey(unsigned char** key) {
	// Initialize variables
	int i;
	int j;
	unsigned int tRand;

	// Seed random number generation
	srand(time(0));

	// Key 0
	tRand = rand();
	j = 0;
	for(i = 0; i < KEY_0_LENGTH; i = i + 1) {
		key[0][i] = tRand & 0xFF;
		tRand = tRand >> 8;
		j = j + 1;
		if(j == sizeof(tRand)) {
			tRand = rand();
			j = 0;
		}
	}

	// Key 1
	tRand = rand();
	j = 0;
	for(i = 0; i < KEY_1_LENGTH; i = i + 1) {
		key[1][i] = tRand & 0xFF;
		tRand = tRand >> 8;
		j = j + 1;
		if(j == sizeof(tRand)) {
			tRand = rand();
			j = 0;
		}
	}

	// Key 2
	tRand = rand();
	j = 0;
	for(i = 0; i < KEY_0_LENGTH; i = i + 1) {
		key[2][i] = tRand & 0xFF;
		tRand = tRand >> 8;
		j = j + 1;
		if(j == sizeof(tRand)) {
			tRand = rand();
			j = 0;
		}
	}

	return 0;
}
