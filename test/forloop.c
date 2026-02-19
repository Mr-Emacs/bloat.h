#define BLOAT_IMPLEMENTATION
#include "../bloat.h"

int main(void) {
  arena_t *arena = arena_alloc();

  array bar = {0};

  for (size_t i = 0; i < 10; i++) {
    da_append_arena_sized(arena, &bar, &i);
  }

  foreach (item, &bar) {
    printf("%zu ", cast(i, item));
  }

  printf("\n");

  arena_free(arena);
}
