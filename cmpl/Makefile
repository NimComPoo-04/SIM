CC = gcc
CFLAGS = -Wall -Wextra -ggdb -std=c11 -O0

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, obj/%.o, $(SRC))

all: build cmpl

build:
	mkdir -p obj

cmpl: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

obj/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf obj cmpl
