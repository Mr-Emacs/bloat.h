all: forloop.exe generic_struct.exe struct_arena.exe string_builder.exe

forloop.exe: forloop.c
	clang.exe -Wall -Wextra -Wpedantic -o forloop.exe forloop.c -Wno-deprecated-declarations

generic_struct.exe: generic_struct.c
	clang.exe -Wall -Wextra -Wpedantic -o generic_struct.exe generic_struct.c -Wno-deprecated-declarations

struct_arena.exe: struct_arena.c
	clang.exe -Wall -Wextra -Wpedantic -o struct_arena.exe struct_arena.c -Wno-deprecated-declarations

string_builder.exe: string_builder.c
	clang.exe -Wall -Wextra -Wpedantic -o string_builder.exe string_builder.c -Wno-deprecated-declarations
