CC = gcc
CFLAGS = -Wall -Wextra -ggdb -std=c99 -O0

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, obj/%.o, $(SRC))

.PHONY: all build clean

all: build emu

build:
	mkdir -p obj

emu: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

obj/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf obj emu
