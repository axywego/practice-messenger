#pragma once

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <vector>
#include <cstdint>
#include <memory>

const std::vector<uint8_t> SERVER_KEY(32, 0x42);

std::vector<uint8_t> encryptAES(const std::vector<uint8_t>& plaintext, const std::vector<uint8_t>& key) {
    if (key.size() != 32) {
        throw std::invalid_argument("Key must be 32 bytes for AES-256");
    }
    
    std::vector<uint8_t> iv(16);
    if (RAND_bytes(iv.data(), iv.size()) != 1) {
        throw std::runtime_error("Failed to generate IV using RAND_bytes");
    }
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context");
    }
    
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption");
    }
    
    std::vector<uint8_t> ciphertext(plaintext.size() + 16);
    int out_len = 0;
    int total_len = 0;
    
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &out_len, plaintext.data(), plaintext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Encryption failed");
    }
    total_len = out_len;
    
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + total_len, &out_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Finalization failed");
    }
    total_len += out_len;
    
    ciphertext.resize(total_len);
    EVP_CIPHER_CTX_free(ctx);
    
    std::vector<uint8_t> result;
    result.reserve(iv.size() + ciphertext.size());
    result.insert(result.end(), iv.begin(), iv.end());
    result.insert(result.end(), ciphertext.begin(), ciphertext.end());
    
    return result;
}

std::vector<uint8_t> decryptAES(const std::vector<uint8_t>& ciphertext_with_iv, const std::vector<uint8_t>& key) {

    if (key.size() != 32) {
        throw std::invalid_argument("Key must be 32 bytes for AES-256");
    }
    
    if (ciphertext_with_iv.size() < 16) {
        throw std::invalid_argument("Ciphertext too short: less than 16 bytes for IV");
    }
    
    std::vector<uint8_t> iv(ciphertext_with_iv.begin(), 
                            ciphertext_with_iv.begin() + 16);
    
    std::vector<uint8_t> ciphertext(ciphertext_with_iv.begin() + 16,
                                     ciphertext_with_iv.end());
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context");
    }
    
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize decryption");
    }
    
    std::vector<uint8_t> plaintext(ciphertext.size());
    int out_len = 0;
    int total_len = 0;
    
    if (EVP_DecryptUpdate(ctx, plaintext.data(), &out_len, ciphertext.data(), ciphertext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption failed");
    }
    total_len = out_len;
    
    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + total_len, &out_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption finalization failed - wrong key or corrupted data");
    }
    total_len += out_len;
    
    plaintext.resize(total_len);
    EVP_CIPHER_CTX_free(ctx);
    
    return plaintext;
}