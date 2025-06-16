#include "MultiLevelMemoryPool.hpp"
#include <cmath>

namespace WebServer {

MultiLevelMemoryPool::MultiLevelMemoryPool() {
    // Initialize each level
    for (size_t i = 0; i < LEVEL_COUNT; ++i) {
        pools_[i].blockSize = getLevelBlockSize(i);
    }
}

MultiLevelMemoryPool::~MultiLevelMemoryPool() {
    // Free all allocated blocks
    for (auto& level : pools_) {
        for (auto block : level.blocks) {
            delete[] block;
        }
    }
}

void* MultiLevelMemoryPool::allocate(size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Find appropriate pool level
    int level = findPoolLevel(size);
    
    if (level < 0) {
        // Requested size is too large for our pools
        char* mem = new char[size + sizeof(size_t)];
        *reinterpret_cast<size_t*>(mem) = LEVEL_COUNT;  // Mark as allocated with new
        return mem + sizeof(size_t);
    }
    
    // Check if we have free slots
    if (!pools_[level].freeSlots.empty()) {
        char* mem = pools_[level].freeSlots.back();
        pools_[level].freeSlots.pop_back();
        return mem + sizeof(size_t);  // Skip the level info
    }
    
    // Allocate a new block
    size_t blockSize = pools_[level].blockSize;
    char* block = new char[blockSize + sizeof(size_t)];  // Add space for level info
    pools_[level].blocks.push_back(block);
    
    // Store the level information
    *reinterpret_cast<size_t*>(block) = level;
    
    // Return pointer to the usable memory
    return block + sizeof(size_t);
}

void MultiLevelMemoryPool::deallocate(void* ptr) {
    if (ptr == nullptr) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Get the original memory address (subtract the size of level info)
    char* mem = static_cast<char*>(ptr) - sizeof(size_t);
    
    // Get the level from the stored information
    size_t level = *reinterpret_cast<size_t*>(mem);
    
    if (level < LEVEL_COUNT) {
        // Add to free slots
        pools_[level].freeSlots.push_back(mem);
    } else {
        // Memory was allocated with new
        delete[] mem;
    }
}

int MultiLevelMemoryPool::findPoolLevel(size_t size) const {
    for (size_t i = 0; i < LEVEL_COUNT; ++i) {
        if (getLevelBlockSize(i) >= size) {
            return static_cast<int>(i);
        }
    }
    return -1;  // Size is too large for our pools
}

size_t MultiLevelMemoryPool::getLevelBlockSize(size_t level) const {
    return MIN_BLOCK_SIZE * (1 << level);  // MIN_BLOCK_SIZE * 2^level
}

} // namespace WebServer