#include "IntegrityVerifier.h"
#include <openssl/md5.h>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace checkstore {

std::string OpenSSLIntegrityVerifier::computeMD5(const std::vector<uint8_t>& data) {
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5(data.data(), data.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

std::string OpenSSLIntegrityVerifier::computeMD5ForFile(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        return "";
    }

    MD5_CTX ctx;
    MD5_Init(&ctx);

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer))) {
        MD5_Update(&ctx, buffer, file.gcount());
    }
    MD5_Update(&ctx, buffer, file.gcount()); // 最后一块

    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5_Final(hash, &ctx);

    std::stringstream ss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

bool OpenSSLIntegrityVerifier::verifyMD5(const std::vector<uint8_t>& data, const std::string& expected_md5) {
    std::string computed = computeMD5(data);
    return computed == expected_md5;
}

bool OpenSSLIntegrityVerifier::verifyMD5ForFile(const std::string& file_path, const std::string& expected_md5) {
    std::string computed = computeMD5ForFile(file_path);
    return computed == expected_md5 && !computed.empty();
}

} // namespace checkstore
