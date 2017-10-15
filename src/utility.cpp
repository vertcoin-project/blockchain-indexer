#include "utility.h"
/*  VTC Blockindexer - A utility to build additional indexes to the 
    Vertcoin blockchain by scanning and indexing the blockfiles
    downloaded by Vertcoin Core.
    
    Copyright (C) 2017  Gert-Jaap Glasbergen

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <openssl/sha.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <ck/ckmath.h>
#include <iomanip>
#include <vector>
#include <secp256k1.h>
#include "crypto/ripemd160.h"

using namespace std;

namespace
{
/* Global secp256k1_context object used for verification. */
secp256k1_context* secp256k1_context_verify = NULL;
}

vector<unsigned char> VtcBlockIndexer::Utility::sha256(vector<unsigned char> input)
{
    unique_ptr<char> outputBuffer(new char[65]);
    unique_ptr<unsigned char> hash(new unsigned char [SHA256_DIGEST_LENGTH]);
   
    unique_ptr<unsigned char> intermediate(new unsigned char [SHA256_DIGEST_LENGTH]);
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, &input[0], input.size());
    SHA256_Final(hash.get(), &sha256);
    return vector<unsigned char>(hash.get(), hash.get()+SHA256_DIGEST_LENGTH);
}

std::string VtcBlockIndexer::Utility::hashToHex(vector<unsigned char> hash) {
    stringstream ss;
    for(uint i = 0; i < hash.size(); i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash.at(i);
    }
    return ss.str();
}

void VtcBlockIndexer::Utility::initECCContextIfNeeded() {
    if(secp256k1_context_verify == NULL) {
        secp256k1_context_verify = secp256k1_context_create(SECP256K1_FLAGS_TYPE_CONTEXT | SECP256K1_FLAGS_BIT_CONTEXT_VERIFY);
    }
}

VtcBlockIndexer::Utility::~Utility() {
    if(secp256k1_context_verify != NULL) {
        secp256k1_context_destroy(secp256k1_context_verify);
        secp256k1_context_verify = NULL;
    }
}

vector<unsigned char> VtcBlockIndexer::Utility::decompressPubKey(vector<unsigned char> compressedKey) {
    initECCContextIfNeeded();
    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_parse(secp256k1_context_verify, &pubkey, &compressedKey[0], compressedKey.size())) {
        return {};
    }
    unique_ptr<unsigned char> pub(new unsigned char[65]);
    size_t publen = 65;
    secp256k1_ec_pubkey_serialize(secp256k1_context_verify, pub.get(), &publen, &pubkey, SECP256K1_EC_UNCOMPRESSED);
    return vector<unsigned char>(pub.get(), pub.get() + 65);
}

vector<unsigned char> VtcBlockIndexer::Utility::publicKeyToAddress(vector<unsigned char> publicKey) {

    return {};
}

vector<unsigned char> VtcBlockIndexer::Utility::ripeMD160(vector<unsigned char> in) {
    unsigned char hash[CRIPEMD160::OUTPUT_SIZE];
    CRIPEMD160().Write(in.data(), in.size()).Finalize(hash);
    return vector<unsigned char>(hash, hash + CRIPEMD160::OUTPUT_SIZE);
}

