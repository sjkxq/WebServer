#include "MemoryPool.hpp"

namespace WebServer {

template<typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool() 
    : currentBlock_(nullptr), 
      freeSlots_(nullptr) {
    // Allocate first block
    currentBlock_ = new Block();
    blocks_.emplace_back(currentBlock_);
    
    // Initialize free slots list
    freeSlots_ = &currentBlock_->slots[0];
    for (size_t i = 0; i < BlockSize - 1; ++i) {
        currentBlock_->slots[i].next = &currentBlock_->slots[i + 1];
    }
    currentBlock_->slots[BlockSize - 1].next = nullptr;
}

template<typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool() {
    // All blocks will be automatically deleted by unique_ptr
}

template<typename T, size_t BlockSize>
T* MemoryPool<T, BlockSize>::allocate() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (freeSlots_ == nullptr) {
        // Allocate new block
        currentBlock_ = new Block();
        blocks_.emplace_back(currentBlock_);
        
        // Initialize free slots from new block
        freeSlots_ = &currentBlock_->slots[0];
        for (size_t i = 0; i < BlockSize - 1; ++i) {
            currentBlock_->slots[i].next = &currentBlock_->slots[i + 1];
        }
        currentBlock_->slots[BlockSize - 1].next = nullptr;
    }
    
    // Get next free slot
    Slot* slot = freeSlots_;
    freeSlots_ = freeSlots_->next;
    
    return &slot->element;
}

template<typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::deallocate(T* ptr) {
    if (ptr == nullptr) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Convert pointer back to slot
    Slot* slot = reinterpret_cast<Slot*>(ptr);
    
    // Add slot back to free list
    slot->next = freeSlots_;
    freeSlots_ = slot;
}

// Explicit template instantiations for common types
template class MemoryPool<char>;
template class MemoryPool<int>;
template class MemoryPool<unsigned char>;

} // namespace WebServer