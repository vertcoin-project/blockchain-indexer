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
#include "mempoolmonitor.h"
#include "blockindexer.h"
#include "blockreader.h"

using namespace std;

namespace VtcBlockIndexer {

/**
 * The BlockFileWatcher class provides methods to watch and scan a blocks directory
 * and process the blockfiles when changes occur.
 */

class BlockFileWatcher {
public:
    /** Constructs a BlockIndexer instance using the given block data directory
     */
    BlockFileWatcher(string blocksDir, const shared_ptr<leveldb::DB> db, const shared_ptr<VtcBlockIndexer::MempoolMonitor> mempoolMonitor);

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
    void scanBlocks(string fileName);

    /** Scans a folder for block files present and passes them to the scanBlocks
     * method
     * 
     * @param dirPath The directory to scan for blockfiles.
     */
    void scanBlockFiles(string dirName);

    /** Orphaned blocks stay in the blockfiles. So this method is created to find out which of the canditate follow-up blocks
     * chain of work behind it.
     * @param matchingBlocks The blocks that should be investigated.
     */
    VtcBlockIndexer::ScannedBlock findLongestChain(vector<VtcBlockIndexer::ScannedBlock> matchingBlocks); 
   
    /** Finds the next block in line (by matching the prevBlockHash which is the
     * key in the unordered_map). Then uses the block processor to do the indexing.
     * Returns the hash of the block that was processed.
     * 
     * @param prevBlockHash the hex hash of the block that was last processed that we should
     * extend the chain onto.
     */     
    string processNextBlock(string prevBlockHash);
    string blocksDir;
    shared_ptr<leveldb::DB> db;
    shared_ptr<VtcBlockIndexer::MempoolMonitor> mempoolMonitor;
    unique_ptr<VtcBlockIndexer::BlockReader> blockReader;
    unique_ptr<VtcBlockIndexer::BlockIndexer> blockIndexer;
    int totalBlocks;
    int blockHeight;
    unordered_map<string, vector<VtcBlockIndexer::ScannedBlock>> blocks;    
    struct timespec maxLastModified;
}; 

}

#endif // BLOCKINDEXER_H_INCLUDED