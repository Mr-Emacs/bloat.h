/* Bloat.h

  My helper library of reusable utilities in C.

  Copyright (C) 2026 xsoder

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  See the LICENSE file for the full license text.
*/

/*
Version note: 0.1.3:
    - Addition of generic dynamic array structure `array`.
    - Has the follwing functions revolving around it.
      - da_append_s(): Memory leak because we can not properly clean it up.
      - da_append_arena(): Accepts arena allocator and the arena manages its
        life cycle.
      - da_free() frees the array if its memory is not managed by the arena.
    - Introduced foreach macro in C to take item and the dynamic array `array`.

Version note: 0.1.2:
    - Rewrote arena internals to use a linked-list of fixed chunks:
        - arena_push() no longer calls realloc, so returned pointers are
          stable for the lifetime of the arena.
        - Added arena_chunk_t (internal); arena_t public fields unchanged
          at the API level.
    - temp_arena_t now stores a chunk pointer + saved pos instead of a
      flat position, so rollback works correctly across chunk boundaries.
    - Fixed arena_pop_to() subtraction sign error from 0.1.1.

Version note: 0.1.1:
    - Introduced temporary arenas:
        - temp_arena_alloc()
        - temp_arena_free()
        - temp_arena_push()
        - temp_arena_pop()
    - Introduced SCOPED_TEMP_ARENA macro:
        - meant for temp arenas to live and die
        - Also updated the docs for better usage. Still imperfect though.

Version note: 0.1.0:
    - Introduced arenas:
        - arena_alloc();
        - arena_free();
        - arena_push();
        - arena_push_zero();
        - arena_pop();
        - arena_pop_to();
        - arena_clear();

    - Introduced string builder:
        - string_builder_append();
*/

#ifndef BLOAT_H
#define BLOAT_H

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAPACITY 1024
#define ARENA_CHUNK_SIZE (1024 * 64)

typedef enum {
  BLOAT_WARN,
  BLOAT_ERROR,
  BLOAT_INFO,
} BLOAT_LOG;

/* NOTE: internal chunk — never exposed directly by the API */
typedef struct arena_chunk_t {
  struct arena_chunk_t *next;
  uint64_t pos;
  uint64_t capacity;
  uint8_t data[];
} arena_chunk_t;

typedef struct {
  arena_chunk_t *head;
  arena_chunk_t *current;
} arena_t;

typedef struct {
  char *items;
  size_t count;
  size_t capacity;
} string_builder_t;

// NOTE: Functions below are related to arena
arena_t *arena_alloc(void);
void arena_free(arena_t *);

void *arena_push(arena_t *, uint64_t);
void *arena_push_zero(arena_t *, uint64_t);

void arena_pop(arena_t *, uint64_t);
void arena_pop_to(arena_t *, uint64_t);
void arena_clear(arena_t *);

typedef struct {
  arena_t *arena;
  arena_chunk_t *saved_chunk; /* NOTE: which chunk was current at push time */
  uint64_t saved_pos;         /* NOTE: pos inside that chunk at push time   */
} temp_arena_t;

temp_arena_t *temp_arena_alloc(arena_t *parent_arena);
void temp_arena_free(temp_arena_t *temp_arena);
void *temp_arena_push(temp_arena_t *temp_arena, uint64_t size);
void temp_arena_pop(temp_arena_t *temp_arena);

// NOTE: Functions below are related to logging
void bloat_log(BLOAT_LOG, char *, ...);

// NOTE: Functions below are related to string builder;
void sb_append(string_builder_t *, const char *);

// Array Related Functions
typedef struct {
  void **items;
  size_t count;
  size_t capacity;
} array;

#define foreach(item, da)                                                      \
  for (size_t _i = 0; _i < (da)->count; _i++)                                  \
    for (void *item = (da)->items[_i], *_once = (void *)1; _once; _once = NULL)

void da_append_s(array *da, void *item, size_t size);
void da_append_arena(arena_t *arena, array *da, void *item, size_t size);
void da_free(array *da);

#define da_append(da, item) da_append_s((da), (item), sizeof(*(item)))

