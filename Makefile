CC=gcc
CFLAGS=-Wall
DEBUGGER=gdb

all:
	$(CC) $(CFLAGS) banker.c

run:
	./a.out

clean:
	rm -f a.out *.o

debug:
	$(DEBUGGER) ./a.out

.PHONY: all run clean debug