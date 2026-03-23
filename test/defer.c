#define _CRT_SECURE_NO_WARNINGS
#define BLOAT_IMPLEMENTATION
#include "../bloat.h"

int main()
{
    string_builder_t sb = {0};
    FILE *f  = fopen("defer.txt", "wb");
    defer_ptr(fclose, f);

    for(int32_t i = 0; i < 20; i++)
        sb_append(&sb, "Hello world\n");

    fprintf(f, "%s\n", sb.items);

    defer_run();
    return 0;
}
