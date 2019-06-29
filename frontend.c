#include "shannon.h"

// Main menu
int mainMenu(int argc, char** argv) {
	int res;
	int scanned = -1;

	printf("\n\n***\nSHANNON\n*****\n");

	while(scanned) {
		printf("\n[1]\tEncode\n[2]\tDecode\n[0]\tQuit\n\n");
		res = 0;
		scanf("%d", &scanned);
		if(scanned == 1) {
			res = encMenu();
		} else if(scanned == 2) {
			res = decMenu();
		}
		if(res) {
			printf("\nExit %d FAILURE\n\n", res);
			return res;
		}
	}

	printf("\nExit 0 SUCCESS\n\n");

	return 0;
}

// Encode menu
int encMenu() {
	printf("\n\n***\nENCODE\n*****\n");

	int res;
	FILE** files = malloc(2 * sizeof(*files));
	uint64_t* filesizes = malloc(2 * sizeof(*filesizes));
	char fname[PATH_MAX];
	struct dataString outfile;
	FILE* output;

	if((res = fileBrowse(SIGNADIR, &(files[0]), &(filesizes[0]), fname))) {
		return res;
	}
	if((res = fileBrowse(NOISEDIR, &(files[1]), &(filesizes[1]), fname))) {
		return res;
	}

	if((res = encode(files, filesizes, &outfile))) {
		return res;
	}

	if(!(output = fopen(fname, "w"))) {
		return 6;
	}
	fwrite(outfile.data, sizeof(*(outfile.data)), outfile.size, output);


	fclose(output);
	fclose(files[0]);
	fclose(files[1]);
	free(outfile.data);
	free(files);
	free(filesizes);
	
	return 0;
}

// Decode menu
int decMenu() {
	printf("\n\n***\nDECODE\n*****\n");

	int res;
	FILE* file;
	uint64_t filesize;
	char fname[PATH_MAX];
	struct dataString outfile;
	FILE* output;

	if((res = fileBrowse(INBOXDIR, &file, &filesize, fname))) {
		return res;
	}

	// Send the files to the backend

	fclose(file);

	return 0;
}

// File browser
int fileBrowse(char* fdir, FILE** file, uint64_t* filesize, char* fname) {

	// Variable declarations
	int scanned = -2;
	FILE* tFile;
	uint64_t tSize;
	DIR* dir;
	struct dirent* ent;
	char* path = malloc(PATH_MAX);
	int i;
	int numents;
	struct stat dirtest = {0};

	// Allocate memory for directory entries
	char** entries = malloc(64 * sizeof(*entries));
	for(i = 0; i < 64; i = i + 1) {
		entries[i] = malloc(PATH_MAX);
	}

	// Get working directory
	if(!getcwd(path, PATH_MAX)) {
		return 2;
	}
	strcpy(fname, path);

	// Construct complete directory path
	strcat(path, "/");
	strcat(path, fdir);
	strcat(path, "/");

	strcat(fname, "/");
	strcat(fname, OUTBODIR);
	strcat(fname, "/");

	if(stat(path, &dirtest) == -1) {
		mkdir(path, 0777);
	}

	printf("\nLoading Directory:\n%s\n", path);
	printf("Choose file:\n");

	// List files in directory
	if(!(dir = opendir(path))) {
		return 3;
	}
	
	i = 0;
	while((ent = readdir(dir))) {
		if(strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..") && ent->d_name[0] != '.') {
			strcpy(entries[i], ent->d_name);
			i = i + 1;
		}
	}
	if(!i) {
		return 3;
	}
	numents = i;

	for(i = 0; i < numents; i = i + 1) {
		printf("[%d]\t%s\n", i + 1, entries[i]);
	}
	printf("[0]\tQuit\n");

	// Select file from directory (or Quit)
	while(scanned != -1) {
		scanf("%d", &scanned);
		if(!scanned) {
			return 1;
		}
		scanned = scanned - 1;
		if(scanned > -1 && scanned < numents) {
			strcat(fname, entries[scanned]);
			strcat(path, entries[scanned]);
			scanned = -1;
		}
	}

	printf("\nFile selected:\n%s\n", path);

	if(stat(path, &dirtest) == -1) {
		return 4;
	}

	tSize = dirtest.st_size;

	// Actually open the file
	if(!(tFile = fopen(path, "r"))) {
		return 4;
	}


	// Clean up
	closedir(dir);
	for(i = 0; i < 64; i = i + 1) {
		free(entries[i]);
	}
	free(entries);
	free(path);

	*file = tFile;
	*filesize = tSize;

	return 0;
}
