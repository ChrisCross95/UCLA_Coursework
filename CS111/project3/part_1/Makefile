# NAME: Christopher Cross, Nikita Siahaan
# EMAIL: cc28@alumni.princeton.edu, niksiahaan@ucla.edu
# ID: 105238261, 405150445

CFLAGS = -g -Wall -Wextra
TARFILES = lab3a.c Makefile ext2_fs.h Makefile README
# ID = 405150445 #or your id, whichever works
ID = 105238261

default:
	gcc $(CFLAGS) lab3a.c -o lab3a

dist:
	tar -czf lab3a-$(ID).tar.gz $(TARFILES)

clean:
	rm -f *.o *.tar.gz

