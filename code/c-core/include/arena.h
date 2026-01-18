#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#define DEFAULT_ALIGNMENT (2 * sizeof(void *))

typedef struct{
  uint8_t* buffer;
  size_t length;
  size_t offset;
}Arena;

static inline bool is_power_of_two(uintptr_t x){
  return (x & (x - 1) == 0);
}

// helper. calculates the memmory padding needed to alogn the offset with the powers of two
static inline uintptr_t align_forward(uintptr_t ptr, size_t aling){
  uintptr_t modulo, p, a;
  if(!is_power_of_two(align)) return ptr;

  p = ptr;
  a = (uintptr_t)align;

  modulo = p & (a - 1);

  if (modulo != 0){
    p += a - modulo;
  } 

  return p;
}

static inline void arena_init(Arena* a, void* buffer size_t length){
  a->buffer = (uint8_t*)buffer;
  a->length = length;
  a->offset = 0;
}

static inline void* arena_alloc_align(Arena* a, size_t size, size_t align){
  uintptr_t current_ptr = (uintptr_t)a->buffer + (uintptr_t)a->offset; 
  
  uintptr_t offset = align_forward(current_ptr, align);
  
  offset -= (uintptr_t)a->buffer;

  if (offset + size > a->length) {
    return NULL;
  }
  
  void* ptr = &a->buffer[offset];

  a->offset = offset + size;

  memset(ptr, 0, size);

  return ptr;

}

static inline void* arena_alloc(Arena* a; size_t size){
  arena_alloc_align(a, size, DEFAULT_ALIGNMENT);
}


static inline void arena_reset(Arena* a){
  a->offset = 0;
}

#define ARENA_NEW(a, Type) ((Type *)arena_alloc(a, sizeof(Type)))
#define ARENA_NEW_ARRAY(a, Type, Count) ((Type *)arena_alloc(a, sizeof(Type) * (Count)))

#endif
