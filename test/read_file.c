#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#define BLOAT_IMPLEMENTATION
#include "../bloat.h"

int main()
{
    FILE *f = fopen(__FILE__, "rb");
    defer_ptr(fclose, f);

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buffer = malloc(fsize);
    defer_ptr(free, buffer);

    size_t byte = fread(buffer, 1, fsize, f);

    string_view_t sv = cstr_to_str_s(buffer, byte);
    while (sv.size > 0) {
        size_t i = sv_trim_by_del(&sv, '\n');
        string_view_t line = { .data = sv.data, .size = i };
        sv_trim_fn(&line, isspace);

        printf("|"SV_fmt"|\n", SV_Args(line));

        sv.data += i;
        sv.size -= i;
        if (sv.size > 0 && sv.data[0] == '\n')
            sv_chop_left(&sv);
    }
}
