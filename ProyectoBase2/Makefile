CC = gcc
CFLAGS = -g -Wall -fexec-charset=UTF-8 -I includes -I libs
LDFLAGS = -lws2_32
SRC_SOURCES = $(wildcard sources/*.c)
SRC_LIBS = $(wildcard libs/*.c)
OBJ = $(patsubst sources/%.c, obj/%.o, $(SRC_SOURCES)) $(patsubst libs/%.c, obj/%.o, $(SRC_LIBS))

all: dirs build/main

dirs:
	mkdir -p obj build includes libs

build/main: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o build/main $(LDFLAGS)

obj/%.o: sources/%.c
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.o: libs/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/*.o build/main