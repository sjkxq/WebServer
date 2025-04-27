#include "circular_buffer.h"
#include <cstring>

CircularBuffer::CircularBuffer(size_t size) 
    : buffer_(size), 
      read_pos_(0), 
      write_pos_(0), 
      capacity_(size) {
    assert(size > 0);
}

size_t CircularBuffer::ReadableBytes() const {
    if (write_pos_ >= read_pos_) {
        return write_pos_ - read_pos_;
    } else {
        return capacity_ - (read_pos_ - write_pos_);
    }
}

size_t CircularBuffer::WritableBytes() const {
    if (write_pos_ >= read_pos_) {
        return capacity_ - (write_pos_ - read_pos_);
    } else {
        return read_pos_ - write_pos_;
    }
}

size_t CircularBuffer::Read(void* buf, size_t len) {
    if (len == 0 || ReadableBytes() == 0) {
        return 0;
    }
    
    len = std::min(len, ReadableBytes());
    char* dest = static_cast<char*>(buf);
    
    // 计算从读位置到缓冲区末尾的数据量
    size_t first_chunk = std::min(len, capacity_ - read_pos_);
    memcpy(dest, &buffer_[read_pos_], first_chunk);
    
    // 如果有剩余数据需要从缓冲区开头读取
    if (first_chunk < len) {
        memcpy(dest + first_chunk, &buffer_[0], len - first_chunk);
    }
    
    read_pos_ = (read_pos_ + len) % capacity_;
    return len;
}

size_t CircularBuffer::Write(const void* data, size_t len) {
    if (len == 0 || WritableBytes() == 0) {
        return 0;
    }
    
    len = std::min(len, WritableBytes());
    const char* src = static_cast<const char*>(data);
    
    // 计算从写位置到缓冲区末尾的剩余空间
    size_t first_chunk = std::min(len, capacity_ - write_pos_);
    memcpy(&buffer_[write_pos_], src, first_chunk);
    
    // 如果有剩余数据需要写入缓冲区开头
    if (first_chunk < len) {
        memcpy(&buffer_[0], src + first_chunk, len - first_chunk);
    }
    
    write_pos_ = (write_pos_ + len) % capacity_;
    return len;
}

void CircularBuffer::Clear() {
    read_pos_ = 0;
    write_pos_ = 0;
}