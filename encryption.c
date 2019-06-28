#include "shannon.h"

// Encode transmission
int encode(FILE** files, unsigned long* filesizes) {
	struct dataString* data = malloc(2 * sizeof(*data));

	data[0].size = filesizes[0];
	data[0].data = malloc(data[0].size * sizeof(*(data[0].data)));
	data[1].size = filesizes[1];
	data[1].data = malloc(data[1].size * sizeof(*(data[1].data)));

	if(data[1].size / 8 < data[0].size) {
		return 6;
	}

	fread(data[0].data, sizeof(*(data[0].data)), data[0].size, files[0]);
	fread(data[1].data, sizeof(*(data[1].data)), data[1].size, files[1]);

	printf("\nSignal size: %lu\nNoise size: %lu\n",
			data[0].size, data[1].size);

	free(data[0].data);
	free(data[1].data);
	free(data);

	return 0;
}
