#define BLOAT_IMPLEMENTATION
#include "../bloat.h"

int main()
{
    void *buffer = malloc(256);
    defer_ptr(free, buffer);

    for(int32_t i = 0; i < 20; i++)
        printf("Hello world %d\n", i);

    defer_run();
    return 0;
}
