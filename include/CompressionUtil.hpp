#pragma once

#include <vector>
#include <string>

class CompressionUtil {
public:
    /**
     * @brief 压缩数据
     * @param data 原始数据
     * @return 压缩后的数据
     * @throws std::runtime_error 如果压缩失败
     */
    static std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> compress(const std::string& data);

    /**
     * @brief 解压数据
     * @param compressedData 压缩后的数据
     * @return 原始数据
     * @throws std::runtime_error 如果解压失败
     */
    static std::vector<uint8_t> decompress(const std::vector<uint8_t>& compressedData);
    static std::string decompressToString(const std::vector<uint8_t>& compressedData);

private:
    // 压缩级别 (0-9, 0=不压缩, 9=最大压缩)
    static constexpr int COMPRESSION_LEVEL = Z_DEFAULT_COMPRESSION;
};