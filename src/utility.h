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

#include <vector>
#include <string>

namespace VtcBlockIndexer {
    
    /**
     * The Utility class provides methods to perform various cryptographic operations
     */
    
    class Utility {
        public:
            /** Calculates a single SHA-256 hash over the input 
             * 
             * @param input the value to hash
             */
            static std::vector<unsigned char> sha256(std::vector<unsigned char> input);
            static std::string hashToHex(std::vector<unsigned char> hash);
            static std::string hashToReverseHex(std::vector<unsigned char> hash);
            static std::vector<unsigned char> decompressPubKey(std::vector<unsigned char> compressedKey);
            static std::vector<unsigned char> publicKeyToAddress(std::vector<unsigned char> publicKey, bool testnet);
            static std::vector<unsigned char> ripeMD160(std::vector<unsigned char> in);
            static std::vector<unsigned char> base58(std::vector<unsigned char> in);
            static std::vector<unsigned char> ripeMD160ToP2PKAddress(std::vector<unsigned char> ripeMD, bool testnet);
            static std::vector<unsigned char> ripeMD160ToP2SHAddress(std::vector<unsigned char> ripeMD, bool testnet);
            static std::vector<unsigned char> bech32Address(std::vector<unsigned char> in, bool testnet);
            static std::vector<unsigned char> hexToBytes(std::string hex);
            ~Utility();
            
        private:
            static std::vector<unsigned char> ripeMD160ToAddress(unsigned char versionByte, std::vector<unsigned char> ripeMD);
            static void initECCContextIfNeeded();
            Utility() {}
    };
}
