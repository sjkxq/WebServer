/*
 * @Author       : mark
 * @Date         : 2020-06-26
 * @copyleft Apache 2.0
 */ 

#ifndef BUFFER_H
#define BUFFER_H
#include "circular_buffer.h"
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <string>

/**
 * @brief 线程安全缓冲区类
 * 
 * 基于循环缓冲区实现的高性能线程安全缓冲区
 */
class Buffer {
public:
    /**
     * @brief 构造函数
     * @param initBuffSize 缓冲区初始大小
     */
    explicit Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    /**
     * @brief 获取可写字节数
     * @return 可写字节数
     */
    size_t WritableBytes() const;
    
    /**
     * @brief 获取可读字节数
     * @return 可读字节数
     */
    size_t ReadableBytes() const;
    
    /**
     * @brief 获取可预读字节数
     * @return 可预读字节数
     */
    size_t PrependableBytes() const;

    /**
     * @brief 获取当前读取位置指针
     * @return 读取位置指针
     */
    const char* Peek() const;
    
    /**
     * @brief 确保有足够的可写空间
     * @param len 需要的空间大小
     */
    void EnsureWriteable(size_t len);
    
    /**
     * @brief 标记已写入数据
     * @param len 已写入的字节数
     */
    void HasWritten(size_t len);

    /**
     * @brief 移动读取位置
     * @param len 移动的字节数
     */
    void Retrieve(size_t len);
    
    /**
     * @brief 移动读取位置到指定位置
     * @param end 目标位置指针
     */
    void RetrieveUntil(const char* end);

    /**
     * @brief 清空缓冲区
     */
    void RetrieveAll();
    
    /**
     * @brief 获取所有可读数据并转换为字符串
     * @return 字符串形式的数据
     */
    std::string RetrieveAllToStr();

    /**
     * @brief 获取当前写入位置指针(常量)
     * @return 写入位置指针
     */
    const char* BeginWriteConst() const;
    
    /**
     * @brief 获取当前写入位置指针
     * @return 写入位置指针
     */
    char* BeginWrite();

    /**
     * @brief 追加字符串数据
     * @param str 要追加的字符串
     */
    void Append(const std::string& str);
    
    /**
     * @brief 追加字符数据
     * @param str 字符指针
     * @param len 数据长度
     */
    void Append(const char* str, size_t len);
    
    /**
     * @brief 追加二进制数据
     * @param data 数据指针
     * @param len 数据长度
     */
    void Append(const void* data, size_t len);
    
    /**
     * @brief 追加另一个缓冲区的数据
     * @param buff 源缓冲区
     */
    void Append(const Buffer& buff);

    /**
     * @brief 从文件描述符读取数据
     * @param fd 文件描述符
     * @param Errno 错误码指针
     * @return 读取的字节数
     */
    ssize_t ReadFd(int fd, int* Errno);
    
    /**
     * @brief 向文件描述符写入数据
     * @param fd 文件描述符
     * @param Errno 错误码指针
     * @return 写入的字节数
     */
    ssize_t WriteFd(int fd, int* Errno);

private:
    char* BeginPtr_();
    const char* BeginPtr_() const;
    void MakeSpace_(size_t len);
    CircularBuffer buffer_; // 循环缓冲区实例
};

#endif //BUFFER_H