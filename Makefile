CC=gcc
CFLAGS=-Wall -std=gnu11 -O0 -g
LDFLAGS=-fsanitize=address

.PHONY: all
all: read_a_byte read_a_string itertest sharing sharing_fixed

read_a_byte: read_a_byte.c
read_a_string: read_a_string.c
itertest: itertest.c

sharing: sharing.c utils.c
sharing_fixed: sharing_fixed.c utils.c

.PHONY: clean
clean:
	rm -f read_a_byte read_a_string cachegrind.* itertest sharing sharing_fixed
