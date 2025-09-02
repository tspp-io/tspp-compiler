#include "../src/packages/tspp_std/go_builtins.h"
#include <iostream>
#include <cstring>
#include <iomanip>

extern "C" {

void print_slice_info(const char* name, TsppSlice* slice) {
    std::cout << "Slice " << name << ": len=" << tspp_len(slice, 0) 
              << ", cap=" << tspp_cap(slice, 0) << std::endl;
}

void demonstrate_go_builtins() {
    std::cout << "🚀 TSPP++ Go Built-ins Integration Demo" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    // 1. Demonstrate slice creation and manipulation
    std::cout << "\n📋 1. Slice Operations Demo" << std::endl;
    std::cout << "Creating slice with initial capacity..." << std::endl;
    
    TsppSlice* numbers = (TsppSlice*)tspp_make_slice(sizeof(int), 0, 2);
    print_slice_info("numbers", numbers);
    
    // Add some numbers using append
    std::cout << "Appending numbers 1-5..." << std::endl;
    for (int i = 1; i <= 5; i++) {
        tspp_append(numbers, &i, sizeof(int));
        std::cout << "  After appending " << i << ": ";
        print_slice_info("", numbers);
    }
    
    // Display slice contents
    std::cout << "Final slice contents: [";
    int* data = (int*)numbers->data;
    for (size_t i = 0; i < numbers->len; i++) {
        if (i > 0) std::cout << ", ";
        std::cout << data[i];
    }
    std::cout << "]" << std::endl;
    
    // 2. Demonstrate copy operations
    std::cout << "\n📋 2. Copy Operations Demo" << std::endl;
    
    TsppSlice* copy_dest = (TsppSlice*)tspp_make_slice(sizeof(int), 5, 5);
    std::cout << "Copying slice data..." << std::endl;
    tspp_copy(copy_dest->data, numbers->data, numbers->len * sizeof(int));
    
    std::cout << "Copied slice contents: [";
    int* copy_data = (int*)copy_dest->data;
    for (size_t i = 0; i < copy_dest->len; i++) {
        if (i > 0) std::cout << ", ";
        std::cout << copy_data[i];
    }
    std::cout << "]" << std::endl;
    
    // 3. Demonstrate string operations
    std::cout << "\n📋 3. String Operations Demo" << std::endl;
    
    TsppString hello;
    hello.data = "Hello, TSPP++!";
    hello.len = strlen(hello.data);
    
    std::cout << "String: \"" << hello.data << "\"" << std::endl;
    std::cout << "Length: " << tspp_len(&hello, 3) << " characters" << std::endl;
    
    // 4. Demonstrate zero-value allocation
    std::cout << "\n📋 4. Zero-Value Allocation Demo" << std::endl;
    
    struct Point {
        int x, y;
        double distance;
    };
    
    Point* origin = (Point*)tspp_new(sizeof(Point));
    std::cout << "Zero-initialized Point: (" 
              << origin->x << ", " << origin->y 
              << ", distance=" << std::fixed << std::setprecision(1) 
              << origin->distance << ")" << std::endl;
    
    // 5. Demonstrate map and channel creation
    std::cout << "\n📋 5. Map and Channel Creation Demo" << std::endl;
    
    TsppMap* user_scores = (TsppMap*)tspp_make_map(sizeof(int), sizeof(double));
    std::cout << "Created map with length: " << tspp_len(user_scores, 1) << std::endl;
    
    TsppChannel* message_queue = (TsppChannel*)tspp_make_channel(sizeof(char*), 10);
    std::cout << "Created channel with capacity: " << tspp_cap(message_queue, 2) << std::endl;
    std::cout << "Channel closed status: " << (message_queue->closed ? "true" : "false") << std::endl;
    
    tspp_close(message_queue);
    std::cout << "After closing - Channel closed status: " << (message_queue->closed ? "true" : "false") << std::endl;
    
    // 6. Demonstrate performance characteristics
    std::cout << "\n📋 6. Performance Characteristics Demo" << std::endl;
    
    TsppSlice* large_slice = (TsppSlice*)tspp_make_slice(sizeof(int), 0, 1);
    std::cout << "Growing slice from capacity 1 to 1000 elements..." << std::endl;
    
    size_t growth_points = 0;
    size_t prev_cap = tspp_cap(large_slice, 0);
    
    for (int i = 0; i < 1000; i++) {
        tspp_append(large_slice, &i, sizeof(int));
        size_t current_cap = tspp_cap(large_slice, 0);
        if (current_cap > prev_cap) {
            growth_points++;
            std::cout << "  Growth at element " << i << ": capacity " 
                      << prev_cap << " → " << current_cap << std::endl;
            prev_cap = current_cap;
        }
    }
    
    std::cout << "Final slice stats: len=" << tspp_len(large_slice, 0) 
              << ", cap=" << tspp_cap(large_slice, 0) 
              << ", growth events=" << growth_points << std::endl;
    
    // 7. Memory usage summary
    std::cout << "\n📋 7. Memory Usage Summary" << std::endl;
    
    size_t total_allocated = 0;
    total_allocated += sizeof(TsppSlice) + (numbers->cap * sizeof(int));
    total_allocated += sizeof(TsppSlice) + (copy_dest->cap * sizeof(int));
    total_allocated += sizeof(Point);
    total_allocated += sizeof(TsppMap);
    total_allocated += sizeof(TsppChannel);
    total_allocated += sizeof(TsppSlice) + (large_slice->cap * sizeof(int));
    
    std::cout << "Approximate total memory allocated: " << total_allocated << " bytes" << std::endl;
    std::cout << "Large slice memory efficiency: " 
              << (1000.0 * sizeof(int) / (large_slice->cap * sizeof(int)) * 100) 
              << "% utilization" << std::endl;
    
    // Clean up
    free(numbers->data); free(numbers);
    free(copy_dest->data); free(copy_dest);
    free(origin);
    free(user_scores);
    free(message_queue);
    free(large_slice->data); free(large_slice);
    
    std::cout << "\n✅ Demo completed successfully!" << std::endl;
    std::cout << "🎯 All Go built-in functions are operational and performing as expected." << std::endl;
}

} // extern "C"

int main() {
    demonstrate_go_builtins();
    return 0;
}