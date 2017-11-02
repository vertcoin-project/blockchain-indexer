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
using namespace std;

namespace VtcBlockIndexer {
// ScannedBlock is used to store information about block headers obtained while initially scanning through the block files
struct ScannedBlock {
    // The filename (without path) where the block is located in
    string fileName;

    // The position inside the block file where the block header starts
    uint64_t filePosition;

    // The total size of the block
    uint32_t blockSize;

    // The hash of the block. This string is the the "reverse hash" used on block explorers
    string blockHash;

    // The hash of the previous block used to form the chain. This string is the the "reverse hash" used on block explorers
    string previousBlockHash; 

    // Contains true if the scanned block came from the testnet
    bool testnet;
};

// Describes a transaction output inside a blockchain transaction
struct TransactionOutput {
    // The value of the output in Satoshis (0.00000001 VTC)
    uint64_t value;
    
    // The output script in Bitcoinscript
    vector<unsigned char> script;
    
    // The index of the output in the list of outputs
    uint32_t index;
};

// Describes a transaction input inside a blockchain transaction
struct TransactionInput {
    // The index of the input int he list of indexes
    uint32_t index;
    
    // The hash of the transaction whose output is being spent
    string txHash;
    
    // The index of the output inside the transaction being spent
    uint32_t txoIndex;

    // Part of all transactions. A number intended to allow unconfirmed time-locked transactions to be updated before being finalized; not currently used except to disable locktime in a transaction
    uint32_t sequence;
    
    // Indicating if this is a coinbase (Generated coins) input
    bool coinbase;
    
    // The script of the input in Bitcoinscript
    vector<unsigned char> script;
    
    // The witness data for the input
    vector<vector<unsigned char>> witnessData;
};

// Describes a transaction inside a block
struct Transaction {
    // The list of inputs for this transaction
    vector<TransactionInput> inputs;

    // The list of outputs for this transaction
    vector<TransactionOutput> outputs;

    // The hash for the transaction. This is the reverse hash as used on block explorers.
    string txHash;

    // The hash for the witness transaction. Contains a different hash in case the transaction uses SegWit. Will be equal to TXHash otherwise.
    string txWitHash;

    // Position inside the blockfile where this transaction starts
    uint64_t filePosition;

    // Version bit for the transaction
    uint32_t version;

    // Locktime. Transaction cannot be spent until this number of blocks have been confirmed after its initial inclusion in the blockchain
    uint32_t lockTime;
};

// Describes a block
struct Block {
    // The blk????.dat file this block is located in.
    string fileName;

    // The position where the block starts inside the file
    int filePosition;
    
    // The hash of the block. This string is the the "reverse hash" used on block explorers
    string blockHash;
    
    string previousBlockHash;
    
    // The merkle root of the transactions inside this block
    string merkleRoot;

    // The height of the block in the chain
    uint64_t height;

    // The list of transactions inside this block
    vector<Transaction> transactions;

    // Indicates if this block is from the testnet
    bool testnet;
};


}
#endif // BLOCKCHAINTYPES_H_INCLUDED