#pragma once

#include <array>
#include <mutex>
#include <memory>
#include <vector>
#include "MemoryPool.hpp"

namespace WebServer {

/**
 * @brief A thread-safe multi-level memory pool that supports different sizes of memory allocation
 * 
 * The multi-level memory pool maintains several memory pools of different sizes
 * and automatically chooses the most appropriate pool for each allocation request.
 */
class MultiLevelMemoryPool {
public:
    // Constants for memory pool configuration
    static constexpr size_t MIN_BLOCK_SIZE = 8;      // Minimum block size (in bytes)
    static constexpr size_t MAX_BLOCK_SIZE = 4096;   // Maximum block size (in bytes)
    static constexpr size_t LEVEL_COUNT = 10;        // Number of memory pool levels

    MultiLevelMemoryPool();
    ~MultiLevelMemoryPool();  // 移除 = default，因为我们有自定义实现

    // Disable copy
    MultiLevelMemoryPool(const MultiLevelMemoryPool&) = delete;
    MultiLevelMemoryPool& operator=(const MultiLevelMemoryPool&) = delete;

    /**
     * @brief Allocate memory of specified size
     * @param size Size of memory to allocate (in bytes)
     * @return Pointer to allocated memory, or nullptr if size is too large
     */
    void* allocate(size_t size);

    /**
     * @brief Deallocate memory
     * @param ptr Pointer to memory to deallocate
     */
    void deallocate(void* ptr);

private:
    /**
     * @brief Find the appropriate pool level for the given size
     * @param size Size of memory requested
     * @return Pool level index, or -1 if size is too large
     */
    int findPoolLevel(size_t size) const;

    /**
     * @brief Get the block size for a specific level
     * @param level Pool level index
     * @return Block size for the specified level
     */
    size_t getLevelBlockSize(size_t level) const;

    // Memory pools for different sizes
    struct PoolLevel {
        size_t blockSize;
        std::vector<char*> blocks;  // Store allocated blocks
        std::vector<char*> freeSlots;  // Store free slots
    };

    std::array<PoolLevel, LEVEL_COUNT> pools_;
    std::mutex mutex_;
};

} // namespace WebServer