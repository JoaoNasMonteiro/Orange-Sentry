#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "logging.h"

#define DEFAULT_ALIGNMENT (2 * sizeof(void *))

/**
 * The core Arena structure.
 * Represents a linear block of memory that allocates by bumping a pointer.
 */
typedef struct {
  uint8_t *buffer;
  size_t length;
  size_t offset;
} Arena;

/**
 * A Slab is a specialized sub-arena.
 * It uses the "Header + Payload" pattern where the struct and its buffer
 * reside in the same contiguous block of memory allocated from a parent.
 */
typedef struct {
  Arena sArena; // The embedded sub-arena
  Arena *parent_arena;
  int id;
} Slab;

/**
 * A stack structure for recycling Slabs.
 * Stores pointers to available Slabs to avoid re-allocation overhead.
 */
typedef struct {
  Slab **slabs;    // Array of pointers to Slabs
  size_t count;    // Current number of available Slabs
  size_t capacity; // Max capacity of the stack
} SlabStack;

static inline bool is_power_of_two(uintptr_t x) {
  return (x != 0) && ((x & (x - 1)) == 0);
}

/**
 * Aligns a memory address forward to the specified alignment.
 * Alignment must be a power of two.
 */
static inline uintptr_t align_forward(uintptr_t ptr, size_t align) {
  if (!is_power_of_two(align)) {
    assert(0 && "Align must be power of two");
    return ptr;
  }
  uintptr_t p = ptr;
  uintptr_t a = (uintptr_t)align;
  return (p + (a - 1)) & ~(a - 1);
}

/**
 * Initializes an Arena with a pre-allocated buffer.
 * Does not allocate memory itself; it just manages the provided range.
 */
static inline void arena_init(Arena *a, void *buffer, size_t length) {
  a->buffer = (uint8_t *)buffer;
  a->length = length;
  a->offset = 0;
}

/**
 * Allocates memory from the Arena with a specific alignment.
 * Returns:
 * void*: Pointer to the aligned memory block.
 * NULL:  If the arena is out of memory.
 */
static inline void *arena_alloc_align(Arena *a, size_t size, size_t align) {
  uintptr_t current_ptr = (uintptr_t)a->buffer + (uintptr_t)a->offset;
  uintptr_t offset_ptr = align_forward(current_ptr, align);

  // Convert back to relative offset for bounds checking
  offset_ptr -= (uintptr_t)a->buffer;

  if (offset_ptr + size > a->length) {
    LOG_ERROR("Could not allocate memory from arena: Arena OOM.");
    return NULL;
  }

  void *ptr = &a->buffer[offset_ptr];
  a->offset = offset_ptr + size;
  memset(ptr, 0, size);
  return ptr;
}

/**
 * Allocates memory from the Arena using the DEFAULT_ALIGNMENT.
 */
static inline void *arena_alloc(Arena *a, size_t size) {
  return arena_alloc_align(a, size, DEFAULT_ALIGNMENT);
}

/**
 * Resets the Arena offset to zero.
 * This effectively "frees" all memory allocated in this arena at once.
 */
static inline void arena_reset(Arena *a) { a->offset = 0; }

// Macros for type-safe allocation
#define ARENA_NEW(a, Type) ((Type *)arena_alloc(a, sizeof(Type)))
#define ARENA_NEW_ARRAY(a, Type, Count)                                        \
  ((Type *)arena_alloc(a, sizeof(Type) * (Count)))

/**
 * Creates a new Slab by allocating a single block from the Parent Arena.
 * The block contains both the Slab metadata and the Slab's internal buffer.
 * * Returns:
 * Slab*: A fully initialized slab.
 * NULL:  If the parent arena is OOM or alignment padding exceeds size.
 */
static inline Slab *arena_create_slab(Arena *parent, size_t size, int id) {
  size_t total_size = sizeof(Slab) + size;
  uint8_t *block = (uint8_t *)arena_alloc(parent, total_size);

  if (!block) {
    LOG_ERROR("Failed to create Slab: Parent Arena OOM");
    return NULL;
  }

  Slab *new_slab = (Slab *)block;

  // Calculate alignment for the internal buffer
  uintptr_t metadata_end = (uintptr_t)block + sizeof(Slab);
  uintptr_t buffer_start = align_forward(metadata_end, DEFAULT_ALIGNMENT);

  size_t padding = buffer_start - metadata_end;

  // Safety Check: Ensure padding didn't eat the whole buffer
  if (padding >= size) {
    LOG_ERROR(
        "Failed to create Slab %d: Alignment padding exceeds requested size",
        id);
    return NULL;
  }

  size_t effective_buffer_size = size - padding;
  uint8_t *slab_buffer = (uint8_t *)buffer_start;

  new_slab->parent_arena = parent;
  new_slab->id = id;

  // Initialize the inner arena with the effective size
  arena_init(&new_slab->sArena, slab_buffer, effective_buffer_size);

  return new_slab;
}

/**
 * Creates a SlabStack, allocating its internal array from the given Arena.
 * Returns NULL on failure.
 */
static inline SlabStack *arena_slab_stack_create(Arena *a, size_t capacity) {
  SlabStack *stack = ARENA_NEW(a, SlabStack);
  if (!stack) {
    LOG_ERROR("Failed to create slab stack: Failed to create Arena.");
    return NULL;
  }

  stack->slabs = ARENA_NEW_ARRAY(a, Slab *, capacity);
  if (!stack->slabs) {
    LOG_ERROR("Failed to create Slab array: Failed to create arena.");
    return NULL;
  }
  stack->capacity = capacity;
  stack->count = 0;
  return stack;
}

/**
 * Pushes a Slab onto the stack for recycling.
 * Also resets the Slab's internal arena so it is fresh for the next user.
 * * Returns:
 * 0: Success.
 * -1: Stack is full (Slab not stored).
 */
static inline int arena_slab_stack_push(SlabStack *stack, Slab *slab) {
  if (stack->count >= stack->capacity) {
    LOG_ERROR(
        "Failed to push slab to stack: number of stacks exceeds capacity");
    return -1;
  }

  // Reset the slab so it's fresh for the next user
  arena_reset(&slab->sArena);

  stack->slabs[stack->count++] = slab;
  return 0;
}

/**
 * Pops a Slab from the stack.
 * Returns:
 * Slab*: A recycled slab.
 * NULL:  Stack is empty.
 */
static inline Slab *arena_slab_stack_pop(SlabStack *stack) {
  if (stack->count == 0) {
    LOG_ERROR("Failed to pop slab: count at zero.");
    return NULL;
  }
  return stack->slabs[--stack->count];
}

/**
 * High-level function to get a Slab.
 * Attempts to recycle a Slab from the stack first. If the stack is empty,
 * it creates a new Slab from the parent Arena.
 * Warn: All Slabs in the stack must be of the same size class.
 * Mixing sizes will cause buffer overflows when reusing small Slabs for large
 * requests.
 */
static inline Slab *arena_acquire_slab(Arena *a, SlabStack *stack, size_t size,
                                       int id) {
  Slab *s = arena_slab_stack_pop(stack);

  if (!s) {
    s = arena_create_slab(a, size, id);
  } else {
    s->id = id;
  }

  return s;
}

/**
 * Releases a Slab back to the stack.
 * If the stack is full, the Slab is abandoned
 */
static inline void arena_release_slab(SlabStack *stack, Slab *s) {
  arena_slab_stack_push(stack, s);
}
