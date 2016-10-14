#include "aes_wrapper.h"


std::string AESWrapper::encrypt(std::string plaintextStr, std::string keyStr, std::string initVectorStr){
    int cipherPlace = (plaintextStr.length()/16 + 1)*16;
    char *ciphertext = new char[cipherPlace];

    int plaintext_len = plaintextStr.length();
    unsigned char *iv = (unsigned char *)initVectorStr.c_str();
    unsigned char *key = (unsigned char *)keyStr.c_str();

    EVP_CIPHER_CTX *ctx;

    int len;
    int ciphertext_len;

    if(!(ctx = EVP_CIPHER_CTX_new())){
        return std::string("");
    }

    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)){
        return std::string("");
    }

    if(1 != EVP_EncryptUpdate(ctx,(unsigned char*) ciphertext, &len, (unsigned char*) plaintextStr.data(), plaintext_len)){
        return std::string("");
    }
    ciphertext_len = len;

    if(1 != EVP_EncryptFinal_ex(ctx,(unsigned char*) ciphertext + len, &len)){
        return std::string("");
    }
    ciphertext_len += len;

    std::string ciphertextStr(reinterpret_cast<const char *>(ciphertext), ciphertext_len);

    /* Clean up */
    delete[] ciphertext;
    EVP_cleanup();
    ERR_free_strings();
    EVP_CIPHER_CTX_free(ctx);

    return ciphertextStr;
}

std::string AESWrapper::decrypt(std::string ciphertextStr, std::string keyStr, std::string initVectorStr)
{
    int cipherPlace = (ciphertextStr.length()/16 +1) * 16;

    char *plaintext = new char[cipherPlace];
    unsigned char *iv = (unsigned char *)initVectorStr.c_str();
    unsigned char *key = (unsigned char *)keyStr.c_str();

    EVP_CIPHER_CTX *ctx;

    int len;

    int plaintext_len;

    if(!(ctx = EVP_CIPHER_CTX_new())){
        return std::string("");
    }

    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)){
        return std::string("");
    }

    if(1 != EVP_DecryptUpdate(ctx,(unsigned char*) plaintext, &len,(unsigned char*) ciphertextStr.data(), ciphertextStr.length())){
        return std::string("");
    }
    plaintext_len = len;

    if(1 != EVP_DecryptFinal_ex(ctx,(unsigned char*) plaintext + len, &len)){
        return std::string("");
    }
    plaintext_len += len;

    std::string plaintextStr(reinterpret_cast<const char *>(plaintext), plaintext_len);
    /* Clean up */
    delete[] plaintext;
    EVP_cleanup();
    ERR_free_strings();
    EVP_CIPHER_CTX_free(ctx);

    return plaintextStr;
}

std::string AESWrapper::generateRandomKey()
{
    return CryptoHelper::generateRandomKey(32);
}

std::string AESWrapper::generateRandomInitVektor()
{
    return CryptoHelper::generateRandomKey(16);
}
