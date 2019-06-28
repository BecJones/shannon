build: shannon.c shannon.h frontend.c frontend.h encryption.c encryption.h
	gcc -o shannon shannon.c frontend.c encryption.c
	mkdir -p ./signal
	mkdir -p ./noise
	mkdir -p ./inbox
	mkdir -p ./output

debug: shannon.c shannon.h frontend.c frontend.h encryption.c encryption.h
	gcc -o shannon -Wall shannon.c frontend.c encryption.c

clean:
	rm shannon signal/* noise/* inbox/*
