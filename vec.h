#pragma once

struct vec {
  int itemsize;
  int capacity; // in number of item
  int len; // in number of item
  void *data;
};

static inline struct vec vec_create(int itemsize) {
  struct vec vec;
  vec.itemsize = itemsize;
  vec.capacity = 16; // init capacity
  vec.len = 0;
  vec.data = malloc(vec.capacity * itemsize);
  return vec;
}

static inline void vec_append(struct vec* vec, void *itemptr) {
  if (vec->len == vec->capacity) {
    vec->capacity <<= 1;
    vec->data =realloc(vec->data, vec->capacity * vec->itemsize);
  }
  memcpy(vec->data + vec->len * vec->itemsize, itemptr, vec->itemsize);
  ++vec->len;
}

static inline void* vec_get_item(struct vec* vec, int idx) {
  assert(idx >= 0 && idx < vec->len);
  return vec->data + idx * vec->itemsize;
}

static inline void vec_free(struct vec* vec) {
  assert(vec->data);
  free(vec->data);
  vec->data = NULL;
}
