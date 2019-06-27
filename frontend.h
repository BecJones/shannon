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

// File browser
int fileBrowse(char* fdir, FILE** files);

#endif
