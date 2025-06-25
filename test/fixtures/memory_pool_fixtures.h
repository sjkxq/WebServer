#ifndef WEBSERVER_TEST_FIXTURES_MEMORY_POOL_FIXTURES_H_
#define WEBSERVER_TEST_FIXTURES_MEMORY_POOL_FIXTURES_H_

#include "base_fixtures.h"
#include "../../include/MemoryPool.h"
#include "../../include/MultiLevelMemoryPool.h"
#include <vector>
#include <memory>
#include <random>

namespace WebServer {
namespace test {

// 基础内存池测试夹具
class MemoryPoolFixture : public ServerTestFixture {
protected:
    void SetUp() override {
        ServerTestFixture::SetUp();
        // 创建默认大小的内存池
        memory_pool_ = std::make_unique<MemoryPool>(default_block_size_);
    }

    void TearDown() override {
        // 清理内存池
        if (memory_pool_) {
            memory_pool_.reset();
        }
        ServerTestFixture::TearDown();
    }

    // 分配并验证内存块
    void* AllocateAndVerify(size_t size) {
        void* ptr = memory_pool_->allocate(size);
        EXPECT_NE(ptr, nullptr);
        // 写入一些数据以验证内存可用
        if (ptr) {
            memset(ptr, 0xAB, size);
        }
        return ptr;
    }

    // 进行多次分配和释放操作
    void StressTest(size_t operation_count) {
        std::vector<void*> allocated_blocks;
        allocated_blocks.reserve(operation_count);

        // 随机数生成器
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> size_dis(1, max_allocation_size_);

        for (size_t i = 0; i < operation_count; ++i) {
            // 随机决定是分配还是释放
            if (allocated_blocks.empty() || gen() % 2 == 0) {
                // 分配操作
                size_t size = size_dis(gen);
                void* ptr = AllocateAndVerify(size);
                if (ptr) {
                    allocated_blocks.push_back(ptr);
                }
            } else {
                // 释放操作
                size_t index = gen() % allocated_blocks.size();
                memory_pool_->deallocate(allocated_blocks[index]);
                allocated_blocks.erase(allocated_blocks.begin() + index);
            }
        }

        // 清理剩余的块
        for (void* ptr : allocated_blocks) {
            memory_pool_->deallocate(ptr);
        }
    }

protected:
    std::unique_ptr<MemoryPool> memory_pool_;
    const size_t default_block_size_ = 4096;  // 4KB
    const size_t max_allocation_size_ = 1024;  // 1KB
};

// 多级内存池测试夹具
class MultiLevelMemoryPoolFixture : public ServerTestFixture {
protected:
    void SetUp() override {
        ServerTestFixture::SetUp();
        // 创建多级内存池
        memory_pool_ = std::make_unique<MultiLevelMemoryPool>();
    }

    void TearDown() override {
        if (memory_pool_) {
            memory_pool_.reset();
        }
        ServerTestFixture::TearDown();
    }

    // 在不同级别进行内存分配测试
    void TestAllocationAtLevel(size_t level_size, size_t count) {
        std::vector<void*> blocks;
        blocks.reserve(count);

        for (size_t i = 0; i < count; ++i) {
            void* ptr = memory_pool_->allocate(level_size);
            EXPECT_NE(ptr, nullptr);
            if (ptr) {
                // 验证内存可写
                memset(ptr, 0xCD, level_size);
                blocks.push_back(ptr);
            }
        }

        // 释放所有块
        for (void* ptr : blocks) {
            memory_pool_->deallocate(ptr);
        }
    }

    // 测试跨级别内存分配
    void TestCrossLevelAllocation() {
        std::vector<std::pair<void*, size_t>> allocations;
        const std::vector<size_t> test_sizes = {8, 16, 32, 64, 128, 256, 512, 1024};

        for (size_t size : test_sizes) {
            void* ptr = memory_pool_->allocate(size);
            EXPECT_NE(ptr, nullptr);
            if (ptr) {
                memset(ptr, 0xEF, size);
                allocations.emplace_back(ptr, size);
            }
        }

        // 随机顺序释放
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(allocations.begin(), allocations.end(), gen);

        for (const auto& alloc : allocations) {
            memory_pool_->deallocate(alloc.first);
        }
    }

    // 性能测试
    void PerformanceTest(size_t operation_count) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> size_dis(1, max_test_size_);

        auto start_time = std::chrono::steady_clock::now();

        for (size_t i = 0; i < operation_count; ++i) {
            size_t size = size_dis(gen);
            void* ptr = memory_pool_->allocate(size);
            EXPECT_NE(ptr, nullptr);
            if (ptr) {
                memory_pool_->deallocate(ptr);
            }
        }

        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time).count();

        std::cout << "Performed " << operation_count << " allocations/deallocations in "
                  << duration << " microseconds" << std::endl;
    }

protected:
    std::unique_ptr<MultiLevelMemoryPool> memory_pool_;
    const size_t max_test_size_ = 4096;  // 最大测试分配大小
};

// 参数化多级内存池测试夹具
class MultiLevelMemoryPoolParameterizedFixture :
    public MultiLevelMemoryPoolFixture,
    public ::testing::WithParamInterface<size_t> {
protected:
    void SetUp() override {
        ServerTestFixture::SetUp();
        // 使用参数化的大小创建内存池
        memory_pool_ = std::make_unique<MultiLevelMemoryPool>();
        test_size_ = GetParam();
    }

    size_t GetTestSize() const {
        return test_size_;
    }

private:
    size_t test_size_;
};

// 定义常用的内存块大小参数
const std::vector<size_t> BLOCK_SIZES = {8, 16, 32, 64, 128, 256, 512, 1024};

} // namespace test
} // namespace WebServer

#endif // WEBSERVER_TEST_FIXTURES_MEMORY_POOL_FIXTURES_H_