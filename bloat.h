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

// NOTE: Functions below are related to logging
void bloat_log(BLOAT_LOG, char *, ...);

// NOTE: Functions below are related to string builder;
void sb_append(string_builder_t *, const char *);

#ifdef BLOAT_IMPLEMNTATION
/*
This power macro is used to see if it is a module or not
and it will return a boolean
*/
// TODO: Better name
#define IS_ALLIGN(x, y) ((x & (y-1)) == 0)

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define ARENA_BASE_POS (sizeof(arena_t))

#define TODO(x) {      \
    printf("%s\n", x); \
    abort();          \
}

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
    if ( pos >= arena->pos) return;
    uint64_t _size = pos - arena->pos;
    arena_pop(arena, _size);
}

void arena_clear(arena_t *arena)
{
    arena_pop_to(arena, ARENA_BASE_POS);
}


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

// NOTE: New line is not required
void bloat_log(BLOAT_LOG type, char *fmt, ...)
{
    va_list args;
    char *log_type = get_type(type);
    if (!log_type) return;

    fprintf(stderr, log_type);

    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

// NOTE: Dynamic array append macros
// FIXME: Use the custom arena allocators for allocations
#define da_alloc(array)                                                      \
    if (!(array)->items) {                                                   \
        (array)->count = 0 ;                                                 \
        (array)->capacity = DEFAULT_CAPACITY;                                \
        (array)->items = malloc((array)->capacity * sizeof((array->items))); \
    }

/* NOTE: This is only for specific reason to have customizable size due to string
 builder requiring strlen instead of the size of the struct.
*/
#define da_realloc_cus(array, size) \
    if ((array)->count >= (array)->capacity) {                        \
        (array)->capacity *= 2;                                       \
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

// NOTE: Undefine useless things a user of the library should not see
#undef da_alloc
#undef da_realloc

#endif
#endif //BLOAT_H
