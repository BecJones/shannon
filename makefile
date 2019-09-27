#######
# Shannon
# Developer: Becquerel Jones
# Last Updated: September 14, 2019
# OS: WSL Ubuntu on Windows 10
#####

build: shannon.c shannon.h frontend.c frontend.h encryption.c encryption.h decryption.c decryption.h
	gcc shannon.c frontend.c encryption.c decryption.c -o shannon
	mkdir -p signal noise inbox output

build-w: shannon.c shannon.h frontend.c frontend.h encryption.c encryption.h
	x86_64-w64-mingw32-gcc shannon.c frontend.c encryption.c decryption.c -o shannon.exe
	mkdir -p signal noise inbox output

debug: shannon.c shannon.h frontend.c frontend.h encryption.c encryption.h
	gcc -Wall shannon.c frontend.c encryption.c decryption.c -o shannon

clean:
	rm -rf shannon shannon.exe signal noise inbox output
