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
#include "filereading.h"
#include "blockchaintypes.h"
#include "hashing.h"
#include <string.h>
#include <memory>
#include <sstream>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>

using namespace std;

VtcBlockIndexer::BlockReader::BlockReader(const string blocksDir) {
    
    this->blocksDir = blocksDir;
}

VtcBlockIndexer::Block VtcBlockIndexer::BlockReader::readBlock(ScannedBlock block, uint64_t blockHeight) {
    VtcBlockIndexer::Block fullBlock;

    fullBlock.height = blockHeight;
    fullBlock.previousBlockHash = block.previousBlockHash;
    fullBlock.blockHash = block.blockHash;
    
    stringstream ss;
    ss << blocksDir << "/" << block.fileName;
    ifstream blockFile(ss.str(), ios_base::in | ios_base::binary);
    
    if(!blockFile.is_open()) {
        cerr << "Block file could not be opened";
        exit(0);
    }

    // Seek to the start of the merkle root (we didn't read that while scanning)
    blockFile.seekg(block.filePosition+36, ios_base::beg);
    unique_ptr<unsigned char> merkleRoot(new unsigned char[32]);
    blockFile.read(reinterpret_cast<char *>(&merkleRoot.get()[0]) , 32);

    stringstream ssMR;
    for(int i = 0; i < 32; i++)
    {
        ssMR << hex << setw(2) << setfill('0') << (int)merkleRoot.get()[i];
    }
    fullBlock.merkleRoot = ssMR.str();

    // Find number of transactions
    blockFile.seekg(block.filePosition+80, ios_base::beg);
    uint64_t txCount = readVarInt(blockFile);
    
    fullBlock.transactions = {};

    for(uint64_t tx = 0; tx < txCount; tx++) {
        bool segwit = false;

        VtcBlockIndexer::Transaction transaction;
        blockFile.read(reinterpret_cast<char *>(&transaction.version), sizeof(transaction.version));

        // determine if this is a segwit tx
        // https://bitcoincore.org/en/segwit_wallet_dev/
        unique_ptr<unsigned char> segwitMarker(new unsigned char[2]);
        blockFile.read(reinterpret_cast<char *>(&segwitMarker.get()[0]) , 2);
        segwit = (segwitMarker.get()[0] == 0x00 && segwitMarker.get()[1] != 0x00);
        
        // If the segwit marker is not found, the number of inputs is located in its place
        // so rewind the stream to continue.
        if(!segwit) blockFile.seekg(-2, ios_base::cur);
        
        transaction.inputs = {};

        uint64_t inputCount = readVarInt(blockFile);
        
        for(uint64_t input = 0; input < inputCount; input++) {
            VtcBlockIndexer::TransactionInput txInput;
            txInput.txHash = readHash(blockFile);
            blockFile.read(reinterpret_cast<char *>(&txInput.txoIndex), sizeof(txInput.txoIndex));
            txInput.script = readString(blockFile);
            blockFile.read(reinterpret_cast<char *>(&txInput.sequence), sizeof(txInput.sequence));
            txInput.index = input;
            transaction.inputs.push_back(txInput);
        }
        
        uint64_t outputCount = readVarInt(blockFile);
        transaction.outputs = {};
        for(uint64_t output = 0; output < outputCount; output++) {
            VtcBlockIndexer::TransactionOutput txOutput;
            blockFile.read(reinterpret_cast<char *>(&txOutput.value), sizeof(txOutput.value));
            txOutput.script = readString(blockFile);
            txOutput.index = output;
            transaction.outputs.push_back(txOutput);
        }

        if(segwit) {
            for(uint64_t input = 0; input < inputCount; input++) {
                uint64_t witnessItems = readVarInt(blockFile);
                if(witnessItems > 0) {
                    transaction.inputs.at(input).witnessData = {};
                    for(uint64_t witnessItem = 0; witnessItem < witnessItems; witnessItem++) {
                        string witnessData = readString(blockFile);
                        transaction.inputs.at(input).witnessData.push_back(witnessData);
                    }
                }
            }
        }

        blockFile.read(reinterpret_cast<char *>(&transaction.lockTime), sizeof(transaction.lockTime));
    }
    
    blockFile.close();
    return fullBlock;
}
  