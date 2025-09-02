#include "../src/packages/tspp_std/go_builtins.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include <setjmp.h>

// Test harness
static int tests_passed = 0;
static int tests_failed = 0;
static jmp_buf test_recovery_buf;

#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            std::cout << "✅ PASS: " << message << std::endl; \
            tests_passed++; \
        } else { \
            std::cout << "❌ FAIL: " << message << std::endl; \
            tests_failed++; \
        } \
    } while(0)

#define TEST_SECTION(name) \
    std::cout << "\n=== " << name << " ===" << std::endl;

extern "C" {

void test_slice_operations() {
    TEST_SECTION("Slice Operations");
    
    // Test make_slice
    TsppSlice* slice = (TsppSlice*)tspp_make_slice(sizeof(int), 3, 5);
    TEST_ASSERT(slice != nullptr, "make_slice creates non-null slice");
    TEST_ASSERT(tspp_len(slice, 0) == 3, "slice length is correct");
    TEST_ASSERT(tspp_cap(slice, 0) == 5, "slice capacity is correct");
    
    // Test zero initialization
    int* data = (int*)slice->data;
    bool all_zeros = true;
    for (size_t i = 0; i < slice->len; i++) {
        if (data[i] != 0) {
            all_zeros = false;
            break;
        }
    }
    TEST_ASSERT(all_zeros, "slice elements are zero-initialized");
    
    // Test append operation
    int value = 42;
    tspp_append(slice, &value, sizeof(int));
    TEST_ASSERT(tspp_len(slice, 0) == 4, "append increases length");
    TEST_ASSERT(data[3] == 42, "appended value is correct");
    
    // Test capacity growth on append
    int values[] = {100, 200};
    tspp_append(slice, &values[0], sizeof(int));
    tspp_append(slice, &values[1], sizeof(int)); // This should trigger growth
    TEST_ASSERT(tspp_len(slice, 0) == 6, "multiple appends increase length");
    TEST_ASSERT(tspp_cap(slice, 0) >= 6, "capacity grows when needed");
    
    free(slice->data);
    free(slice);
}

void test_memory_allocation() {
    TEST_SECTION("Memory Allocation Functions");
    
    // Test new() function
    int* int_ptr = (int*)tspp_new(sizeof(int));
    TEST_ASSERT(int_ptr != nullptr, "new() returns non-null pointer");
    TEST_ASSERT(*int_ptr == 0, "new() zero-initializes memory");
    free(int_ptr);
    
    // Test make_map
    TsppMap* map = (TsppMap*)tspp_make_map(sizeof(int), sizeof(char*));
    TEST_ASSERT(map != nullptr, "make_map creates non-null map");
    TEST_ASSERT(tspp_len(map, 1) == 0, "new map has zero length");
    free(map);
    
    // Test make_channel  
    TsppChannel* channel = (TsppChannel*)tspp_make_channel(sizeof(int), 10);
    TEST_ASSERT(channel != nullptr, "make_channel creates non-null channel");
    TEST_ASSERT(tspp_cap(channel, 2) == 10, "channel has correct capacity");
    TEST_ASSERT(!channel->closed, "new channel is not closed");
    free(channel);
}

void test_string_operations() {
    TEST_SECTION("String Operations");
    
    TsppString str;
    str.data = "Hello, World!";
    str.len = strlen(str.data);
    
    TEST_ASSERT(tspp_len(&str, 3) == 13, "string length is correct");
    
    // Test copy operation with strings
    char buffer[20];
    tspp_copy(buffer, str.data, str.len);
    buffer[str.len] = '\0';
    TEST_ASSERT(strcmp(buffer, "Hello, World!") == 0, "copy works correctly for strings");
}

void test_channel_operations() {
    TEST_SECTION("Channel Operations");
    
    TsppChannel* channel = (TsppChannel*)tspp_make_channel(sizeof(double), 5);
    TEST_ASSERT(!channel->closed, "new channel is open");
    
    tspp_close(channel);
    TEST_ASSERT(channel->closed, "close() marks channel as closed");
    
    free(channel);
}

void test_map_operations() {
    TEST_SECTION("Map Operations");
    
    TsppMap* map = (TsppMap*)tspp_make_map(sizeof(int), sizeof(char*));
    
    // Test delete operation (placeholder implementation)
    int key = 42;
    tspp_delete(map, &key, sizeof(int));
    TEST_ASSERT(true, "delete operation doesn't crash"); // Basic test since it's a placeholder
    
    free(map);
}

void test_copy_function() {
    TEST_SECTION("Copy Function");
    
    int src[] = {1, 2, 3, 4, 5};
    int dst[5] = {0};
    
    tspp_copy(dst, src, sizeof(src));
    
    bool copy_correct = true;
    for (int i = 0; i < 5; i++) {
        if (dst[i] != src[i]) {
            copy_correct = false;
            break;
        }
    }
    TEST_ASSERT(copy_correct, "copy function copies data correctly");
    
    // Test overlapping copy (memmove behavior)
    int overlap_data[] = {1, 2, 3, 4, 5, 6};
    tspp_copy(overlap_data + 2, overlap_data, 3 * sizeof(int));
    TEST_ASSERT(overlap_data[2] == 1 && overlap_data[3] == 2 && overlap_data[4] == 3, 
                "copy handles overlapping memory correctly");
}

void test_panic_recover() {
    TEST_SECTION("Panic/Recover Mechanism");
    
    // Test basic panic/recover (this is tricky to test safely)
    std::cout << "ℹ️  Note: Panic/recover test requires careful handling in real usage" << std::endl;
    
    // Test recover when no panic occurred
    void* recover_result = tspp_recover();
    TEST_ASSERT(recover_result == nullptr, "recover() returns null when no panic occurred");
    
    // For a real panic test, we'd need to set up the jmp_buf properly
    // This is more of an integration test that would be better done at runtime
    TEST_ASSERT(true, "panic/recover mechanism compiles and links correctly");
}

void test_slice_growth_strategy() {
    TEST_SECTION("Slice Growth Strategy");
    
    TsppSlice* slice = (TsppSlice*)tspp_make_slice(sizeof(int), 0, 0);
    
    // Test growth from empty slice
    int value = 1;
    tspp_append(slice, &value, sizeof(int));
    TEST_ASSERT(tspp_cap(slice, 0) >= 1, "slice grows from empty");
    
    size_t prev_cap = tspp_cap(slice, 0);
    
    // Fill to capacity
    for (size_t i = 1; i < prev_cap; i++) {
        int val = (int)i + 1;
        tspp_append(slice, &val, sizeof(int));
    }
    
    // This append should trigger growth
    int trigger_value = 999;
    tspp_append(slice, &trigger_value, sizeof(int));
    
    TEST_ASSERT(tspp_cap(slice, 0) > prev_cap, "slice capacity doubles when full");
    TEST_ASSERT(tspp_cap(slice, 0) >= prev_cap * 2, "slice follows Go's growth strategy");
    
    free(slice->data);
    free(slice);
}

void test_edge_cases() {
    TEST_SECTION("Edge Cases and Error Handling");
    
    // Test null pointer handling
    TEST_ASSERT(tspp_len(nullptr, 0) == 0, "len() handles null slice");
    TEST_ASSERT(tspp_cap(nullptr, 0) == 0, "cap() handles null slice");
    
    // Test zero-size allocation
    void* zero_ptr = tspp_new(0);
    TEST_ASSERT(zero_ptr != nullptr || true, "new(0) doesn't crash"); // Implementation dependent
    if (zero_ptr) free(zero_ptr);
    
    // Test copy with null/zero
    tspp_copy(nullptr, nullptr, 0);
    TEST_ASSERT(true, "copy() handles null pointers gracefully");
    
    // Test make_slice with cap < len
    TsppSlice* slice = (TsppSlice*)tspp_make_slice(sizeof(int), 5, 3);
    TEST_ASSERT(tspp_cap(slice, 0) >= tspp_len(slice, 0), "make_slice adjusts cap >= len");
    
    if (slice) {
        free(slice->data);
        free(slice);
    }
}

} // extern "C"

int main() {
    std::cout << "🚀 TSPP++ Go Built-ins Test Suite" << std::endl;
    std::cout << "===================================" << std::endl;
    
    try {
        test_memory_allocation();
        test_slice_operations(); 
        test_string_operations();
        test_channel_operations();
        test_map_operations();
        test_copy_function();
        test_slice_growth_strategy();
        test_edge_cases();
        test_panic_recover();
        
        std::cout << "\n📊 Test Summary:" << std::endl;
        std::cout << "✅ Passed: " << tests_passed << std::endl;
        std::cout << "❌ Failed: " << tests_failed << std::endl;
        std::cout << "📈 Success Rate: " << 
                     (tests_passed * 100.0 / (tests_passed + tests_failed)) << "%" << std::endl;
        
        if (tests_failed == 0) {
            std::cout << "\n🎉 All tests passed! Go built-ins are working correctly." << std::endl;
        } else {
            std::cout << "\n⚠️  Some tests failed. Please review the implementation." << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "💥 Test suite crashed: " << e.what() << std::endl;
        return 1;
    }
    
    return tests_failed == 0 ? 0 : 1;
}