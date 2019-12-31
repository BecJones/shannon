#######
# Shannon; Makefile
# Becquerel Jones
# December 30, 2019
# Debian 10: Buster
# Vim
#####

build: frontend.c encryption.c
	gcc -o shannon frontend.c encryption.c
	mkdir -p signal noise inbox output

debug: frontend.c encryption.c
	gcc -Wall -o shannon frontend.c encryption.c

clean:
	rm -rf shannon ignal noise inbox output
