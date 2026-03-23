#define _CRT_SECURE_NO_WARNINGS
#define BLOAT_IMPLEMENTATION
#include "../bloat.h"

int main(void)
{
  arena_t *arena = arena_alloc();

  string_builder_t *sb =  arena_push(arena, sizeof(string_builder_t));
  sb_append(sb, "Ur mom");

  bloat_log(BLOAT_INFO, "%s %s", "Created string builder and appended", "Ur mom");
  printf("String builder: %s\n", sb->items);

  arena_free(arena);
  return 0;
}
