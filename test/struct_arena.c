#define BLOAT_IMPLEMENTATION
#include "../bloat.h"

struct Foo {
  char *name;
  int age;
};

int main(void) {
  arena_t *arena = arena_alloc();

  struct Foo *foo = arena_push(arena, sizeof(struct Foo));

  foo->name = "Hello world";
  foo->age = 20;

  printf("Foo struct: %s %d\n", foo->name, foo->age);

  arena_free(arena);
  return 0;
}
