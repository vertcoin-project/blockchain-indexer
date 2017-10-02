#include "hashing.h"
#include <openssl/sha.h>
#include <iostream>
#include <fstream>

void sha256(unsigned char * input, unsigned char * output)
{
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, sizeof(input));
    SHA256_Final(output, &sha256);
}

void doubleSha256(unsigned char * input, unsigned char * output)
{
    unsigned char intermediate[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, sizeof(input));
    SHA256_Final(intermediate, &sha256);

    SHA256_Init(&sha256);
    SHA256_Update(&sha256, intermediate, sizeof(intermediate));
    SHA256_Final(output, &sha256);

}
