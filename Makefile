CC=gcc
LD=gcc
CFLAGS=-Wall -Wextra -O2

TARGET=elfmodify

.PHONY: all clean

all: $(TARGET)

elfmodify: elfmodify.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) $(TARGET)