#ifdef BLOAT_IMPLEMENTATION

#define IS_ALLIGN(x, y) ((x & (y - 1)) == 0)

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

static uint64_t align_mem(uint64_t pos, uint64_t size) {
  if (IS_ALLIGN(pos, size))
    return pos;
  return ((pos + (size - 1)) & ~(size - 1));
}

static arena_chunk_t *chunk_alloc(uint64_t capacity) {
  arena_chunk_t *chunk = malloc(sizeof(arena_chunk_t) + capacity);
  if (!chunk)
    return NULL;
  chunk->next = NULL;
  chunk->pos = 0;
  chunk->capacity = capacity;
  return chunk;
}

arena_t *arena_alloc(void) {
  arena_t *arena = malloc(sizeof(arena_t));
  if (!arena)
    return NULL;
  arena->head = chunk_alloc(ARENA_CHUNK_SIZE);
  if (!arena->head) {
    bloat_log(BLOAT_ERROR, "%s", "Could not allocate arena chunk");
    free(arena);
    return NULL;
  }
  arena->current = arena->head;
  return arena;
}

void arena_free(arena_t *arena) {
  arena_chunk_t *chunk = arena->head;
  while (chunk) {
    arena_chunk_t *next = chunk->next;
    free(chunk);
    chunk = next;
  }
  free(arena);
}

void *arena_push(arena_t *arena, uint64_t size) {
  arena_chunk_t *chunk = arena->current;
  uint64_t pos = align_mem(chunk->pos, size);

  if (pos + size > chunk->capacity) {
    /* Current chunk is full — chain a new one. Old chunk stays in
       place, so every pointer previously returned remains valid.    */
    uint64_t cap = MAX(size, (uint64_t)ARENA_CHUNK_SIZE);
    arena_chunk_t *next = chunk_alloc(cap);
    if (!next) {
      bloat_log(BLOAT_ERROR, "%s", "Could not allocate arena chunk");
      return NULL;
    }
    chunk->next = next;
    arena->current = next;
    chunk = next;
    pos = 0;
  }

  chunk->pos = pos + size;
  return chunk->data + pos;
}

void *arena_push_zero(arena_t *arena, uint64_t size) {
  void *ptr = arena_push(arena, size);
  if (ptr)
    memset(ptr, 0, size);
  return ptr;
}

/* Pop bytes off the current chunk only. Cannot cross chunk boundaries
   since we never track which chunk a given pointer came from.          */
void arena_pop(arena_t *arena, uint64_t size) {
  size = MIN(size, arena->current->pos);
  arena->current->pos -= size;
}

/* pos is treated as a position within the current chunk */
void arena_pop_to(arena_t *arena, uint64_t pos) {
  if (pos >= arena->current->pos)
    return;
  arena->current->pos = pos;
}

/* Reset to a completely empty state, freeing all chunks except the first */
void arena_clear(arena_t *arena) {
  /* free every chunk after head */
  arena_chunk_t *chunk = arena->head->next;
  while (chunk) {
    arena_chunk_t *next = chunk->next;
    free(chunk);
    chunk = next;
  }
  arena->head->next = NULL;
  arena->head->pos = 0;
  arena->current = arena->head;
}
/* NOTE: temp arenas */
temp_arena_t *temp_arena_alloc(arena_t *parent_arena) {
  temp_arena_t *temp = malloc(sizeof(temp_arena_t));
  if (!temp)
    return NULL;
  temp->arena = parent_arena;
  temp->saved_chunk = parent_arena->current;
  temp->saved_pos = parent_arena->current->pos;
  return temp;
}

static void _temp_arena_restore(temp_arena_t *temp) {
  arena_chunk_t *chunk = temp->saved_chunk->next;
  while (chunk) {
    arena_chunk_t *next = chunk->next;
    free(chunk);
    chunk = next;
  }
  temp->saved_chunk->next = NULL;
  temp->saved_chunk->pos = temp->saved_pos;
  temp->arena->current = temp->saved_chunk;
}

void temp_arena_free(temp_arena_t *temp) {
  _temp_arena_restore(temp);
  free(temp);
}

