#include <stdio.h>

#define BLOAT_IMPLEMNTATION
#include "bloat.h"

struct Foo {
    char *name;
    int age;
};

int main(void)
{
    arena_t *arena = arena_alloc();

    string_builder_t *sb = arena_push(arena, sizeof(string_builder_t));

    sb_append(sb, "Ur mom");
    printf("%s\n", sb->items);

    free(arena);
    return 0;
}
