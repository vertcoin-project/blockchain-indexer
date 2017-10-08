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

#ifndef BLOCKCHAINTYPES_H_INCLUDED
#define BLOCKCHAINTYPES_H_INCLUDED

#include <stdlib.h>
#include <vector>
#include <ck/ckmath.h>
namespace VtcBlockIndexer {
struct ScannedBlock {
    std::string fileName;
    int filePosition;
    uint32_t blockSize;
    std::string blockHash;
    std::string previousBlockHash; 
};
struct TransactionOutput {
    uint64_t value;
    std::string script;
    uint32_t index;
};

struct TransactionInput {
    uint32_t index;
    std::string txHash;
    uint32_t txoIndex;
    uint32_t sequence;
    std::string script;
};

struct Transaction {
    std::vector<TransactionInput> inputs;
    std::vector<TransactionOutput> outputs;
    std::string txHash;
    uint32_t version;
    uint32_t lockTime;
};

struct Block {
    std::string blockHash;
    std::string merkleRoot;
    std::string previousBlockHash;
    int time;
    std::vector<Transaction> transactions;
};


}
#endif // BLOCKCHAINTYPES_H_INCLUDED