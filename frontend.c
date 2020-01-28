/*******
 * Shannon; Frontend
 * Becquerel Jones
 * December 30, 2019
 * Debian 10: Buster
 * Vim
*****/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "encryption.h"
#include "frontend.h"

// Main menu
int mainMenu(int argc, char **argv) {
	int res;
	int i;
	int scanned = -1;
	char **prompt = malloc(4 * sizeof(*prompt));
	for(i = 0; i < 4; i = i + 1) {
		prompt[i] = malloc(32 * sizeof(**prompt));
	}

	printf("\n\n***\nSHANNON\n*****\n");

	strcpy(prompt[0], "\n***\nMAIN MENU\n*****");
	strcpy(prompt[1], "[1]\tEncode");
	strcpy(prompt[2], "[2]\tDecode");
	strcpy(prompt[3], "[0]\tQuit");

	while(scanned) {
		//printf("\n[1]\tEncode\n[2]\tDecode\n[0]\tQuit\n\n");
		//scanf("%d", &scanned);
		while((res = userPrompt(prompt, 4, "%d", &scanned)));
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

	// Cleanup
	for(i = 0; i < 4; i = i + 1) {
		free(prompt[i]);
	}
	free(prompt);

	return 0;
}

// Encode menu
int encMenu() {
	printf("\n\n***\nENCODE\n*****\n");

	// Initialize variables
	int res;
	FILE **files = malloc(2 * sizeof(*files));
	uint64_t *filesizes = malloc(2 * sizeof(*filesizes));
	char fname[PATH_MAX];
	char path[PATH_MAX];
	struct datastring *infiles;
	struct datastring outfile;

	// Allocate space for files
	files = malloc(2 * sizeof(*files));
	if(errno == ENOMEM) {
		return -2;
	}

	// Allocate space for file sizes
	filesizes = malloc(2 * sizeof(*filesizes));
	if(errno == ENOMEM) {
		return -2;
	}

	// Allocate space for input files
	infiles = malloc(2 * sizeof(*infiles));
	if(errno == ENOMEM) {
		return -2;
	}

	// Select signal file
	printf("Signal File:\n");
	if((res = fileBrowse(SIGNADIR, &(files[0]), &(filesizes[0]), fname))) {
		return res;
	}

	// Select noise file
	printf("Noise File:\n");
	if((res = fileBrowse(NOISEDIR, &(files[1]), &(filesizes[1]), fname))) {
		return res;
	}

	// Allocate space for signal file
	infiles[0].size = filesizes[0];
	infiles[0].data = malloc(infiles[0].size * sizeof(*(infiles[0].data)));
	if(errno == ENOMEM) {
		return -2;
	}

	// Allocate space for noise file
	infiles[1].size = filesizes[1];
	infiles[1].data = malloc(infiles[1].size * sizeof(*(infiles[1].data)));
	if(errno == ENOMEM) {
		return -2;
	}

	// Read files
	fread(infiles[0].data, 1, infiles[0].size, files[0]);
	fread(infiles[1].data, 1, infiles[1].size, files[1]);

	// Hide signal file in noise file
	// encode() frees infile data
	if((res = encode(infiles, &outfile))) {
		return res;
	}

	// Export result
	if((res = constructPath(OUTBODIR, path))) {
		return res;
	}
	strcat(path, fname);
	if((res = exportFile(path, outfile))) {
		return res;
	}

	// Cleanup
	fclose(files[0]);
	fclose(files[1]);
	free(outfile.data);
	free(infiles);
	free(files);
	free(filesizes);
	
	return 0;
}

// Decode menu
int decMenu() {
	printf("\n\n***\nDECODE\n*****\n");

	// Initialize variables
	int res;
	FILE* file;
	uint64_t filesize;
	char path[PATH_MAX];
	struct datastring infile;
	struct datastring outfile;

	// Select encoded file
	if((res = fileBrowse(INBOXDIR, &file, &filesize, path))) {
		return res;
	}

	// Allocate space for input file
	infile.size = filesize;
	infile.data = malloc(infile.size * sizeof(*(infile.data)));
	if(errno == ENOMEM) {
		return -2;
	}

	// Read input file
	fread(infile.data, 1, infile.size, file);

	// Find signal in encoded file; save to outfile
	// Decode frees infile data
	if((res = decode(&infile, &outfile))) {
		return res;
	}
	
	// Export result
	if((res = constructPath(OUTBODIR, path))) {
		return res;
	}
	strcat(path, "message.txt");
	if((res = exportFile(path, outfile))) {
		return res;
	}

	// Cleanup
	fclose(file);
	free(outfile.data);

	return 0;
}

// User prompt
int userPrompt(char **prompt, int lines, const char *format, void *result) {
	int res = 0;
	int i;
	for(i = 0; i < lines; i = i + 1) {
		printf("%s\n", prompt[i]);
	}
	printf("USER>> ");
	if(!scanf(format, result)) {
		res = 7;
	}
	while(getchar() != '\n');
	return res;
}

// Construct path
int constructPath(char *fdir, char *path) {
	if(!getcwd(path, PATH_MAX)) {
		return 2;
	}
	strcat(path, "/");
	strcat(path, fdir);
	strcat(path, "/");
	return 0;
}

// File browser
int fileBrowse(char *fdir, FILE **file, uint64_t *filesize, char *fname) {

	// Variable declarations
	int res;
	int scanned = 0;
	FILE *tFile;
	uint64_t tSize;
	DIR *dir;
	struct dirent* ent;
	char *path = malloc(PATH_MAX);
	int i;
	int numents;
	struct stat dirtest = {0};
	char **menu;

	// Allocate memory for directory entries
	char** entries = malloc(64 * sizeof(*entries));
	for(i = 0; i < 64; i = i + 1) {
		entries[i] = malloc(PATH_MAX);
	}

	// Get path to directory
	if((res = constructPath(fdir, path))) {
		printf("ERROR: Invalid Path\n\"%s\", \"%s\"\n", fdir, path);
		return res;
	}

	// Ensure that directory exists
	if(!(dir = opendir(path))) {
		printf("ERROR: No Directory\n\"%s\"\n", path);
		return 3;
	}

	// Load directory contents
	printf("Loading Directory:\n\"%s\"\n", path);
	i = 0;
	while((ent = readdir(dir))) {
		if(ent->d_name[0] != '.') {
			strcpy(entries[i], ent->d_name);
			i = i + 1;
		}
	}
	// Ensure that directory has contents
	if(!i) {
		printf("ERROR: Empty Directory\n\"%s\"\n", path);
		return 3;
	}
	numents = i;

	// Generate menu
	char tpath[PATH_MAX];
	uint64_t tsize;
	menu = malloc((numents + 2) * sizeof(*menu));
	for(i = 0; i < numents + 2; i = i + 1) {
		menu[i] = malloc(PATH_MAX);
	}
	strcpy(menu[0], "\n***\nFILE MENU\n*****");
	for(i = 0; i < numents; i = i + 1) {
		strcpy(tpath, path);
		strcat(tpath, entries[i]);
		stat(tpath, &dirtest);
		tsize = dirtest.st_size;
		sprintf(menu[i + 1], "[%d]\t%s\t.\t.\t%lu B", i + 1, entries[i], tsize);
	}
	strcpy(menu[numents + 1], "[0]\tQuit");

	// Select file from directory (or Quit)
	while(scanned > -2) {
		while((res = userPrompt(menu, numents + 2, "%d", &scanned)));
		if(!scanned) {
			printf("Aborted by User.\n");
			return 1;
		}
		scanned = scanned - 1;
		if(scanned > -1 && scanned < numents) {
			strcpy(fname, entries[scanned]);
			strcat(path, entries[scanned]);
			scanned = -2;
		} else {
			scanned = 0;
		}
	}
	printf("\nFile selected:\n%s\n", path);

	// Ensure that file exists
	if(stat(path, &dirtest) == -1) {
		printf("ERROR: Invalid File\n\"%s\"", path);
		return 4;
	}

	// Check file size
	tSize = dirtest.st_size;

	// Actually open the file
	if(!(tFile = fopen(path, "r"))) {
		return 4;
	}


	// Cleanup
	closedir(dir);
	for(i = 0; i < numents + 2; i = i + 1) {
		free(menu[i]);
	}
	free(menu);
	for(i = 0; i < 64; i = i + 1) {
		free(entries[i]);
	}
	free(entries);
	free(path);

	*file = tFile;
	*filesize = tSize;

	return 0;
}

// Export file
int exportFile(char* path, struct datastring output) {
	FILE* outfile;

	if(!(outfile = fopen(path, "w"))) {
		return 6;
	}
	fwrite(output.data, sizeof(*(output.data)), output.size, outfile);
	fclose(outfile);

	return 0;
}


// Main
int main(int argc, char **argv) {
	return mainMenu(argc, argv);
} // Main
