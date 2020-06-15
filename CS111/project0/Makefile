default: lab0

lab0: lab0.o
	gcc -g -Wall -Wextra lab0.c -o lab0

lab0.o: lab0.c
	gcc -g -c lab0.c

dist:
	tar -cvzf lab0-105238261.tar.gz \
	lab0.c Makefile backtrace.png breakpoint.png README

check:
	temp_file=$(mktemp);  
	echo "012345" >> temp_file;
	echo "testing --input"; ./lab0 --input=temp_file;
	echo "testing --input --output"; ./lab0 --input=temp_file --output=out_file;
	diff temp_file out_file;
	echo "testing --segfault --catch... "; ./lab0 --segfault --catch
clean:
	rm -f  *.o *.tar.gz
