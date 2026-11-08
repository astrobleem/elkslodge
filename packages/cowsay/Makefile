ELKS_ROOT ?= $(HOME)/elks
TOOLCHAIN ?= $(ELKS_ROOT)/cross
CC = $(TOOLCHAIN)/bin/ia16-elf-gcc
CFLAGS = -Os -Wall -Wextra -mcmodel=small -melks-libc -march=i8086 -mtune=i8086 -mno-segment-relocation-stuff -fno-builtin-printf -fno-builtin-fprintf
CPPFLAGS = -I$(ELKS_ROOT)/libc/include -I$(ELKS_ROOT)/elks/include -I$(ELKS_ROOT)/include
all: cowsay elksay elkthink cowthink
cowsay: cowsay.c
	PATH="$(ELKS_ROOT)/elks/tools/bin:$$PATH" $(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $<
elksay elkthink cowthink: cowsay
	cp cowsay $@
host: cowsay.c
	cc -DHOST_TEST -std=c99 -Wall -Wextra -Werror -O2 -o cowsay-host $<
	cp cowsay-host elksay-host
	cp cowsay-host elkthink-host
	cp cowsay-host cowthink-host
.PHONY: all host