void *temp_arena_push(temp_arena_t *temp, uint64_t size) {
  return arena_push(temp->arena, size);
}

void temp_arena_pop(temp_arena_t *temp) { _temp_arena_restore(temp); }

/* NOTE:(xsoder)
Please be ware to use this macro for scope based temporary arenas meaning
you do not even need to do free call in your app preventing use after
free issues.

usage:
    int foo(arena_t *arena) {
        SCOPED_TEMP_ARENA(arena, t) {
            void *mem = temp_arena_push(t, 1024);
            if (mem) {
                // do task
            }
        }
    }
See no need to call the free here this makes it a nicer helper in my opinion.
*/
#define SCOPED_TEMP_ARENA(parent_arena, temp_arena_var)                        \
  for (temp_arena_t *temp_arena_var = temp_arena_alloc(parent_arena);          \
       temp_arena_var != NULL;                                                 \
       temp_arena_free(temp_arena_var), temp_arena_var = NULL)

/* NOTE: logging functions */

static char *get_type(BLOAT_LOG type) {
  if (type == BLOAT_WARN)
    return "BLOAT_WARN: ";
  else if (type == BLOAT_ERROR)
    return "BLOAT_ERROR: ";
  else if (type == BLOAT_INFO)
    return "BLOAT_INFO: ";
  else {
    printf("Unknown type — refer to BLOAT_WARN, BLOAT_ERROR, BLOAT_INFO\n");
    return NULL;
  }
}

void bloat_log(BLOAT_LOG type, char *fmt, ...) {
  char *log_type = get_type(type);
  if (!log_type)
    return;

  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "%s", log_type);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
}

/* NOTE: dynamic array macros (internal use by sb_append) */

#define _da_alloc(array)                                                       \
  if (!(array)->items) {                                                       \
    (array)->count = 0;                                                        \
    (array)->capacity = DEFAULT_CAPACITY;                                      \
    (array)->items = malloc((array)->capacity * sizeof(*(array)->items));      \
  }

#define _da_realloc_cus(array, size)                                           \
  if ((array)->count >= (array)->capacity) {                                   \
    (array)->capacity *= 2;                                                    \
    (array)->items = realloc((array)->items, (array)->capacity * (size));      \
  }

/* NOTE: string builder */

void sb_append(string_builder_t *sb, const char *item) {
  if (!sb->items)
    _da_alloc(sb);
  size_t len = strlen(item);
  _da_realloc_cus(sb, len);
  strcpy(&sb->items[sb->count], item);
  sb->count += len;
}

void da_append_s(array *da, void *item, size_t size) {
  if (!da->items) {
    da->count = 0;
    da->capacity = 256;
    da->items = malloc(sizeof(void *) * da->capacity);
  }

  if (da->count >= da->capacity) {
    da->capacity *= 2;
    da->items = realloc(da->items, sizeof(void *) * da->capacity);
  }

  void *temp = malloc(size);
  memcpy(temp, item, size);
  da->items[da->count++] = temp;
}

void da_append_arena(arena_t *arena, array *da, void *item, size_t size) {
  if (!da->items) {
    da->count = 0;
    da->capacity = 256;
    da->items = arena_push(arena, sizeof(void *) * da->capacity);
  }

  if (da->count >= da->capacity) {
    size_t old_cap = da->capacity;
    da->capacity *= 2;
    void **new_items = arena_push(arena, sizeof(void *) * da->capacity);
    memcpy(new_items, da->items, sizeof(void *) * old_cap);
    da->items = new_items;
  }

  void *temp = arena_push(arena, size);
  memcpy(temp, item, size);
  da->items[da->count++] = temp;
}

void da_free(array *da) {
  if (!da->items)
    return;
  for (size_t i = 0; i < da->count; i++) {
    free(da->items[i]);
  }
  free(da->items);

  da->items = NULL;
  da->count = 0;
  da->capacity = 0;
}

#undef _da_alloc
#undef _da_realloc_cus

#endif /* BLOAT_IMPLEMENTATION */
#endif /* BLOAT_H */
