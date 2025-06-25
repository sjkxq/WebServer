#ifndef WEBSERVER_TEST_FIXTURES_FIXTURE_FACTORY_H_
#define WEBSERVER_TEST_FIXTURES_FIXTURE_FACTORY_H_

#include "base_fixtures.h"
#include "thread_pool_fixtures.h"
#include "memory_pool_fixtures.h"
#include "logger_fixtures.h"
#include "network_fixtures.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

namespace WebServer {
namespace test {

// 测试夹具配置选项
struct FixtureOptions {
    // 通用选项
    std::string test_name;
    bool verbose{false};
    
    // 线程池选项
    size_t thread_count{4};
    
    // 内存池选项
    size_t block_size{4096};
    
    // 日志选项
    LogLevel log_level{LogLevel::INFO};
    std::string log_file;
    
    // 网络选项
    uint16_t port{0};  // 0表示随机端口
    bool non_blocking{false};
};

// 测试夹具工厂类
class ServerTestFixtureFactory {
public:
    // 创建基础测试夹具
    static std::unique_ptr<ServerTestFixture> CreateFixture(const FixtureOptions& options = {}) {
        return std::make_unique<ServerTestFixture>();
    }
    
    // 创建线程池测试夹具
    static std::unique_ptr<ThreadPoolFixture> CreateThreadPoolFixture(const FixtureOptions& options = {}) {
        auto fixture = std::make_unique<ThreadPoolFixture>();
        // 可以在这里根据options进行额外配置
        return fixture;
    }
    
    // 创建内存池测试夹具
    static std::unique_ptr<MemoryPoolFixture> CreateMemoryPoolFixture(const FixtureOptions& options = {}) {
        auto fixture = std::make_unique<MemoryPoolFixture>();
        // 可以在这里根据options进行额外配置
        return fixture;
    }
    
    // 创建多级内存池测试夹具
    static std::unique_ptr<MultiLevelMemoryPoolFixture> CreateMultiLevelMemoryPoolFixture(const FixtureOptions& options = {}) {
        auto fixture = std::make_unique<MultiLevelMemoryPoolFixture>();
        // 可以在这里根据options进行额外配置
        return fixture;
    }
    
    // 创建日志测试夹具
    static std::unique_ptr<LoggerFixture> CreateLoggerFixture(const FixtureOptions& options = {}) {
        auto fixture = std::make_unique<LoggerFixture>();
        // 可以在这里根据options进行额外配置
        if (options.log_level != LogLevel::INFO) {
            fixture->logger_->setLevel(options.log_level);
        }
        return fixture;
    }
    
    // 创建网络测试夹具
    static std::unique_ptr<NetworkFixture> CreateNetworkFixture(const FixtureOptions& options = {}) {
        auto fixture = std::make_unique<NetworkFixture>();
        // 可以在这里根据options进行额外配置
        return fixture;
    }
    
    // 创建HTTP测试夹具
    static std::unique_ptr<HttpFixture> CreateHttpFixture(const FixtureOptions& options = {}) {
        auto fixture = std::make_unique<HttpFixture>();
        // 可以在这里根据options进行额外配置
        return fixture;
    }
    
    // 根据类型字符串创建测试夹具
    static std::unique_ptr<ServerTestFixture> CreateFixtureByType(const std::string& type, const FixtureOptions& options = {}) {
        static const std::unordered_map<std::string, std::function<std::unique_ptr<ServerTestFixture>(const FixtureOptions&)>> fixture_creators = {
            {"base", [](const FixtureOptions& opt) -> std::unique_ptr<ServerTestFixture> {
                return CreateFixture(opt);
            }},
            {"thread_pool", [](const FixtureOptions& opt) -> std::unique_ptr<ServerTestFixture> {
                return CreateThreadPoolFixture(opt);
            }},
            {"memory_pool", [](const FixtureOptions& opt) -> std::unique_ptr<ServerTestFixture> {
                return CreateMemoryPoolFixture(opt);
            }},
            {"multi_level_memory_pool", [](const FixtureOptions& opt) -> std::unique_ptr<ServerTestFixture> {
                return CreateMultiLevelMemoryPoolFixture(opt);
            }},
            {"logger", [](const FixtureOptions& opt) -> std::unique_ptr<ServerTestFixture> {
                return CreateLoggerFixture(opt);
            }},
            {"network", [](const FixtureOptions& opt) -> std::unique_ptr<ServerTestFixture> {
                return CreateNetworkFixture(opt);
            }},
            {"http", [](const FixtureOptions& opt) -> std::unique_ptr<ServerTestFixture> {
                return CreateHttpFixture(opt);
            }}
        };
        
        auto it = fixture_creators.find(type);
        if (it != fixture_creators.end()) {
            return it->second(options);
        }
        
        // 默认返回基础测试夹具
        return CreateFixture(options);
    }
    
    // 创建组合测试夹具
    class CompositeFixture : public ServerTestFixture {
    public:
        CompositeFixture(std::vector<std::unique_ptr<ServerTestFixture>> fixtures)
            : fixtures_(std::move(fixtures)) {}
        
        void SetUp() override {
            ServerTestFixture::SetUp();
            for (auto& fixture : fixtures_) {
                fixture->SetUp();
            }
        }
        
        void TearDown() override {
            for (auto& fixture : fixtures_) {
                fixture->TearDown();
            }
            ServerTestFixture::TearDown();
        }
        
    private:
        std::vector<std::unique_ptr<ServerTestFixture>> fixtures_;
    };
    
    // 创建组合测试夹具
    static std::unique_ptr<CompositeFixture> CreateCompositeFixture(
        const std::vector<std::string>& fixture_types,
        const FixtureOptions& options = {}) {
        
        std::vector<std::unique_ptr<ServerTestFixture>> fixtures;
        for (const auto& type : fixture_types) {
            fixtures.push_back(CreateFixtureByType(type, options));
        }
        
        return std::make_unique<CompositeFixture>(std::move(fixtures));
    }
};

} // namespace test
} // namespace WebServer

#endif // WEBSERVER_TEST_FIXTURES_FIXTURE_FACTORY_H_