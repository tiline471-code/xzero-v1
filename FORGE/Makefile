CC = clang
CFLAGS = -Wall -Wextra -O2 -I../COMMON

all: forge

forge:
	$(CC) $(CFLAGS) src/main.c src/raster.c src/frame.c src/anvil.c ../COMMON/bytecode.c -o forge

clean:
	rm -f forge
