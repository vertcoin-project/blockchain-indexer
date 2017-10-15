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


#ifndef BLOCKINDEXER_H_INCLUDED
#define BLOCKINDEXER_H_INCLUDED

#include <iostream>
#include <fstream>

#include "blockchaintypes.h"

namespace VtcBlockIndexer {

/**
 * The BlockIndexer class provides methods to index a block that was fully 
 * read (so including its transactions). It will index the necessary elements
 * to be able to query balances per address, blocks by hash and index, and 
 * data to handle reorgs.
 */

class BlockIndexer {
public:
    /** Constructs a BlockIndexer instance using the given block data directory
     */
    BlockIndexer();

    /** Open the LevelDB database
     */
    bool open();

    /** Closes the LevelDB database
     */
    bool close();

    /** Indexes the contents of the block
     */
    bool indexBlock(Block block);

private:
    /** Returns the next index to use for storing the TXO
     */
    int getNextTxoIndex(std::string prefix);
};

}

#endif // BLOCKINDEXER_H_INCLUDED