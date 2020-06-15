


default: lab4b

lab4b: lab4b.c
	gcc -std=c11 -Wall -Wextra lab4b.c -o lab4b -lmraa -lm -lpthread

check: lab4b
	echo OFF | ./lab4b --log=temp --period=1 --scale=F
	rm -f temp

clean:
	-rm -f *.gz lab4b 

dist: 
	tar -czvf lab4b-105238261.tar.gz lab4b.c \
	README Makefile 
