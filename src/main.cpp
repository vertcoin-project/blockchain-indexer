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

#include <unordered_map>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <memory>
#include <vector>

#include "utility.h"
#include "blockscanner.h"
#include "blockindexer.h"
#include "blockchaintypes.h"
#include "blockreader.h"

using namespace std;

// This map will contain the blocks. The reason for using a vector as value
// is that there can be multiple blocks having the same previousBlockHash in 
// case there is a orphaned block in the block files.
unordered_map<string, vector<VtcBlockIndexer::ScannedBlock>> blocks;
int totalBlocks = 0;
int blockHeight = 0;
string blocksDir;
VtcBlockIndexer::BlockReader blockReader("");
VtcBlockIndexer::BlockIndexer blockIndexer;
/** Uses the blockscanner to scan blocks within a file and add them to the
 * unordered map.
 * 
 * @param fileName The file name of the BLK????.DAT to scan for blocks.
 */
void scanBlocks(string fileName) {
    unique_ptr<VtcBlockIndexer::BlockScanner> blockScanner(new VtcBlockIndexer::BlockScanner(blocksDir, fileName));
    if(blockScanner->open())
    {
        while(blockScanner->moveNext()) {
            totalBlocks++;
            VtcBlockIndexer::ScannedBlock block = blockScanner->scanNextBlock();

            // Create an empty vector inside the unordered map if this previousBlockHash
            // was not found before.
            if(blocks.find(block.previousBlockHash) == blocks.end()) {
                blocks[block.previousBlockHash] = {};
            }

            // Check if a block with the same hash already exists. Unfortunately, I found
            // instances where a block is included in the block files more than once.
            vector<VtcBlockIndexer::ScannedBlock> matchingBlocks = blocks[block.previousBlockHash];
            bool blockFound = false;
            for(VtcBlockIndexer::ScannedBlock matchingBlock : matchingBlocks) {
                if(matchingBlock.blockHash == block.blockHash) {
                    blockFound = true;
                }
            }

            // If the block is not present, add it to the vector.
            if(!blockFound) {
                blocks[block.previousBlockHash].push_back(block);
            }
        }
        blockScanner->close();
    }
}

/** Scans a folder for block files present and passes them to the scanBlocks
 * method
 * 
 * @param dirPath The directory to scan for blockfiles.
 */
void scanBlockFiles(char* dirPath) {
    DIR *dir;
    dirent *ent;

    dir = opendir(dirPath);
    while ((ent = readdir(dir)) != NULL) {
        const string file_name = ent->d_name;

        // Check if the filename starts with "blk"
        string prefix = "blk"; 
        if(strncmp(file_name.c_str(), prefix.c_str(), prefix.size()) == 0)
        {
            scanBlocks(file_name);
        }
    }
    closedir(dir);
}

VtcBlockIndexer::ScannedBlock findLongestChain(vector<VtcBlockIndexer::ScannedBlock> matchingBlocks) {
    vector<string> nextBlockHashes;
    for(uint i = 0; i < matchingBlocks.size(); i++) {
        nextBlockHashes.push_back(matchingBlocks.at(i).blockHash);
    }

    while(true) {
       
        for(uint i = 0; i < nextBlockHashes.size(); i++) {
            int countChains = 0;
            for(uint i = 0; i < nextBlockHashes.size(); i++) {
                if(nextBlockHashes.at(i) != "") {
                    countChains++;
                } 
            }
    
            if(countChains == 1) {
                for(uint i = 0; i < nextBlockHashes.size(); i++) {
                    if(nextBlockHashes.at(i) != "") {
                        return matchingBlocks.at(i);
                    } 
                }
            }

            if(blocks.find(nextBlockHashes.at(i)) == blocks.end()) {
                nextBlockHashes.at(i).assign("");
            } else {
                vector<VtcBlockIndexer::ScannedBlock> matchingBlocks = blocks[nextBlockHashes.at(i)];
                VtcBlockIndexer::ScannedBlock bestBlock = matchingBlocks.at(0);
                if(matchingBlocks.size() > 1) { 
                    bestBlock = findLongestChain(matchingBlocks);
                }
                nextBlockHashes.at(i).assign(bestBlock.blockHash);
            }
        }
    }
}

/** Finds the next block in line (by matching the prevBlockHash which is the
 * key in the unordered_map). Then uses the block processor to do the indexing.
 * Returns the hash of the block that was processed.
 * 
 * @param prevBlockHash the hex hash of the block that was last processed that we should
 * extend the chain onto.
 */
string processNextBlock(string prevBlockHash) {
    // If there is no block present with this hash as previousBlockHash, return an empty 
    // string signaling we're at the end of the chain.
    if(blocks.find(prevBlockHash) == blocks.end()) {
        return "";
    }

    // Find the blocks that match
    vector<VtcBlockIndexer::ScannedBlock> matchingBlocks = blocks[prevBlockHash];

    if(matchingBlocks.size() > 0) {
        VtcBlockIndexer::ScannedBlock bestBlock = matchingBlocks.at(0);
        if(matchingBlocks.size() > 1) { 
            bestBlock = findLongestChain(matchingBlocks);
        } 

        VtcBlockIndexer::Block fullBlock = blockReader.readBlock(bestBlock, blockHeight);
        blockIndexer.indexBlock(fullBlock);
        return bestBlock.blockHash;

    } else {
        // Somehow found an empty vector in the unordered_map. This should not happen. 
        // But just in case, returning an empty value here.
        return "";
    }
}




int main(int argc, char* argv[]) {
    // If user did not supply the command line parameter, show the usage and exit.
    if(argc < 2) {
        cerr << "Usage: vtc_indexer [blocks_dir]" << endl;
        exit(0);
    }
    
    blocksDir.assign(argv[1]);
    blockReader = VtcBlockIndexer::BlockReader(blocksDir);
    cout << "Opening LevelDB..." << endl;
    blockIndexer.open();
    
    string test("\x04\x50\x86\x3A\xD6\x4A\x87\xAE\x8A\x2F\xE8\x3C\x1A\xF1\xA8\x40\x3C\xB5\x3F\x53\xE4\x86\xD8\x51\x1D\xAD\x8A\x04\x88\x7E\x5B\x23\x52\x2C\xD4\x70\x24\x34\x53\xA2\x99\xFA\x9E\x77\x23\x77\x16\x10\x3A\xBC\x11\xA1\xDF\x38\x85\x5E\xD6\xF2\xEE\x18\x7E\x9C\x58\x2B\xA6");
    vector<unsigned char> testVector(test.begin(), test.end());
    testVector.resize(65);
    VtcBlockIndexer::Utility::publicKeyToAddress(testVector);
    
   // return 0;

    cout << "Scanning blocks..." << endl;

    scanBlockFiles(argv[1]);
    
    cout << "Found " << totalBlocks << " blocks. Constructing longest chain..." << endl;

    // The blockchain starts with the genesis block that has a zero hash as Previous Block Hash
    string nextBlock = "0000000000000000000000000000000000000000000000000000000000000000";
    string processedBlock = processNextBlock(nextBlock);
    while(processedBlock != "") {
        if(blockHeight % 100000 == 0) {
            cout << "Constructing chain at height " << blockHeight << " hash: " << processedBlock << endl;
        }

        blockHeight++;
        nextBlock = processedBlock;
        processedBlock = processNextBlock(nextBlock);
    }

    blockIndexer.close();
    cout << "Done. Processed " << blockHeight << " blocks. Have a nice day." << endl;
}