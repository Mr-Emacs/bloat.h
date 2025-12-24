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

    struct Foo *foo = arena_push(arena, sizeof(struct Foo));
    foo->name = "Hello";
    foo->age = 17;
    printf("%s\n", foo->name);
    printf("%d\n", foo->age);

    bloat_log(BLOAT_INFO, "%d", foo->age);

    arena_free(arena);
    return 0;
}
