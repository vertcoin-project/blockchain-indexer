#include "hashing.h"
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

std::string sha256(unsigned char * input, int inputLen)
{
    std::unique_ptr<char> outputBuffer(new char[65]);
    std::unique_ptr<unsigned char> hash(new unsigned char [SHA256_DIGEST_LENGTH]);
   
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, inputLen);

    SHA256_Final(hash.get(), &sha256);

    std::stringstream ss;
    for(int i = 0; i < 32; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash.get()[i];
    }
    return ss.str();

}



std::string doubleSha256(unsigned char * input, int inputLen)
{
    std::unique_ptr<char> outputBuffer(new char[65]);
    std::unique_ptr<unsigned char> hash(new unsigned char [SHA256_DIGEST_LENGTH]);
   
    std::unique_ptr<unsigned char> intermediate(new unsigned char [SHA256_DIGEST_LENGTH]);
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, inputLen);
    SHA256_Final(intermediate.get(), &sha256);
    
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, intermediate.get(), 32);
    SHA256_Final(hash.get(), &sha256);
    
    std::stringstream ss;
    for(int i = 0; i < 32; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash.get()[i];
    }
    return ss.str();
}
