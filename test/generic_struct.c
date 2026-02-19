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
    da_append_arena(arena, &arr, &rect, sizeof(rect));
    rect.x += POS;
    rect.y += POS;
    rect.w += i;
    rect.h += i;
  }

  foreach (item, &arr) {
    rectangle rec = *(rectangle *)item;
    printf("Rectnagle x:%d, y:%d, w:%d, h:%d\n", rec.x, rec.y, rec.w, rec.h);
  }

  arena_free(arena);
}
