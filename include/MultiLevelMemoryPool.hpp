#pragma once

#include <array>
#include <mutex>
#include <memory>
#include <vector>
#include "MemoryPool.hpp"

namespace WebServer {

/**
 * @brief 支持不同大小内存分配的线程安全多级内存池
 * 
 * 多级内存池维护多个不同大小的内存池，
 * 并自动为每个分配请求选择最合适的内存池。
 */
class MultiLevelMemoryPool {
public:
    // 内存池配置常量
    static constexpr size_t MIN_BLOCK_SIZE = 8;      // 最小块大小（字节）
    static constexpr size_t MAX_BLOCK_SIZE = 4096;   // 最大块大小（字节）
    static constexpr size_t LEVEL_COUNT = 10;        // 内存池级别数量

    MultiLevelMemoryPool();
    ~MultiLevelMemoryPool();  // 移除 = default，因为我们有自定义实现

    // 禁用复制
    MultiLevelMemoryPool(const MultiLevelMemoryPool&) = delete;
    MultiLevelMemoryPool& operator=(const MultiLevelMemoryPool&) = delete;

    /**
     * @brief 分配指定大小的内存
     * @param size 要分配的内存大小（字节）
     * @return 指向已分配内存的指针，如果大小过大则返回nullptr
     */
    void* allocate(size_t size);

    /**
     * @brief 释放内存
     * @param ptr 指向要释放的内存的指针
     */
    void deallocate(void* ptr);

private:
    /**
     * @brief 为给定大小找到合适的内存池级别
     * @param size 请求的内存大小
     * @return 内存池级别索引，如果大小过大则返回-1
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