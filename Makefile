CFLAGS=-Wall -g -std=c11 -Wpedantic -Werror -D_POSIX_C_SOURCE=200809L
all: hello ex3 ex4 ex16 ex17

clean:
	rm -f hello ex3 ex4 ex16 ex17

