#include "filereading.h"
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

#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <iomanip>

uint64_t readVarInt(std::ifstream& stream)
{
    uint8_t prefix = 0;
    stream.read(reinterpret_cast<char *>(&prefix), sizeof(prefix));
    if(prefix < 253) {
        return prefix;
    }

    if(prefix == 253) {
        uint16_t value = 0;
        stream.read(reinterpret_cast<char *>(&value), sizeof(value));
        return value;
        
    } else if (prefix == 254) {
        uint32_t value = 0;
        stream.read(reinterpret_cast<char *>(&value), sizeof(value));
        return value;

    } else if (prefix == 255) {
        uint64_t value = 0;
        stream.read(reinterpret_cast<char *>(&value), sizeof(value));
        return value;

    } else { 
        // This should never happen, but just in case (and to fix compiler warning)
        return 0;
    }

    
}


std::string readHash(std::ifstream& stream) {
    std::unique_ptr<unsigned char> hash(new unsigned char[32]);
    stream.read(reinterpret_cast<char *>(&hash.get()[0]) , 32);
    
    std::stringstream ss;
    for(int i = 0; i < 32; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash.get()[i];
    }
    return ss.str();
}

std::string readString(std::ifstream& stream) {
    uint64_t length = readVarInt(stream);
    std::unique_ptr<char> data(new char[length]);
    stream.read(reinterpret_cast<char *>(&data.get()[0]) , length);
    std::stringstream ss;
    ss << data.get();
    return ss.str();
    
}
