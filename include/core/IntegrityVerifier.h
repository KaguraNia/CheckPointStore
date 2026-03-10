#ifndef CHECKSTORE_INTEGRITY_VERIFIER_H
#define CHECKSTORE_INTEGRITY_VERIFIER_H

#include <string>
#include <vector>

namespace checkstore {

    // 数据完整性验证器 (对应文档 4.6 节)
    class IntegrityVerifier {
    public:
        virtual ~IntegrityVerifier() = default;

        // 计算数据的 MD5 哈希
        virtual std::string computeMD5(const std::vector<uint8_t>& data) = 0;

        // 计算文件的 MD5 哈希
        virtual std::string computeMD5ForFile(const std::string& file_path) = 0;

        // 校验数据的 MD5
        virtual bool verifyMD5(const std::vector<uint8_t>& data, const std::string& expected_md5) = 0;

        // 校验文件的 MD5
        virtual bool verifyMD5ForFile(const std::string& file_path, const std::string& expected_md5) = 0;
    };

    // 基于 OpenSSL 的简单实现 (原型用)
    class OpenSSLIntegrityVerifier : public IntegrityVerifier {
    public:
        // ... 实现上述虚函数
    };

} // namespace checkstore
#endif
