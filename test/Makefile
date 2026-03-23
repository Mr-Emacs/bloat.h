ifeq ($(OS),Windows_NT)
	CC := clang.exe
else
	CC := clang
endif

CFLAGS = -Wall -Wextra -Wpedantic -std=c23 -Wno-unsequenced
SRCS   = $(wildcard *.c)
EXES   = $(SRCS:.c=.exe)

all: $(EXES)

%.exe: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	del *.exe
