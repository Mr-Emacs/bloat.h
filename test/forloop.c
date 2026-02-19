#define BLOAT_IMPLEMENTATION
#include "../bloat.h"

int main(void) {
  arena_t *arena = arena_alloc();

  array bar = {0};

  for (size_t i = 0; i < 10; i++) {
    da_append_arena(arena, &bar, &i, sizeof(i));
  }

  foreach (item, &bar) {
    printf("%d\n", *(int *)item);
  }

  arena_free(arena);
}
