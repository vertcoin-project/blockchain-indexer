#include <fstream>
#include <vector>

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
namespace VtcBlockIndexer {
    
    /**
     * The Util class provides methods to perform various cryptographic operations
     */
    
    class FileReader {
        public:
            /** Reads a varint from a ifstream. The first byte determines the size of  
             *  the int. If it is below 0xFD it's a uint8_t, if it's 0xFD it's followed by
             *  a uint16_t (2 bytes). If it's 0xFE it's followed by a uint32_t (4 bytes), 
             *  and 0xFF means a uint64_t (8 bytes)
             * @param stream the stream to read from 
             */   
            static uint64_t readVarInt(std::ifstream& stream);

            /** Reads a hash (32 bytes) from the ifstream and returns it as hex encoded
             * 
             * @param stream the stream to read from 
             */ 
            static std::string readHash(std::ifstream& stream);

            /** Reads a string (first a VarInt with the length, then the contents) from 
             * the ifstream and returns it as vector<char>
             * 
             * @param stream the stream to read from 
             */ 
            static std::vector<unsigned char> readString(std::ifstream& stream);
    };
}