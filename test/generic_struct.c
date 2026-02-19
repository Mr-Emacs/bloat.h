#define BLOAT_IMPLEMENTATION
#include "../bloat.h"

#define POS 100

typedef struct {
  int x, y, w, h;
} rectangle;

int main(void) {
  arena_t *arena = arena_alloc();
  array arr = {0};

  rectangle rect = {
      .x = 0,
      .y = 0,
      .w = 100,
      .h = 100,
  };

  for (size_t i = 0; i < 10; i++) {
    da_append_arena_sized(arena, &arr, &rect);
    rect.x += POS;
    rect.y += POS;
    rect.w += i;
    rect.h += i;
  }

  foreach (item, &arr) {
    printf("Rectangle x:%d, y:%d, w:%d, h:%d\n", cast(rect, item).x,
           cast(rect, item).y, cast(rect, item).w, cast(rect, item).h);
  }

  arena_free(arena);
}
