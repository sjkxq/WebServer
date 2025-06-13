#pragma once

#include <mutex>
#include <vector>
#include <memory>

namespace WebServer {

/**
 * @brief A thread-safe memory pool for fixed-size memory blocks
 * 
 * @tparam T Type of objects to allocate
 * @tparam BlockSize Number of objects per memory block
 */
template<typename T, size_t BlockSize = 1024>
class MemoryPool {
public:
    MemoryPool();
    ~MemoryPool();

    // Disable copy
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    /**
     * @brief Allocate memory for an object
     * @return Pointer to allocated memory
     */
    T* allocate();

    /**
     * @brief Deallocate memory
     * @param ptr Pointer to memory to deallocate
     */
    void deallocate(T* ptr);

private:
    union Slot {
        T element;
        Slot* next;
    };

    struct Block {
        Slot slots[BlockSize];
        Block* next;
    };

    // Current block
    Block* currentBlock_;
    
    // Free slots list
    Slot* freeSlots_;
    
    // All allocated blocks
    std::vector<std::unique_ptr<Block>> blocks_;
    
    // Mutex for thread safety
    std::mutex mutex_;
};

} // namespace WebServer