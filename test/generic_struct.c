#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#define BLOAT_IMPLEMENTATION
#include "../bloat.h"

#define POS 100

typedef struct {
  int x, y, w, h;
} rectangle;

int main(void)
{
  arena_t *arena = arena_alloc();
  if(!arena) {
      printf("ABOBA URMOM");
      return 1;
  }
  array arr = {0};

  rectangle rect = {
      .x = 0,
      .y = 0,
      .w = 100,
      .h = 100,
  };

  for (size_t i = 0; i < 10; i++) {
    da_append_arena_sized(arena, &arr, rect);
    rect.x += POS;
    rect.y += POS;
    rect.w += i;
    rect.h += i;
  }

  foreach (item, &arr) {
      rectangle rec = *(rectangle *)(item);
      printf("X:%d Y:%d W:%d H:%d\n", rec.x, rec.y, rec.w, rec.h);
  }

  arena_free(arena);
}
