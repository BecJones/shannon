#include "shannon.h"

// Encode transmission
int encode(FILE** files, uint64_t* filesizes, struct dataString* outfile) {
	// Initialize variables
	int res;
	uint64_t i;
	uint64_t j;
	unsigned char** key = malloc(3 * sizeof(*key));
	unsigned char* header = malloc(HEADER_LENGTH * sizeof(*header));
	unsigned char* sigsize;

	struct dataString* data = malloc(2 * sizeof(*data));
	struct dataString finalsig;
	struct dataString* sigcomp = malloc(6 * sizeof(*sigcomp));

	// Allocate memory for file contents
	data[0].size = filesizes[0];
	data[0].data = malloc(data[0].size * sizeof(*(data[0].data)));
	data[1].size = filesizes[1];
	data[1].data = malloc(data[1].size * sizeof(*(data[1].data)));

	// Ensure that noise file is large enough to contain signal file contents
	if((data[1].size - MAX_OFFSET) / 8 < data[0].size +
			KEY_0_LENGTH + KEY_1_LENGTH + KEY_2_LENGTH +
			HEADER_LENGTH + sizeof(data[0].size)) {
		return 5;
	}

	// Read files
	fread(data[0].data, sizeof(*(data[0].data)), data[0].size, files[0]);
	fread(data[1].data, sizeof(*(data[1].data)), data[1].size, files[1]);

	printf("\nSignal size: %lu\nNoise size: %lu\n",
			data[0].size, data[1].size);

	// Initialize sizes for all final signal components
	sigcomp[0].size = KEY_0_LENGTH;
	sigcomp[1].size = KEY_1_LENGTH;
	sigcomp[2].size = KEY_2_LENGTH;
	sigcomp[3].size = sizeof(data[0].size);
	sigcomp[4].size = HEADER_LENGTH;
	sigcomp[5].size = data[0].size;

	//Generate keys (signal components 0-2)
	if((res = initKeys(sigcomp))) {
		return res;
	}

	// Allocate/assign message size string
	for(i = 0; i < sigcomp[3].size; i = i + 1) {
		sigsize[i] = (data[0].size >> i) & 0xFF;
	}

	// Generate header
	for(i = 0; i < sigcomp[4].size; i = i + 1) {
		sigcomp[4].data[i] = 0xFF - i;
	}

	// Copy raw signal to signal component
	strcpy(sigcomp[5].data, data[0].data);

	// Encode raw signal with keys
	if((res = applyKeys(sigcomp))) {
		return res;
	}

	// Concatenate final signal
	if((res = buildFinalSignal(&finalsig, sigcomp))) {
		return res;
	}

	// Prepare output struct
	outfile->size = data[1].size;
	outfile->data = malloc(outfile->size);

	// Hide signal in noise
	if((res = insertSignal(outfile, &finalsig, &(data[1]), sigcomp[4].data))) {
		return res;
	}

	// Cleanup
	for(i = 0; i < 6; i = i + 1) {
		free(sigcomp[i].data);
	}
	free(sigcomp);
	free(data[0].data);
	free(data[1].data);
	free(data);
	free(finalsig.data);

	return 0;
}

// Initialize keys
int initKeys(struct dataString* sigcomp) {
	// Initialize variables
	int keynum;
	int i;
	int shiftnum;
	unsigned int tRand;

	// Seed random number generation
	srand(time(0));

	// Generate keys
	for(keynum = 0; keynum < 3; keynum = keynum + 1) {
		tRand = rand();
		shiftnum = 0;
		for(i = 0; i < sigcomp[keynum].size; i = i + 1) {
			sigcomp[keynum].data[i] = tRand & 0xFF;
			shiftnum = shiftnum + 1;
			if(shiftnum >= sizeof(tRand)) {
				tRand = rand();
				shiftnum = 0;
			}
		}
	}

	return 0;
}

// Apply keys to raw signal
int applyKeys(struct dataString* sigcomp) {
	uint64_t i;
	int j;

	for(i = 0; i < sigcomp[5].size; i = i + 1) {
		for(j = 0; j < 3; j = j + 1) {
			sigcomp[5].data[i] = sigcomp[5].data[i] ^ sigcomp[j].data[i % sigcomp[j].size];
		}
	}

	return 0;
}

// Build final signal
int buildFinalSignal(struct dataString* finalsig, struct dataString* sigcomp) {
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
	for(i = offset; i < offset + signal->size; i = i + 1) {
		output->data[i] = (output->data[i] & 0xFE) | 
			((signal->data[bytenum] >> bitnum) & 0x01);
		bitnum = bitnum + 1;
		if(bitnum >= 7) {
			bitnum = 0;
			bytenum = bytenum + 1;
		}
	}

	// Cleanup
	free(headercheck);

	return 0;
}
