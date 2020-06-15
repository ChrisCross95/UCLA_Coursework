default: netconfig lab1b-server lab1b-client

netconfig:
	./netconfig.sh

lab1b-client: lab1b-client.o
	gcc -Wall -Wextra -pthread -lz lab1b-client.o -o lab1b-client

lab1b-server: lab1b-server.o
	gcc -Wall -Wextra -pthread -lz lab1b-server.o -o lab1b-server

lab1b-server.o: lab1b-server.c netdefs.h
	gcc -Wall -g -c lab1b-server.c

lab1b-client.o: lab1b-client.c netdefs.h
	gcc -Wall -g -c lab1b-client.c
	
dist:
	tar -cvzf lab1b-105238261.tar.gz \
	lab1b-client.c lab1b-server.c netconfig.sh netdefs.h \
	README Makefile
	
clean:
	rm -f lab1b-server lab1b-client *.o sdef
	> netdefs.h
