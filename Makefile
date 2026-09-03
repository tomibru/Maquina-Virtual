CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)
TARGET = bin/vm

all: $(TARGET)

$(TARGET): $(OBJ) | bin
	$(CC) $(CFLAGS) -o $@ $(OBJ)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

bin:
	mkdir -p bin

build:
	mkdir -p build

clean:
	rm -rf build bin

.PHONY: all clean
