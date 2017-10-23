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


#ifndef BLOCKFILEWATCHER_H_INCLUDED
#define BLOCKFILEWATCHER_H_INCLUDED

#include <iostream>
#include <fstream>
#include <unordered_map>
#include "leveldb/db.h"
#include "leveldb/write_batch.h"
#include "blockchaintypes.h"

namespace VtcBlockIndexer {

/**
 * The BlockFileWatcher class provides methods to watch and scan a blocks directory
 * and process the blockfiles when changes occur.
 */

class BlockFileWatcher {
public:
    /** Constructs a BlockIndexer instance using the given block data directory
     */
    BlockFileWatcher(std::string blocksDir, leveldb::DB* dbInstance);

    /** Starts watching the blocksdir for changes and will execute an incremental
     * indexing when files have changed */
    void startWatcher();

    /** Updates the blockchain index incrementally */
    void updateIndex();
    
private:
    /** Uses the blockscanner to scan blocks within a file and add them to the
     * unordered map.
     * 
     * @param fileName The file name of the BLK????.DAT to scan for blocks.
     */
    void scanBlocks(std::string fileName);
    void scanBlockFiles(std::string dirName);
    VtcBlockIndexer::ScannedBlock findLongestChain(std::vector<VtcBlockIndexer::ScannedBlock> matchingBlocks);      
    std::string processNextBlock(std::string prevBlockHash);
    std::string blocksDir;
    leveldb::DB* db;
    int totalBlocks;
    int blockHeight;
    unordered_map<string, vector<VtcBlockIndexer::ScannedBlock>> blocks;    
    struct timespec maxLastModified;
}; 

}

#endif // BLOCKINDEXER_H_INCLUDED