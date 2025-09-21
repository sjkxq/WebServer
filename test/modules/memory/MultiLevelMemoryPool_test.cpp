#include "MultiLevelMemoryPool.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace WebServer;

TEST(MultiLevelMemoryPoolTest, BasicAllocationDeallocation) {
    MultiLevelMemoryPool pool;
    
    // Test allocations of different sizes
    void* p1 = pool.allocate(8);    // Should use level 0 (8 bytes)
    void* p2 = pool.allocate(16);   // Should use level 1 (16 bytes)
    void* p3 = pool.allocate(32);   // Should use level 2 (32 bytes)
    void* p4 = pool.allocate(64);   // Should use level 3 (64 bytes)
    void* p5 = pool.allocate(8192); // Too large, should use operator new
    
    // Check that all pointers are valid
    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    ASSERT_NE(p3, nullptr);
    ASSERT_NE(p4, nullptr);
    ASSERT_NE(p5, nullptr);
    
    // Write to the allocated memory to ensure it's usable
    char* c1 = static_cast<char*>(p1);
    char* c2 = static_cast<char*>(p2);
    char* c3 = static_cast<char*>(p3);
    char* c4 = static_cast<char*>(p4);
    char* c5 = static_cast<char*>(p5);
    
    for (int i = 0; i < 8; ++i) c1[i] = 'a';
    for (int i = 0; i < 16; ++i) c2[i] = 'b';
    for (int i = 0; i < 32; ++i) c3[i] = 'c';
    for (int i = 0; i < 64; ++i) c4[i] = 'd';
    for (int i = 0; i < 100; ++i) c5[i] = 'e';
    
    // Verify the written data
    for (int i = 0; i < 8; ++i) EXPECT_EQ(c1[i], 'a');
    for (int i = 0; i < 16; ++i) EXPECT_EQ(c2[i], 'b');
    for (int i = 0; i < 32; ++i) EXPECT_EQ(c3[i], 'c');
    for (int i = 0; i < 64; ++i) EXPECT_EQ(c4[i], 'd');
    for (int i = 0; i < 100; ++i) EXPECT_EQ(c5[i], 'e');
    
    // Deallocate memory
    pool.deallocate(p1);
    pool.deallocate(p2);
    pool.deallocate(p3);
    pool.deallocate(p4);
    pool.deallocate(p5);
    
    // Allocate again to test reuse of free slots
    void* p6 = pool.allocate(8);   // Should reuse p1's memory
    void* p7 = pool.allocate(16);  // Should reuse p2's memory
    void* p8 = pool.allocate(32);  // Should reuse p3's memory
    void* p9 = pool.allocate(64);  // Should reuse p4's memory
    
    // Check that all pointers are valid
    ASSERT_NE(p6, nullptr);
    ASSERT_NE(p7, nullptr);
    ASSERT_NE(p8, nullptr);
    ASSERT_NE(p9, nullptr);
    
    // Deallocate again
    pool.deallocate(p6);
    pool.deallocate(p7);
    pool.deallocate(p8);
    pool.deallocate(p9);
}

TEST(MultiLevelMemoryPoolTest, ConcurrentAllocation) {
    MultiLevelMemoryPool pool;
    std::vector<std::thread> threads;
    std::vector<void*> pointers(1000);
    
    // Create multiple threads that allocate and deallocate memory
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&pool, &pointers, i]() {
            for (int j = 0; j < 100; ++j) {
                int index = i * 100 + j;
                // Allocate memory of different sizes
                size_t size = 8 << (j % 8);  // Sizes from 8 to 1024
                pointers[static_cast<size_t>(index)] = pool.allocate(size);
                ASSERT_NE(pointers[static_cast<size_t>(index)], nullptr);
                
                // Write to memory to ensure it's usable
                char* mem = static_cast<char*>(pointers[static_cast<size_t>(index)]);
                for (size_t k = 0; k < size; ++k) {
                    mem[k] = static_cast<char>(i);
                }
            }
        });
    }
    
    // Wait for all allocation threads to finish
    for (auto& thread : threads) {
        thread.join();
    }
    
    threads.clear();
    
    // Create threads to verify and deallocate memory
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&pool, &pointers, i]() {
            for (int j = 0; j < 100; ++j) {
                int index = i * 100 + j;
                size_t size = 8 << (j % 8);
                
                // Verify memory contents
                char* mem = static_cast<char*>(pointers[static_cast<size_t>(index)]);
                for (size_t k = 0; k < size; ++k) {
                    EXPECT_EQ(mem[k], static_cast<char>(i));
                }
                
                // Deallocate memory
                pool.deallocate(pointers[static_cast<size_t>(index)]);
            }
        });
    }
    
    // Wait for all deallocation threads to finish
    for (auto& thread : threads) {
        thread.join();
    }
}