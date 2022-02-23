CC=g++
CFLAGS=-std=c++11 

all: ringmaster player

ringmaster: ringmaster.o server.o potato.o clientInfo.o
	$(CC) $(CFLAGS) -o $@ ringmaster.o server.o potato.o clientInfo.o

player: player.o server.o potato.o clientInfo.o
	$(CC) $(CFLAGS) -o $@ player.o server.o potato.o clientInfo.o

server: server.o potato.o clientInfo.o
	$(CC) $(CFLAGS) -o $@ server.o potato.o clientInfo.o

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean 
clean:
	rm -rf *.o *~ ringmaster player

server.o: server.h potato.h clientInfo.h
ringmaster.o: server.h ringmaster.h potato.h
player.o: server.h player.h potato.h
potato.o: potato.h
