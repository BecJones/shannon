#ifndef _frontend_h_
#define _frontend_h_

#define SIGNADIR "signal"
#define NOISEDIR "noise"
#define INBOXDIR "inbox"
#define OUTBODIR "output"

// Main menu
int mainMenu();

// Encode menu
int encMenu();

// Decode menu
int decMenu();

// Construct path
int constructPath(char* fdir, char* path);

// File browser
int fileBrowse(char* fdir, FILE** file, uint64_t* filesize, char* fname);

// Export file
int exportFile(char* path, struct dataString output);

#endif
