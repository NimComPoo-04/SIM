CC = gcc
CFLAGS = -Wall -Wextra -ggdb -O0 -std=c11

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, obj/%.o, $(SRC))

all: build asm

build:
	@mkdir -p obj

asm: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

obj/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf obj asm
