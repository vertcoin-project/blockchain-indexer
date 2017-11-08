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
#include "blockscanner.h"
#include "utility.h"
#include <string.h>
#include <memory>
#include <sstream>
#include <string>
#include <iomanip>

const char magic[] = "\xfa\xbf\xb5\xda";
const char magicTestnet[] = "\x76\x65\x72\x74";

VtcBlockIndexer::BlockScanner::BlockScanner(const std::string blocksDir, const std::string blockFileName) {
    std::stringstream ss;
    ss << blocksDir << "/" << blockFileName;
    this->blockFilePath = ss.str();
    this->blockFileName = blockFileName;
}

bool VtcBlockIndexer::BlockScanner::open() {
    this->blockFileStream.open(this->blockFilePath, std::ios_base::in | std::ios_base::binary);
    return this->blockFileStream.is_open();
}

bool VtcBlockIndexer::BlockScanner::close() {
    if(!this->blockFileStream.is_open()) return false;
    this->blockFileStream.close();
    return !this->blockFileStream.is_open();
}

bool VtcBlockIndexer::BlockScanner::moveNext() {
    std::unique_ptr<char> buffer(new char[4]);
    this->blockFileStream.read(buffer.get(), 4);

    if(this->blockFileStream.eof()) {
        return false;   
    }

    if(this->blockFileStream.fail()) {
        return false;   
    }

    bool magicMatch = (memcmp(buffer.get(), magic, 4) == 0);
    if(!magicMatch) { 
        magicMatch = (memcmp(buffer.get(), magicTestnet, 4) == 0);
        if(magicMatch) { 
            this->testnet = true;
        }
    } else { 
        this->testnet = false;
    }

    return (magicMatch);
}

VtcBlockIndexer::ScannedBlock VtcBlockIndexer::BlockScanner::scanNextBlock() {
    VtcBlockIndexer::ScannedBlock block;

    uint32_t blockSize;
    this->blockFileStream.read(reinterpret_cast<char *>(&blockSize), sizeof(blockSize));

    // Store the file name and position of the block inside the struct so we can
    // use that to read the actual block later after sorting the blockchain.
    block.fileName = this->blockFileName;
    block.filePosition = this->blockFileStream.tellg();

    vector<unsigned char> blockHeader(80);
    this->blockFileStream.read(reinterpret_cast<char *>(&blockHeader[0]) , 80);

    block.blockHash = VtcBlockIndexer::Utility::hashToReverseHex(VtcBlockIndexer::Utility::sha256(VtcBlockIndexer::Utility::sha256(blockHeader)));
    vector<unsigned char> previousBlockHash(32);
    memcpy(&previousBlockHash[0], &blockHeader[4], 32);
    block.previousBlockHash =  VtcBlockIndexer::Utility::hashToReverseHex(previousBlockHash);
    
    this->blockFileStream.seekg(blockSize - 80, std::ios_base::cur);
    
    block.testnet = this->testnet;
    return block;
}