#include "go_builtins.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>

// Global panic/recover support
static jmp_buf panic_buf;
static bool panic_active = false;
static char panic_message[1024] = {0};

extern "C" {

// Memory allocation functions

void* tspp_make_slice(size_t elem_size, size_t len, size_t cap) {
    if (cap < len) {
        cap = len;
    }
    
    TsppSlice* slice = (TsppSlice*)malloc(sizeof(TsppSlice));
    if (!slice) {
        tspp_panic("out of memory");
        return nullptr;
    }
    
    slice->data = calloc(cap, elem_size);  // Zero-initialize
    if (!slice->data && cap > 0) {
        free(slice);
        tspp_panic("out of memory");
        return nullptr;
    }
    
    slice->len = len;
    slice->cap = cap;
    return slice;
}

void* tspp_make_map(size_t key_size, size_t value_size) {
    // For now, create a simple map structure
    // In a real implementation, this would be a hash table
    TsppMap* map = (TsppMap*)malloc(sizeof(TsppMap));
    if (!map) {
        tspp_panic("out of memory");
        return nullptr;
    }
    
    // Placeholder implementation
    map->impl = nullptr;  // Would point to hash table implementation
    map->len = 0;
    return map;
}

void* tspp_make_channel(size_t elem_size, size_t buffer_size) {
    TsppChannel* channel = (TsppChannel*)malloc(sizeof(TsppChannel));
    if (!channel) {
        tspp_panic("out of memory");
        return nullptr;
    }
    
    // Placeholder implementation
    channel->impl = nullptr;  // Would point to channel implementation
    channel->cap = buffer_size;
    channel->closed = false;
    return channel;
}

void* tspp_new(size_t size) {
    void* ptr = calloc(1, size);  // Zero-initialize like Go's new()
    if (!ptr && size > 0) {
        tspp_panic("out of memory");
        return nullptr;
    }
    return ptr;
}

// Collection functions

size_t tspp_len(const void* collection, int type) {
    if (!collection) return 0;
    
    switch (type) {
        case 0: {  // slice
            const TsppSlice* slice = (const TsppSlice*)collection;
            return slice->len;
        }
        case 1: {  // map
            const TsppMap* map = (const TsppMap*)collection;
            return map->len;
        }
        case 2: {  // channel - would need implementation
            return 0;  // placeholder
        }
        case 3: {  // string
            const TsppString* str = (const TsppString*)collection;
            return str->len;
        }
        default:
            return 0;
    }
}

size_t tspp_cap(const void* collection, int type) {
    if (!collection) return 0;
    
    switch (type) {
        case 0: {  // slice
            const TsppSlice* slice = (const TsppSlice*)collection;
            return slice->cap;
        }
        case 2: {  // channel
            const TsppChannel* channel = (const TsppChannel*)collection;
            return channel->cap;
        }
        default:
            return 0;  // Maps and strings don't have capacity
    }
}

// Slice operations

TsppSlice* tspp_append(TsppSlice* slice, const void* elem, size_t elem_size) {
    if (!slice || !elem) return slice;
    
    // Check if we need to grow the slice
    if (slice->len >= slice->cap) {
        // Grow capacity (similar to Go's growth strategy)
        size_t new_cap = slice->cap == 0 ? 1 : slice->cap * 2;
        void* new_data = realloc(slice->data, new_cap * elem_size);
        if (!new_data) {
            tspp_panic("out of memory during slice append");
            return slice;
        }
        slice->data = new_data;
        slice->cap = new_cap;
    }
    
    // Copy the element to the end of the slice
    char* data_ptr = (char*)slice->data;
    memcpy(data_ptr + (slice->len * elem_size), elem, elem_size);
    slice->len++;
    
    return slice;
}

void tspp_copy(void* dst, const void* src, size_t n) {
    if (dst && src && n > 0) {
        memmove(dst, src, n);  // Use memmove to handle overlapping memory
    }
}

// Map operations

void tspp_delete(TsppMap* map, const void* key, size_t key_size) {
    if (!map || !key) return;
    
    // Placeholder implementation
    // In a real implementation, this would remove the key from the hash table
    // For now, we just acknowledge the operation
    (void)key_size;  // Suppress unused parameter warning
}

// Channel operations

void tspp_close(TsppChannel* channel) {
    if (channel) {
        channel->closed = true;
    }
}

// Error handling

void tspp_panic(const char* message) {
    if (message) {
        strncpy(panic_message, message, sizeof(panic_message) - 1);
        panic_message[sizeof(panic_message) - 1] = '\0';
    } else {
        strcpy(panic_message, "panic");
    }
    
    panic_active = true;
    
    // Print panic message to stderr
    fprintf(stderr, "panic: %s\n", panic_message);
    
    // If we have a recovery point, jump to it
    if (panic_active) {
        longjmp(panic_buf, 1);
    }
    
    // Otherwise, exit the program
    exit(1);
}

void* tspp_recover() {
    if (panic_active) {
        panic_active = false;
        return panic_message;
    }
    return nullptr;
}

// Memory management

void tspp_gc_collect() {
    // Placeholder for garbage collection
    // In a real implementation, this would trigger the Boehm GC
}

}  // extern "C"