CC=gcc
CFLAGS=-Wall
DEBUGGER=gdb

all:
	$(CC) $(CFLAGS) -o banker banker.c

run:
	./banker

clean:
	rm -f banker *.o
	rm -f result.txt

debug:
	$(DEBUGGER) ./banker

diff:
	diff result.txt expected.txt

.PHONY: all run clean debug diff