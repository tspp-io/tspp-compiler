#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

// Forward declarations for TSPP types
typedef struct TsppSlice {
    void* data;
    size_t len;
    size_t cap;
} TsppSlice;

typedef struct TsppString {
    const char* data;
    size_t len;
} TsppString;

typedef struct TsppMap {
    void* impl;  // Internal implementation
    size_t len;
} TsppMap;

typedef struct TsppChannel {
    void* impl;  // Internal implementation
    size_t cap;
    bool closed;
} TsppChannel;

// Go built-in functions

// Memory allocation functions
void* tspp_make_slice(size_t elem_size, size_t len, size_t cap);
void* tspp_make_map(size_t key_size, size_t value_size);
void* tspp_make_channel(size_t elem_size, size_t buffer_size);
void* tspp_new(size_t size);

// Collection functions
size_t tspp_len(const void* collection, int type);  // type: 0=slice, 1=map, 2=channel, 3=string
size_t tspp_cap(const void* collection, int type);  // type: 0=slice, 2=channel

// Slice operations
TsppSlice* tspp_append(TsppSlice* slice, const void* elem, size_t elem_size);
void tspp_copy(void* dst, const void* src, size_t n);

// Map operations
void tspp_delete(TsppMap* map, const void* key, size_t key_size);

// Channel operations
void tspp_close(TsppChannel* channel);

// Error handling
void tspp_panic(const char* message);
void* tspp_recover();

// Memory management
void tspp_gc_collect();

#ifdef __cplusplus
}
#endif