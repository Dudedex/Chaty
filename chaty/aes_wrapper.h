#ifndef AES_WRAPPER_H
#define AES_WRAPPER_H

#include <openssl/conf.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <iostream>
#include "../shared/crypto_helper.h"

class AESWrapper
{
    public:
        static std::string encrypt(std::string plaintextStr, std::string keyStr, std::string initVectorStr);
        static std::string decrypt(std::string ciphertextStr, std::string keyStr, std::string initVectorStr);
        static std::string generateRandomKey();
        static std::string generateRandomInitVektor();

};

#endif // AES_WRAPPER_H
