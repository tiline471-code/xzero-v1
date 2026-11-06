CC = clang
CFLAGS = -Wall -Wextra -O2 -I../COMMON

all: kvm

kvm:
	$(CC) $(CFLAGS) main.c kvm.c stack.c ../COMMON/bytecode.c -o kvm

clean:
	rm -f kvm
