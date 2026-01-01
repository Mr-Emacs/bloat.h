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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

#define DEFAULT_CAPACITY 1024

typedef enum {
    BLOAT_WARN,
    BLOAT_ERROR,
    BLOAT_INFO,
} BLOAT_LOG;

typedef struct {
    uint64_t pos;
    uint64_t size;
    uint64_t capacity;
    void *data;
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
    uint64_t start_pos;
} temp_arena_t;

temp_arena_t *temp_arena_alloc(arena_t *parent_arena);
void temp_arena_free(temp_arena_t *temp_arena);
void *temp_arena_push(temp_arena_t *temp_arena, uint64_t size);
void temp_arena_pop(temp_arena_t *temp_arena);

// NOTE: Functions below are related to logging
void bloat_log(BLOAT_LOG, char *, ...);

// NOTE: Functions below are related to string builder;
void sb_append(string_builder_t *, const char *);

#ifdef BLOAT_IMPLEMNTATION
/*
This macro is used to see if it is a module or not
and it will return a boolean
*/
// TODO: Think of a better name
#define IS_ALLIGN(x, y) ((x & (y-1)) == 0)

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define ARENA_BASE_POS (sizeof(arena_t))

uint64_t align_mem(uint64_t pos, uint64_t size)
{
    if (IS_ALLIGN(pos, size)) return pos;
    return ((pos + (size - 1)) & ~(size - 1));
}

arena_t *arena_alloc(void)
{
    arena_t *arena = malloc(sizeof(arena_t));
    arena->capacity = DEFAULT_CAPACITY;
    arena->size = 0;
    arena->pos = 0;
    arena->data = malloc(sizeof(arena->size) * arena->capacity);
    if (!arena->data) {
        bloat_log(BLOAT_ERROR, "%s", "Could not allocate more memory");
        free(arena);
        return NULL;
    }
    return arena;
}

void arena_free(arena_t *arena)
{
    free(arena->data);
    free(arena);
}

void *arena_push(arena_t *arena, uint64_t size)
{
    // TODO: Add proper enum for error handling
    uint64_t _pos = align_mem(arena->pos, size);
    uint64_t _new_pos = _pos + size;

    if (_new_pos > arena->capacity) {
        bloat_log(BLOAT_ERROR, "The new position %lu is less than the capacirty %lu",
                  _new_pos, arena->capacity);
        return NULL;
    }

    arena->pos = _new_pos;
    return (char*)arena->data + _pos;
}

void *arena_push_zero(arena_t *arena, uint64_t size)
{
    void *ptr = arena_push(arena, size);
    if (ptr) memset(ptr, 0, size);
    return ptr;
}

void arena_pop(arena_t *arena, uint64_t size)
{
    size = MIN(size, arena->pos - ARENA_BASE_POS);
    arena->pos -= size;
}

void arena_pop_to(arena_t *arena, uint64_t pos)
{
    if (pos >= arena->pos) return;
    uint64_t _size = pos - arena->pos;
    arena_pop(arena, _size);
}

#define arena_clear(arena) arena_pop_to(arena, ARENA_BASE_POS)

temp_arena_t *temp_arena_alloc(arena_t *parent_arena)
{
    temp_arena_t *temp_arena = malloc(*temp_arena_t);
    temp_arena->arena = parent_arena;
    temp_arena->start_pos = parent_arena->pos;
    return temp_arena;
}

void temp_arena_free(temp_arena_t *temp_arena)
{
    uint64_t end_pos = temp_arena->parent_arena->pos;
    temp_arena->parent_arena->pos = temp_arena->start_pos;
    free(temp_arena);
}

void *temp_arena_push(temp_arena_t *temp_arena, uint64_t size)
{
    return arena_push(temp_arena->parent_arena, size);
}

void temp_arena_pop(temp_arena_t *temp_arena)
{
    temp_arena->parent_arena->pos = temp_arena->start_pos;
}

/* NOTE:(xsoder)
Please be ware to use this macro for scope based temporary arenas meaning
you do not even need to do free call in your app preventing use after
free issues.

usage:
    int foo(arena_t arena) {
        SCOPED_TEMP_ARENA(arena, temp_arena_var);
        void *mem = temp_arena_push(arena, UINT32_MAX);
        if (mem) {
            // do task
        }
    }
See no need to call the free here this makes it a nicer helper in my opinion.
*/

#define SCOPED_TEMP_ARENA(parent_arena, temp_arena_var)                  \
    for (temp_arena_t *temp_arena_var = temp_arena_alloc(parent_arena);  \
         temp_arena_var != NULL;                                         \
         temp_arena_free(temp_arena_var), temp_arena_var = NULL)

char *get_type(BLOAT_LOG type)
{
    if (type == BLOAT_WARN) return "BLOAT_WARN: ";
    else if (type == BLOAT_ERROR) return "BLOAT_ERROR: ";
    else if (type == BLOAT_INFO) return "BLOAT_INFO: ";
    else {
        printf("Unkown type refer to BLOAT_WARN, BLOAT_ERROR, BLOAT_INFO\n");
        return NULL;
    }
}

void bloat_log(BLOAT_LOG type, char *fmt, ...)
{
    char *log_type = get_type(type);
    if (!log_type) return;

    va_list args;
    va_start(args, fmt);

    fprintf(stderr, log_type);

    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
}

// NOTE:  Dynamic array append macros
// FIXME: DEPRICIATED: Use the custom arena allocators for allocations.
#define da_alloc(array)                                                      \
    if (!(array)->items) {                                                   \
        (array)->count = 0 ;                                                 \
        (array)->capacity = DEFAULT_CAPACITY;                                \
        (array)->items = malloc((array)->capacity * sizeof((array->items))); \
    }

/* NOTE:(xsoder)
    This is only for specific reason to have customizable size due to string
    builder requiring strlen instead of the size of the struct.
*/
#define da_realloc_cus(array, size)                                             \
    if ((array)->count >= (array)->capacity) {                                  \
        (array)->capacity *= 2;                                                 \
        (array)->items = realloc((array)->items, (((array)->capacity) * size)); \
    }

#define da_realloc(array) da_realloc_cus((array), sizeof((array)))

#define da_append(array, item)                         \
    do {                                               \
        da_realloc((array));                           \
        (array)->items[(array)->count++] = item;       \
    } while(0)

#define da_free(array)                                 \
    free((array)->items)                               \
    free((array))                                      \

// NOTE: String builder functions
void sb_append(string_builder_t *sb, const char *item)
{
    if (sb->items == NULL) {
        da_alloc(sb);
    }
    size_t len = strlen(item);
    da_realloc_cus(sb, len);
    strcpy(&sb->items[sb->count], item);
    sb->count += len;
}

#undef da_alloc
#undef da_realloc
#undef da_realloc_cus

#endif
#endif //BLOAT_H
