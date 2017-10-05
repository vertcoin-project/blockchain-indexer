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
#include "blockreader.h"
#include "blockchaintypes.h"
#include "hashing.h"
#include <string.h>
#include <memory>
#include <sstream>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>

VtcBlockIndexer::BlockReader::BlockReader(const std::string blocksDir) {
    
    this->blocksDir = blocksDir;
}

VtcBlockIndexer::Block VtcBlockIndexer::BlockReader::readBlock(ScannedBlock block) {
    VtcBlockIndexer::Block fullBlock;

    fullBlock.previousBlockHash = block.previousBlockHash;
    fullBlock.blockHash = block.blockHash;
    
    std::stringstream ss;
    ss << blocksDir << "/" << block.fileName;
    std::ifstream blockFile(ss.str(), std::ios_base::in | std::ios_base::binary);
    
    if(!blockFile.is_open()) {
        std::cerr << "Block file could not be opened";
        exit(0);
    }

    // Seek to the start of the merkle root (we didn't read that while scanning)
    blockFile.seekg(block.filePosition+36, std::ios_base::beg);
    std::unique_ptr<unsigned char> merkleRoot(new unsigned char[32]);
    blockFile.read(reinterpret_cast<char *>(&merkleRoot.get()[0]) , 32);

    std::stringstream ssMR;
    for(int i = 0; i < 32; i++)
    {
        ssMR << std::hex << std::setw(2) << std::setfill('0') << (int)merkleRoot.get()[i];
    }
    fullBlock.merkleRoot = ssMR.str();
    
    blockFile.close();
    return fullBlock;
}
  