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
#include <iomanip>
#include <vector>
#include <secp256k1.h>
#include "crypto/ripemd160.h"
#include "crypto/bech32.h"
#include "coinparams.h"
#include <assert.h>     /* assert */

using namespace std;

namespace
{
    /* Global secp256k1_context object used for verification. */
    secp256k1_context* secp256k1_context_verify = NULL;

    typedef std::vector<uint8_t> data;

    template<int frombits, int tobits, bool pad>
    bool convertbits(data& out, const data& in) {
        int acc = 0;
        int bits = 0;
        const int maxv = (1 << tobits) - 1;
        const int max_acc = (1 << (frombits + tobits - 1)) - 1;
        for (size_t i = 0; i < in.size(); ++i) {
            int value = in[i];
            acc = ((acc << frombits) | value) & max_acc;
            bits += frombits;
            while (bits >= tobits) {
                bits -= tobits;
                out.push_back((acc >> bits) & maxv);
            }
        }
        if (pad) {
            if (bits) out.push_back((acc << (tobits - bits)) & maxv);
        } else if (bits >= frombits || ((acc << (tobits - bits)) & maxv)) {
            return false;
        }
        return true;
    }
}

vector<unsigned char> VtcBlockIndexer::Utility::sha256(vector<unsigned char> input)
{
    unique_ptr<unsigned char> hash(new unsigned char [SHA256_DIGEST_LENGTH]);

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

std::string VtcBlockIndexer::Utility::hashToReverseHex(vector<unsigned char> hash) {
    if(hash.size() == 0) return "";
    stringstream ss;
    for(uint i = hash.size(); i-- > 0;)
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
    if (!secp256k1_ec_pubkey_parse(secp256k1_context_verify, &pubkey, &compressedKey[0], 33)) {
        return {};
    }
    unique_ptr<unsigned char> pub(new unsigned char[65]);
    size_t publen = 65;
    secp256k1_ec_pubkey_serialize(secp256k1_context_verify, pub.get(), &publen, &pubkey, SECP256K1_EC_UNCOMPRESSED);

    
     vector<unsigned char> returnValue (pub.get(), pub.get() + 65);
     return returnValue;
}     


string VtcBlockIndexer::Utility::publicKeyToAddress(vector<unsigned char> publicKey) {
    vector<unsigned char> hashedKey = sha256(publicKey);    
    vector<unsigned char> ripeMD = ripeMD160(hashedKey);

    return ripeMD160ToP2PKAddress(ripeMD);
}

string VtcBlockIndexer::Utility::ripeMD160ToP2PKAddress(vector<unsigned char> ripeMD) {
    return ripeMD160ToAddress(VtcBlockIndexer::CoinParams::p2pkhVersion, ripeMD);
}
string VtcBlockIndexer::Utility::ripeMD160ToP2SHAddress(vector<unsigned char> ripeMD) {
    return ripeMD160ToAddress(VtcBlockIndexer::CoinParams::p2shVersion, ripeMD);
}

string VtcBlockIndexer::Utility::ripeMD160ToAddress(unsigned char versionByte, vector<unsigned char> ripeMD) {
    ripeMD.insert(ripeMD.begin(), versionByte);
    vector<unsigned char> doubleHashedRipeMD = sha256(sha256(ripeMD));
    for(int i = 0; i < 4; i++) {
        ripeMD.push_back(doubleHashedRipeMD.at(i));
    }
    cout << "RipeMD: " << hashToHex(ripeMD) << endl;

    string returnValue = base58(ripeMD);
    return returnValue;

}

vector<unsigned char> VtcBlockIndexer::Utility::hexToBytes(const std::string hex) {
    vector<unsigned char> bytes;
  
    for (unsigned int i = 0; i < hex.length(); i += 2) {
      string byteString = hex.substr(i, 2);
      unsigned char byte = (unsigned char) strtol(byteString.c_str(), NULL, 16);
      bytes.push_back(byte);
    }
  
    return bytes;
}

vector<unsigned char> VtcBlockIndexer::Utility::ripeMD160(vector<unsigned char> in) {
    unsigned char hash[CRIPEMD160::OUTPUT_SIZE];
    CRIPEMD160().Write(in.data(), in.size()).Finalize(hash);
    return vector<unsigned char>(hash, hash + CRIPEMD160::OUTPUT_SIZE);
}

static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

std::string VtcBlockIndexer::Utility::base58(vector<unsigned char> in)
{
    unsigned char* pbegin = &in[0];
    unsigned char* pend = &in[0] + in.size();

    // Skip & count leading zeroes.
    int zeroes = 0;
    int length = 0;
    while (pbegin != pend && *pbegin == 0) {
        pbegin++;
        zeroes++;
    }


    // Allocate enough space in big-endian base58 representation.
    int size = (in.size()-zeroes) * 138 / 100 + 1; // log(256) / log(58), rounded up.

    std::vector<unsigned char> b58(size);
    // Process the bytes.
    while (pbegin != pend) {
        int carry = *pbegin;
        int i = 0;
        // Apply "b58 = b58 * 256 + ch".
        for (std::vector<unsigned char>::reverse_iterator it = b58.rbegin(); (carry != 0 || i < length) && (it != b58.rend()); it++, i++) {
            carry += 256 * (*it);
            *it = carry % 58;
            carry /= 58;
        }

        assert(carry == 0);
        length = i;
        pbegin++;
    }
    // Skip leading zeroes in base58 result.
    std::vector<unsigned char>::iterator it = b58.begin() + (size - length);
    while (it != b58.end() && *it == 0)
        it++;
    // Translate the result into a string.
    std::string str;
    str.reserve(zeroes + (b58.end() - it));
    str.assign(zeroes, '1');
    while (it != b58.end())
        str += pszBase58[*(it++)];
    return str;
}

string VtcBlockIndexer::Utility::bech32Address(vector<unsigned char> in) {
    vector<unsigned char> enc;
    enc.push_back(0); // witness version
    if(convertbits<8, 5, true>(enc, in)) {
        return bech32::Encode(VtcBlockIndexer::CoinParams::bech32Prefix, enc);
    }
    else{
        return "";
    }
}
