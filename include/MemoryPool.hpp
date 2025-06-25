#pragma once

#include <mutex>
#include <vector>
#include <memory>

namespace WebServer {

/**
 * @brief 用于固定大小内存块的线程安全内存池
 * 
 * @tparam T 要分配的对象类型
 * @tparam BlockSize 每个内存块中的对象数量
 */
template<typename T, size_t BlockSize = 1024>
class MemoryPool {
public:
    MemoryPool();
    ~MemoryPool();

    // 禁用复制
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    /**
     * @brief 为对象分配内存
     * @return 指向已分配内存的指针
     */
    T* allocate();

    /**
     * @brief 释放内存
     * @param ptr 指向要释放的内存的指针
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

    // 当前内存块
    Block* currentBlock_;
    
    // 空闲槽位列表
    Slot* freeSlots_;
    
    // 所有已分配的内存块
    std::vector<std::unique_ptr<Block>> blocks_;
    
    // 用于线程安全的互斥锁
    std::mutex mutex_;
};

} // namespace WebServer