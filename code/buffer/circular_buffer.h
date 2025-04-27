#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <vector>
#include <atomic>
#include <cassert>
#include <cstddef>

/**
 * @brief 线程安全的循环缓冲区实现
 * 
 * 提供高效的循环读写操作，适用于高并发场景
 */
class CircularBuffer {
public:
    /**
     * @brief 构造函数
     * @param size 缓冲区大小
     */
    explicit CircularBuffer(size_t size = 1024);
    
    ~CircularBuffer() = default;

    /**
     * @brief 获取可读字节数
     */
    size_t ReadableBytes() const;
    
    /**
     * @brief 获取可写字节数
     */
    size_t WritableBytes() const;

    /**
     * @brief 读取数据
     * @param buf 输出缓冲区
     * @param len 要读取的长度
     * @return 实际读取的字节数
     */
    size_t Read(void* buf, size_t len);
    
    /**
     * @brief 写入数据
     * @param data 输入数据
     * @param len 数据长度
     * @return 实际写入的字节数
     */
    size_t Write(const void* data, size_t len);

    /**
     * @brief 清空缓冲区
     */
    void Clear();

    /**
     * @brief 获取当前读取位置指针
     * @return 读取位置指针
     */
    const char* Peek() const {
        return &buffer_[read_pos_];
    }

    /**
     * @brief 移动读取位置
     * @param len 移动的字节数
     */
    void Retrieve(size_t len) {
        read_pos_ += len;
    }

    /**
     * @brief 移动读取位置到指定位置
     * @param end 目标位置指针
     */
    void RetrieveUntil(const char* end) {
        read_pos_ = end - &buffer_[0];
    }

    /**
     * @brief 获取当前写入位置指针(常量)
     * @return 写入位置指针
     */
    const char* BeginWriteConst() const {
        return &buffer_[write_pos_];
    }

    /**
     * @brief 获取当前写入位置指针
     * @return 写入位置指针
     */
    char* BeginWrite() {
        return &buffer_[write_pos_];
    }

    /**
     * @brief 标记已写入数据
     * @param len 已写入的字节数
     */
    void HasWritten(size_t len) {
        write_pos_ += len;
    }

    /**
     * @brief 获取缓冲区容量
     * @return 缓冲区容量
     */
    size_t Capacity() const {
        return capacity_;
    }

    /**
     * @brief 调整缓冲区大小
     * @param new_size 新缓冲区大小
     */
    void Resize(size_t new_size) {
        buffer_.resize(new_size);
        capacity_ = new_size;
    }

    /**
     * @brief 获取缓冲区当前大小
     * @return 缓冲区当前大小
     */
    size_t size() const {
        return buffer_.size();
    }

private:
    std::vector<char> buffer_;      // 缓冲区
    std::atomic<size_t> read_pos_;  // 读位置
    std::atomic<size_t> write_pos_; // 写位置
    size_t capacity_;               // 缓冲区容量
};

#endif // CIRCULAR_BUFFER_H