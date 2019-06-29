build: shannon.c shannon.h frontend.c frontend.h encryption.c encryption.h
	gcc -o shannon shannon.c frontend.c encryption.c
	mkdir -p signal noise inbox output

debug: shannon.c shannon.h frontend.c frontend.h encryption.c encryption.h
	gcc -o shannon -Wall shannon.c frontend.c encryption.c

clean:
	rm -f shannon signal/* noise/* inbox/*
