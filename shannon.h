#ifndef _shannon_h_
#define _shannon_h_

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "frontend.h"

// Main
int main(int argc, char** argv);

// Data string
// Similar to a rudimentary string, but designed to carry arbitrary byte data
struct dataString {
	int size;
	char* data;
};

#endif
