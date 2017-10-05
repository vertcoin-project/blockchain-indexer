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


#ifndef BLOCKREADER_H_INCLUDED
#define BLOCKREADER_H_INCLUDED

#include <iostream>
#include <fstream>

#include "blockchaintypes.h"

namespace VtcBlockIndexer {

/**
 * The BlockReader class provides methods to read in the full details of 
 * a block and its transactions from the block file based on a ScannedBlock 
 */

class BlockReader {
public:
    /** Constructs a BlockReader instance using the given block data directory
     * 
     * @param blocksDir required Directory where the blockfiles are located.
     */
    BlockReader(const std::string blocksDir);
     
    /** Reads the entire contents of the block that was scanned
     */
    Block readBlock(ScannedBlock block);

private:

    /** Directory containing the blocks
     */
    std::string blocksDir; 
};

}

#endif // BLOCKREADER_H_INCLUDED