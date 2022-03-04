CC=g++
CFLAGS=-O3
EXTRAFLAGS=-lpqxx -lpq
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp, %.o, $(SRCS))

all: test

test: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(EXTRAFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $< $(EXTRAFLAGS)

.PHONY: clean clobber
clean:
	rm -f *~ *.o *.zip test

clobber:
	rm -f *~ *.o

main.o: exerciser.h exception.h
exerciser.o: query_funcs.h