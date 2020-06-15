CC=g++ -std=c++17
CFLAGS=-I.
DEPS=
OBJ=server.o
USERID=105238261

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: server 

server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.o server client *.tar.gz

dist: tarball
tarball: clean
	tar -cvzf /tmp/$(USERID).tar.gz --exclude=./.vagrant . && mv /tmp/$(USERID).tar.gz .
