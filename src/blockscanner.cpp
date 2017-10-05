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
#include "hashing.h"
#include <string.h>
#include <memory>
#include <sstream>
#include <string>
#include <iomanip>

const char magic[] = "\xfa\xbf\xb5\xda";

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

    if(this->blockFileStream.eof() || this->blockFileStream.fail()) return false;
    return (memcmp(buffer.get(), magic, 4) == 0);
}

VtcBlockIndexer::ScannedBlock VtcBlockIndexer::BlockScanner::scanNextBlock() {
    VtcBlockIndexer::ScannedBlock block;

    uint32_t blockSize;
    this->blockFileStream.read(reinterpret_cast<char *>(&blockSize), sizeof(blockSize));

    block.fileName = this->blockFileName;
    block.filePosition = this->blockFileStream.tellg();

    std::unique_ptr<unsigned char> blockHeader(new unsigned char[80]);
    this->blockFileStream.read(reinterpret_cast<char *>(&blockHeader.get()[0]) , 80);

    block.blockHash = doubleSha256(blockHeader.get(), 80);
    std::unique_ptr<unsigned char> previousBlockHash(new unsigned char[32]);
    memcpy(previousBlockHash.get(), blockHeader.get()+4, 32);

    std::stringstream ss;
    for(int i = 0; i < 32; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)previousBlockHash.get()[i];
    }
    block.previousBlockHash = ss.str();
    
    this->blockFileStream.seekg(blockSize - 80, std::ios_base::cur);
    
    return block;
}