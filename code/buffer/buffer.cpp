/*
 * @Author       : mark
 * @Date         : 2020-06-26
 * @copyleft Apache 2.0
 */ 
#include "buffer.h"
#include "circular_buffer.h"
#include <string>
#include <cstddef>

/**
 * @brief 构造函数
 * @param initBuffSize 缓冲区初始大小
 */
Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize) {}

/**
 * @brief 获取可读字节数
 * @return 可读字节数
 */
size_t Buffer::ReadableBytes() const {
    return buffer_.ReadableBytes();
}

/**
 * @brief 获取可写字节数
 * @return 可写字节数
 */
size_t Buffer::WritableBytes() const {
    return buffer_.WritableBytes();
}

/**
 * @brief 获取可预读字节数
 * @return 可预读字节数
 */
size_t Buffer::PrependableBytes() const {
    return 0; // 循环缓冲区不支持预读
}

/**
 * @brief 获取当前读取位置指针
 * @return 读取位置指针
 */
const char* Buffer::Peek() const {
    return buffer_.Peek();
}

/**
 * @brief 移动读取位置
 * @param len 移动的字节数
 */
void Buffer::Retrieve(size_t len) {
    buffer_.Retrieve(len);
}

/**
 * @brief 移动读取位置到指定位置
 * @param end 目标位置指针
 */
void Buffer::RetrieveUntil(const char* end) {
    buffer_.RetrieveUntil(end);
}

/**
 * @brief 清空缓冲区
 */
void Buffer::RetrieveAll() {
    buffer_.Clear();
}

/**
 * @brief 获取所有可读数据并转换为字符串
 * @return 字符串形式的数据
 */
std::string Buffer::RetrieveAllToStr() {
    std::string str(buffer_.Peek(), buffer_.ReadableBytes());
    buffer_.Clear();
    return str;
}

/**
 * @brief 获取当前写入位置指针(常量)
 * @return 写入位置指针
 */
const char* Buffer::BeginWriteConst() const {
    return buffer_.BeginWriteConst();
}

/**
 * @brief 获取当前写入位置指针
 * @return 写入位置指针
 */
char* Buffer::BeginWrite() {
    return buffer_.BeginWrite();
}

/**
 * @brief 标记已写入数据
 * @param len 已写入的字节数
 */
void Buffer::HasWritten(size_t len) {
    buffer_.HasWritten(len);
}

/**
 * @brief 追加字符串数据
 * @param str 要追加的字符串
 */
void Buffer::Append(const std::string& str) {
    buffer_.Write(str.data(), str.length());
}

/**
 * @brief 追加二进制数据
 * @param data 数据指针
 * @param len 数据长度
 */
void Buffer::Append(const void* data, size_t len) {
    buffer_.Write(data, len);
}

/**
 * @brief 追加字符数据
 * @param str 字符指针
 * @param len 数据长度
 */
void Buffer::Append(const char* str, size_t len) {
    buffer_.Write(str, len);
}

/**
 * @brief 追加另一个缓冲区的数据
 * @param buff 源缓冲区
 */
void Buffer::Append(const Buffer& buff) {
    buffer_.Write(buff.Peek(), buff.ReadableBytes());
}

/**
 * @brief 确保有足够的可写空间
 * @param len 需要的空间大小
 */
void Buffer::EnsureWriteable(size_t len) {
    if(buffer_.WritableBytes() < len) {
        buffer_.Resize(buffer_.Capacity() * 2); // 自动扩容
    }
}

ssize_t Buffer::ReadFd(int fd, int* saveErrno) {
    char buff[65535];
    struct iovec iov[2];
    const size_t writable = WritableBytes();
    /* 分散读， 保证数据全部读完 */
    iov[0].iov_base = buffer_.BeginWrite();
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);
    if(len < 0) {
        *saveErrno = errno;
    }
    else if(static_cast<size_t>(len) <= writable) {
        buffer_.HasWritten(len);
    }
    else {
        buffer_.HasWritten(buffer_.Capacity());
        Append(buff, len - writable);
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int* saveErrno) {
    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, Peek(), readSize);
    if(len < 0) {
        *saveErrno = errno;
        return len;
    } 
    buffer_.Retrieve(len);
    return len;
}

char* Buffer::BeginPtr_() {
    return buffer_.BeginWrite();
}

const char* Buffer::BeginPtr_() const {
    return buffer_.BeginWriteConst();
}

void Buffer::MakeSpace_(size_t len) {
    if(WritableBytes() + PrependableBytes() < len) {
        buffer_.Resize(buffer_.Capacity() + len + 1);
    }
}