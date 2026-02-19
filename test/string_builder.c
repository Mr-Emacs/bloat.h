#define BLOAT_IMPLEMENTATION
#include "../bloat.h"

int main(void) {
  arena_t *arena = arena_alloc();

  string_builder_t *sb = arena_push(arena, sizeof(string_builder_t));
  char *text = "Ur mom";

  sb_append(sb, text);

  bloat_log(BLOAT_INFO, "%s %s", "Created string builder and appended", text);
  printf("String builder: %s\n", sb->items);

  arena_free(arena);
  return 0;
}
