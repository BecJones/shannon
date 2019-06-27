build: shannon.c shannon.h frontend.c frontend.h
	gcc -o shannon shannon.c frontend.c

debug: shannon.c shannon.h frontend.c frontend.h
	gcc -o shannon -Wall shannon.c frontend.c

clean: shannon
	rm shannon
