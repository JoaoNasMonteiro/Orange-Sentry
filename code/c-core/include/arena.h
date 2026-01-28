#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define DEFAULT_ALIGNMENT (2 * sizeof(void *))

typedef struct {
  uint8_t *buffer;
  size_t length;
  size_t offset;
} Arena;

static inline bool is_power_of_two(uintptr_t x) {
  return (x != 0) && ((x & (x - 1)) == 0);
}

static inline uintptr_t align_forward(uintptr_t ptr, size_t align) {
  if (!is_power_of_two(align)) {
    assert(0 && "Align must be power of two");
    return ptr;
  }

  uintptr_t p = ptr;
  uintptr_t a = (uintptr_t)align;

  return (p + (a - 1)) & ~(a - 1);
}

static inline void arena_init(Arena *a, void *buffer, size_t length) {
  a->buffer = (uint8_t *)buffer;
  a->length = length;
  a->offset = 0;
}

static inline void *arena_alloc_align(Arena *a, size_t size, size_t align) {
  uintptr_t current_ptr = (uintptr_t)a->buffer + (uintptr_t)a->offset;

  uintptr_t offset_ptr = align_forward(current_ptr, align);

  offset_ptr -= (uintptr_t)a->buffer;

  if (offset_ptr + size > a->length) {
    return NULL;
  }

  void *ptr = &a->buffer[offset_ptr];

  a->offset = offset_ptr + size;

  memset(ptr, 0, size);

  return ptr;
}

static inline void *arena_alloc(Arena *a, size_t size) {
  return arena_alloc_align(a, size, DEFAULT_ALIGNMENT);
}

static inline void arena_reset(Arena *a) { a->offset = 0; }

#define ARENA_NEW(a, Type) ((Type *)arena_alloc(a, sizeof(Type)))
#define ARENA_NEW_ARRAY(a, Type, Count)                                        \
  ((Type *)arena_alloc(a, sizeof(Type) * (Count)))
