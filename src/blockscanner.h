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


#ifndef BLOCKSCANNER_H_INCLUDED
#define BLOCKSCANNER_H_INCLUDED

#include <iostream>
#include <fstream>

#include "blockchaintypes.h"

namespace VtcBlockIndexer {

/**
 * The BlockScanner class provides methods to scan blk????.dat files
 * for block data. It only scans blocks, and reads its header. No 
 * block data like transactions are read.
 */

class BlockScanner {
public:
    /** Constructs a BlockScanner instance using the given block data file
     * 
     * @param blocksDir required Directory where the blockfile is located.
     * @param blockFileName required Filename of the block file to read.
     */
    BlockScanner(const std::string blocksDir, const std::string blockFileName);
     
    /** Opens the file for reading and allows scanning for blocks
     */
    bool open();

    /** Tries reading the magic string from the file stream and move the
     *  file pointer to the start of the block following it. If the magic
     *  string was not found, either because of the EOF or the wrong 
     *  sequence was found, there is no block and this function will return
     *  false.
     */
    bool moveNext();

    /** Scans the next block. Scanning only reads the header and returns a
     *  ScannedBlock struct that contains the file the block was found in,
     *  the start position and length inside that file, its hash and 
     *  previousBlockHash. A collection of ScannedBlock objects should be
     *  sufficient to construct the blockchain.
     */
    ScannedBlock scanNextBlock();

    /** Closes the file
     */
    bool close();

private:

    /** Reference to the stream when the blockfile was opened
     */
    std::ifstream blockFileStream;
    
    /** Full path to the blockfile
     */
    std::string blockFilePath;
    
    /** File name only of the blockfile
     */
    std::string blockFileName;

    /** When the scanner finds magic bytes from testnet it will toggle this to true */
    bool testnet;
    
};

}

#endif // BLOCKSCANNER_H_INCLUDED