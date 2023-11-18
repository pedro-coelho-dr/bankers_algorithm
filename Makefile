CC=gcc
CFLAGS=-Wall
DEBUGGER=gdb
TARGET=banker

all: $(TARGET)

$(TARGET): banker.c
	$(CC) $(CFLAGS) -o $(TARGET) banker.c

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o

debug: $(TARGET)
	$(DEBUGGER) ./$(TARGET)

.PHONY: all run clean debug