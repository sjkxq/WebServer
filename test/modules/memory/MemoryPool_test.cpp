#include "MemoryPool.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace WebServer;

TEST(MemoryPoolTest, BasicAllocation) {
    MemoryPool<int> pool;
    
    int* ptr1 = pool.allocate();
    int* ptr2 = pool.allocate();
    
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr1, ptr2);
    
    *ptr1 = 42;
    *ptr2 = 84;
    
    ASSERT_EQ(*ptr1, 42);
    ASSERT_EQ(*ptr2, 84);
    
    pool.deallocate(ptr1);
    pool.deallocate(ptr2);
}

TEST(MemoryPoolTest, ReuseMemory) {
    MemoryPool<int> pool;
    
    // Allocate and store pointer
    int* ptr1 = pool.allocate();
    pool.deallocate(ptr1);
    
    // Allocate again and verify same pointer is returned
    int* ptr2 = pool.allocate();
    ASSERT_EQ(ptr1, ptr2); // Verify memory is reused
    
    // Clean up
    pool.deallocate(ptr2);
}

TEST(MemoryPoolTest, ThreadSafety) {
    MemoryPool<int> pool;
    constexpr int kThreads = 4;
    constexpr int kAllocations = 1000;
    
    std::vector<std::thread> threads;
    std::vector<int*> pointers(kThreads * kAllocations);
    
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([i, &pool, &pointers]() {
            for (int j = 0; j < kAllocations; ++j) {
                int* ptr = pool.allocate();
                *ptr = i * kAllocations + j;
                pointers[static_cast<size_t>(i * kAllocations + j)] = ptr;
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Verify all allocations are unique
    for (size_t i = 0; i < pointers.size(); ++i) {
        for (size_t j = i + 1; j < pointers.size(); ++j) {
            ASSERT_NE(pointers[i], pointers[j]);
        }
    }
    
    // Clean up
    for (auto ptr : pointers) {
        pool.deallocate(ptr);
    }
}