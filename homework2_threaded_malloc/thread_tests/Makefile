# Makefile for testing
#
# args:
# 	MALLOC_VERSION: LOCK_VERSION for locked version, NOLOCK_VERSION for non-locked version
#
# example: make MALLOC_VERSION=NOLOCK_VERSION

CC=gcc
CFLAGS=-O3
WDIR=../my_malloc

all: thread_test thread_test_malloc_free thread_test_malloc_free_change_thread thread_test_measurement

thread_test: thread_test.c
	$(CC) $(CFLAGS) -I$(WDIR) -D$(MALLOC_VERSION) -L$(WDIR) -Wl,-rpath=$(WDIR) -o $@ thread_test.c -lmymalloc -lrt -lpthread

thread_test_malloc_free: thread_test_malloc_free.c
	$(CC) $(CFLAGS) -I$(WDIR) -D$(MALLOC_VERSION) -L$(WDIR) -Wl,-rpath=$(WDIR) -o $@ thread_test_malloc_free.c -lmymalloc -lrt -lpthread

thread_test_malloc_free_change_thread: thread_test_malloc_free_change_thread.c
	$(CC) $(CFLAGS) -I$(WDIR) -D$(MALLOC_VERSION) -L$(WDIR) -Wl,-rpath=$(WDIR) -o $@ thread_test_malloc_free_change_thread.c -lmymalloc -lrt -lpthread

thread_test_measurement: thread_test_measurement.c
	$(CC) $(CFLAGS) -I$(WDIR) -D$(MALLOC_VERSION) -L$(WDIR) -Wl,-rpath=$(WDIR) -o $@ thread_test_measurement.c -lmymalloc -lrt -lpthread

clean:
	rm -f *~ *.o thread_test thread_test_malloc_free thread_test_malloc_free_change_thread thread_test_measurement

clobber:
	rm -f *~ *.o
