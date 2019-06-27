build: shannon.c shannon.h frontend.c frontend.h encryption.c encryption.h
	gcc -o shannon shannon.c frontend.c encryption.c

debug: shannon.c shannon.h frontend.c frontend.h encryption.c encryption.h
	gcc -o shannon -Wall shannon.c frontend.c encryption.c

clean: shannon
	rm shannon
