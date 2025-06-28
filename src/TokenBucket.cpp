#include "TokenBucket.hpp"
#include <algorithm>

TokenBucket::TokenBucket(size_t capacity, double tokensPerSecond)
    : capacity_(capacity),
      tokensPerSecond_(tokensPerSecond),
      tokens_(capacity),
      lastRefillTime_(std::chrono::steady_clock::now()) {}

bool TokenBucket::tryConsume(size_t tokens) {
    std::lock_guard<std::mutex> lock(mutex_);
    refill();

    if (tokens_ < tokens) {
        return false; // 令牌不足
    }

    tokens_ -= tokens;
    return true;
}

size_t TokenBucket::getCurrentTokens() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return tokens_;
}

void TokenBucket::refill() {
    auto now = std::chrono::steady_clock::now();
    double elapsedSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(
        now - lastRefillTime_).count();
    
    size_t newTokens = static_cast<size_t>(elapsedSeconds * tokensPerSecond_);
    if (newTokens > 0) {
        tokens_ = std::min(tokens_ + newTokens, capacity_);
        lastRefillTime_ = now;
    }
}