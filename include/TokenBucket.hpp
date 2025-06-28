#pragma once

#include <chrono>
#include <mutex>

class TokenBucket {
public:
    /**
     * @brief 构造函数
     * @param capacity 桶的最大容量
     * @param tokensPerSecond 每秒添加的令牌数
     */
    TokenBucket(size_t capacity, double tokensPerSecond);

    /**
     * @brief 尝试消费指定数量的令牌
     * @param tokens 请求的令牌数量
     * @return 是否成功获取令牌
     */
    bool tryConsume(size_t tokens = 1);

    /**
     * @brief 获取当前桶中的令牌数量
     * @return 当前令牌数量
     */
    size_t getCurrentTokens() const;

private:
    void refill(); // 根据时间差补充令牌

    mutable std::mutex mutex_;
    const size_t capacity_;          // 桶的最大容量
    const double tokensPerSecond_;   // 每秒添加的令牌数
    size_t tokens_;                  // 当前令牌数量
    std::chrono::steady_clock::time_point lastRefillTime_; // 上次补充令牌的时间
};