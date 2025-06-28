#include "CompressionUtil.hpp"
#include <zlib.h>
#include <stdexcept>
#include <cstring>

// 压缩数据的内部实现
static std::vector<uint8_t> compressData(const uint8_t* data, size_t size) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, CompressionUtil::COMPRESSION_LEVEL) != Z_OK) {
        throw std::runtime_error("Failed to initialize zlib compression");
    }

    // 设置输入数据
    zs.next_in = const_cast<Bytef*>(data);
    zs.avail_in = static_cast<uInt>(size);

    // 预分配输出缓冲区
    std::vector<uint8_t> output(deflateBound(&zs, static_cast<uLong>(size)));
    zs.next_out = output.data();
    zs.avail_out = static_cast<uInt>(output.size());

    // 执行压缩
    int ret = deflate(&zs, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&zs);
        throw std::runtime_error("Failed to compress data");
    }

    // 调整输出缓冲区大小
    output.resize(zs.total_out);
    deflateEnd(&zs);
    return output;
}

// 解压数据的内部实现
static std::vector<uint8_t> decompressData(const uint8_t* compressedData, size_t compressedSize) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (inflateInit(&zs) != Z_OK) {
        throw std::runtime_error("Failed to initialize zlib decompression");
    }

    // 设置输入数据
    zs.next_in = const_cast<Bytef*>(compressedData);
    zs.avail_in = static_cast<uInt>(compressedSize);

    // 初始输出缓冲区
    std::vector<uint8_t> output(compressedSize * 2); // 初始猜测大小
    size_t totalOut = 0;

    do {
        if (zs.total_out >= output.size()) {
            output.resize(output.size() * 2);
        }

        zs.next_out = output.data() + zs.total_out;
        zs.avail_out = static_cast<uInt>(output.size() - zs.total_out);

        int ret = inflate(&zs, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END) {
            inflateEnd(&zs);
            throw std::runtime_error("Failed to decompress data");
        }
    } while (zs.avail_out == 0);

    output.resize(zs.total_out);
    inflateEnd(&zs);
    return output;
}

// 公共接口实现
std::vector<uint8_t> CompressionUtil::compress(const std::vector<uint8_t>& data) {
    return compressData(data.data(), data.size());
}

std::vector<uint8_t> CompressionUtil::compress(const std::string& data) {
    return compressData(reinterpret_cast<const uint8_t*>(data.data()), data.size());
}

std::vector<uint8_t> CompressionUtil::decompress(const std::vector<uint8_t>& compressedData) {
    return decompressData(compressedData.data(), compressedData.size());
}

std::string CompressionUtil::decompressToString(const std::vector<uint8_t>& compressedData) {
    auto decompressed = decompressData(compressedData.data(), compressedData.size());
    return std::string(reinterpret_cast<const char*>(decompressed.data()), decompressed.size());
}