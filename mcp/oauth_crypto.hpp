#pragma once
#include "common.hpp"
#ifdef _WIN32
#include <bcrypt.h>
#else
#include <openssl/evp.h>
#include <openssl/rand.h>
#endif
namespace gptframes::mcp {
inline std::string oauth_random_bytes(std::size_t count) {
    if (count > 1024) throw std::invalid_argument("Random size");
    std::string result(count, '\0');
#ifdef _WIN32
    if (BCryptGenRandom(nullptr, reinterpret_cast<PUCHAR>(result.data()),
                        static_cast<ULONG>(count), BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0)
        throw std::runtime_error("Secure random unavailable");
#else
    if (RAND_bytes(reinterpret_cast<unsigned char*>(result.data()), static_cast<int>(count)) != 1)
        throw std::runtime_error("Secure random unavailable");
#endif
    return result;
}
inline std::string oauth_sha256(const std::string& input) {
    if (input.size() > max_message_bytes) throw std::invalid_argument("Hash input too large");
    std::string output(32, '\0');
#ifdef _WIN32
    struct Algorithm {
        BCRYPT_ALG_HANDLE handle = nullptr;
        ~Algorithm() { if (handle) BCryptCloseAlgorithmProvider(handle, 0); }
    } algorithm;
    if (BCryptOpenAlgorithmProvider(&algorithm.handle, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0)
        throw std::runtime_error("SHA256 unavailable");
    DWORD size = 0, received = 0;
    if (BCryptGetProperty(algorithm.handle, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&size),
                          sizeof(size), &received, 0) < 0 || !size || size > 65536)
        throw std::runtime_error("SHA256 initialization failed");
    std::vector<unsigned char> object(size);
    struct Hash {
        BCRYPT_HASH_HANDLE handle = nullptr;
        ~Hash() { if (handle) BCryptDestroyHash(handle); }
    } hash;
    if (BCryptCreateHash(algorithm.handle, &hash.handle, object.data(), size, nullptr, 0, 0) < 0 ||
        BCryptHashData(hash.handle, reinterpret_cast<PUCHAR>(const_cast<char*>(input.data())),
                        static_cast<ULONG>(input.size()), 0) < 0 ||
        BCryptFinishHash(hash.handle, reinterpret_cast<PUCHAR>(output.data()), 32, 0) < 0)
        throw std::runtime_error("SHA256 failed");
#else
    unsigned length = 0;
    if (EVP_Digest(input.data(), input.size(), reinterpret_cast<unsigned char*>(output.data()),
                    &length, EVP_sha256(), nullptr) != 1 || length != 32)
        throw std::runtime_error("SHA256 failed");
#endif
    return output;
}
inline std::string oauth_base64url(const std::string& input) {
    constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string result;
    unsigned value = 0; int bits = -6;
    for (unsigned char byte : input) {
        value = (value << 8) | byte; bits += 8;
        while (bits >= 0) { result += alphabet[(value >> bits) & 63]; bits -= 6; }
    }
    if (bits > -6) result += alphabet[(value << (-bits)) & 63];
    return result;
}
inline std::string oauth_random_token() { return oauth_base64url(oauth_random_bytes(32)); }
inline std::string oauth_digest(const std::string& input) { return oauth_base64url(oauth_sha256(input)); }
}
