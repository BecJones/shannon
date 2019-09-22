/*******
 * Shannon
 * Developer: Becquerel Jones
 * Last Updated: September 22, 2019
 * OS: WSL Ubuntu on Windows 10
*****/

#ifndef _decryption_h_
#define _decryption_h_

// Decode transmission
int decode(FILE* file, uint64_t filesize, struct dataString* outfile);

// Reconstitute data
int reconstitute(struct dataString data, struct dataString* recd);

// Get offset
int getOffset(struct dataString recd, uint64_t* offset);

// Get transmission length
int getLength(struct dataString sigcomp, uint64_t* length);

// Get keys
int getKeys(struct dataString sigcomp, struct dataString* keys);

// Get signal
int getSignal(struct dataString sigcomp, struct dataString* finalsig);

#endif
