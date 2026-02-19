CFLAGS = -Wall -Wextra -Wpedantic -Wno-deprecated-declarations -std=c23 -Wno-unsequenced

all: forloop.exe generic_struct.exe struct_arena.exe string_builder.exe

forloop.exe: forloop.c
	clang.exe $(CFLAGS) -o forloop.exe forloop.c
generic_struct.exe: generic_struct.c
	clang.exe $(CFLAGS) -o generic_struct.exe generic_struct.c

struct_arena.exe: struct_arena.c
	clang.exe $(CFLAGS) -o struct_arena.exe struct_arena.c

string_builder.exe: string_builder.c
	clang.exe $(CFLAGS) -o string_builder.exe string_builder.c
