#include "hashing.h"
#include <openssl/sha.h>

unsigned char * sha256(unsigned char * str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str, sizeof(str));
    SHA256_Final(hash, &sha256);
    return hash;
}
