default: lab1a

labla: lab1a.o
	gcc -Wall -Wextra lab1a.o -o lab1a

lab1a.o: lab1a.c
	gcc -Wall -g -c lab1a.c

dist:
	tar -cvzf lab1a-105238261.tar.gz \
	lab1a.c README Makefile

clean:
	rm lab1a *.o *tar.gz 
