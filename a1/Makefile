CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -g
LIB=-lSDL3 -lSDL3_ttf

# in this case, .PHONY ensures "make all" would always execute
# without it, "make all" wouldn't execute if a file named "all" exist
# usually optional, likely to work without it
# common phony: all, install, clean, distclean, TAGS, info, check
.PHONY: all
all: a.out

# $@ refers to the filename produced (a.out in this case)
# $^ refers to all the files after :
a.out: main.o sdl2_gfx.o
	${CC} ${CFLAGS} -o $@ $^ ${LIB}

main.o: main.c
	gcc ${CFLAGS} -c $^ -o $@

sdl2_gfx.o: sdl2_gfx.c
	gcc ${CFLAGS} -c $^ -o $@
	
.PHONY: clean
clean:
	rm -f *.o a
