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
#include "filereader.h"
#include "blockchaintypes.h"
#include "utility.h"
#include <string.h>
#include <memory>
#include <sstream>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

VtcBlockIndexer::BlockReader::BlockReader(const string blocksDir) {
    
    this->blocksDir = blocksDir;
}

std::vector<unsigned char> VtcBlockIndexer::BlockReader::readRawBlockHeader(string fileName, uint64_t filePosition) {
    stringstream ss;
    ss << blocksDir << "/" << fileName;
    ifstream blockFile(ss.str(), ios_base::in | ios_base::binary);
    vector<unsigned char> blockHeader(80);
    blockFile.read(reinterpret_cast<char *>(&blockHeader[0]) , 80);
    blockFile.close();
    return blockHeader;
}
    

VtcBlockIndexer::Block VtcBlockIndexer::BlockReader::readBlock(string fileName, uint64_t filePosition, uint64_t blockHeight, bool headerOnly) {
    VtcBlockIndexer::Block fullBlock;

    fullBlock.fileName = fileName;
    fullBlock.filePosition = filePosition;
    fullBlock.height = blockHeight;
    
    stringstream ss;
    ss << blocksDir << "/" << fileName;
    ifstream blockFile(ss.str(), ios_base::in | ios_base::binary);
    
    if(!blockFile.is_open()) {
        cerr << "Block file could not be opened" << endl;
        exit(0);
    }

    blockFile.seekg(filePosition, ios_base::beg);
    vector<unsigned char> blockHeader(80);
    blockFile.read(reinterpret_cast<char *>(&blockHeader[0]) , 80);
    fullBlock.blockHash = VtcBlockIndexer::Utility::hashToReverseHex(VtcBlockIndexer::Utility::sha256(VtcBlockIndexer::Utility::sha256(blockHeader)));
   
    blockFile.seekg(filePosition, ios_base::beg);
    
    blockFile.read(reinterpret_cast<char *>(&fullBlock.version), sizeof(fullBlock.version));
    fullBlock.previousBlockHash = VtcBlockIndexer::Utility::hashToReverseHex(VtcBlockIndexer::FileReader::readHash(blockFile));
    fullBlock.merkleRoot = VtcBlockIndexer::Utility::hashToReverseHex(VtcBlockIndexer::FileReader::readHash(blockFile));
    blockFile.read(reinterpret_cast<char *>(&fullBlock.time), sizeof(fullBlock.time));
    blockFile.read(reinterpret_cast<char *>(&fullBlock.bits), sizeof(fullBlock.bits));
    blockFile.read(reinterpret_cast<char *>(&fullBlock.nonce), sizeof(fullBlock.nonce));
    
    if(!headerOnly) {
        // Find number of transactions
        blockFile.seekg(filePosition+80, ios_base::beg);
        uint64_t txCount = VtcBlockIndexer::FileReader::readVarInt(blockFile);
        fullBlock.transactions = {};
        for(uint64_t tx = 0; tx < txCount; tx++) {
            VtcBlockIndexer::Transaction transaction = readTransaction(blockFile);
            fullBlock.transactions.push_back(transaction);
        }
    }
    uint64_t endPosBlock = blockFile.tellg();
    fullBlock.byteSize = endPosBlock - filePosition;
    blockFile.close();
    return fullBlock;
}

VtcBlockIndexer::Transaction VtcBlockIndexer::BlockReader::readTransaction(istream& blockFile) {
    bool segwit = false;
    
    VtcBlockIndexer::Transaction transaction;
    uint64_t startPosTx = blockFile.tellg();
    
    transaction.filePosition = startPosTx;
    blockFile.read(reinterpret_cast<char *>(&transaction.version), sizeof(transaction.version));
    
    // determine if this is a segwit tx
    // https://bitcoincore.org/en/segwit_wallet_dev/
    unique_ptr<unsigned char> segwitMarker(new unsigned char[2]);
    blockFile.read(reinterpret_cast<char *>(&segwitMarker.get()[0]) , 2);
    segwit = (segwitMarker.get()[0] == 0x00 && segwitMarker.get()[1] != 0x00);
    
    // If the segwit marker is not found, the number of inputs is located in its place
    // so rewind the stream to continue.
    if(!segwit) blockFile.seekg(-2, ios_base::cur);
    
    uint64_t startPosInputs = blockFile.tellg();

    transaction.inputs = {};

    uint64_t inputCount = VtcBlockIndexer::FileReader::readVarInt(blockFile);
    
    for(uint64_t input = 0; input < inputCount; input++) {
        VtcBlockIndexer::TransactionInput txInput;
        txInput.txHash = VtcBlockIndexer::Utility::hashToReverseHex(VtcBlockIndexer::FileReader::readHash(blockFile));
        blockFile.read(reinterpret_cast<char *>(&txInput.txoIndex), sizeof(txInput.txoIndex));
        txInput.script = VtcBlockIndexer::FileReader::readString(blockFile);
        blockFile.read(reinterpret_cast<char *>(&txInput.sequence), sizeof(txInput.sequence));
        txInput.index = input;
        txInput.coinbase = (input == 0 && txInput.txHash == "0000000000000000000000000000000000000000000000000000000000000000" && txInput.txoIndex == 4294967295);
        transaction.inputs.push_back(txInput);
    }
    
    uint64_t outputCount = VtcBlockIndexer::FileReader::readVarInt(blockFile);
    transaction.outputs = {};
    for(uint64_t output = 0; output < outputCount; output++) {
        VtcBlockIndexer::TransactionOutput txOutput;
        blockFile.read(reinterpret_cast<char *>(&txOutput.value), sizeof(txOutput.value));
        txOutput.script = VtcBlockIndexer::FileReader::readString(blockFile);
        txOutput.index = output;
        transaction.outputs.push_back(txOutput);
    }

    uint64_t endPosOutputs = blockFile.tellg();
    

    if(segwit) {
        for(uint64_t input = 0; input < inputCount; input++) {
            uint64_t witnessItems = VtcBlockIndexer::FileReader::readVarInt(blockFile);
            if(witnessItems > 0) {
                transaction.inputs.at(input).witnessData = {};
                for(uint64_t witnessItem = 0; witnessItem < witnessItems; witnessItem++) {
                    vector<unsigned char> witnessData = VtcBlockIndexer::FileReader::readString(blockFile);
                    transaction.inputs.at(input).witnessData.push_back(witnessData);
                }
            }
        }
    }

    blockFile.read(reinterpret_cast<char *>(&transaction.lockTime), sizeof(transaction.lockTime));

    uint64_t endPosTx = blockFile.tellg();

    blockFile.seekg(startPosTx, ios_base::beg);

    // The tx hash must still be calculated over the original serialization format.
    // That's why this seems a bit overcomplex
    uint64_t txitxoLength = endPosOutputs-startPosInputs;
    std::vector<unsigned char> txHashBytes(
        4 + // version
        txitxoLength +
        4 // locktime
    );
    
    blockFile.read(reinterpret_cast<char *>(&txHashBytes[0]), sizeof(transaction.version));
    
    blockFile.seekg(startPosInputs, ios_base::beg);
    blockFile.read(reinterpret_cast<char *>(&txHashBytes[0] + 4), txitxoLength);
    
    blockFile.seekg(endPosTx-4, ios_base::beg);
    blockFile.read(reinterpret_cast<char *>(&txHashBytes[0] + 4 + txitxoLength), sizeof(transaction.lockTime));
    transaction.txHash = VtcBlockIndexer::Utility::hashToReverseHex(VtcBlockIndexer::Utility::sha256(VtcBlockIndexer::Utility::sha256(txHashBytes)));

    if(segwit) {
        blockFile.seekg(startPosTx, ios_base::beg);
        uint64_t length = endPosTx-startPosTx;
        std::vector<unsigned char> transactionBytes(length);
        blockFile.read(reinterpret_cast<char *>(&transactionBytes[0]) , length);
        transaction.txWitHash = VtcBlockIndexer::Utility::hashToReverseHex(VtcBlockIndexer::Utility::sha256(VtcBlockIndexer::Utility::sha256(transactionBytes)));
    } else {
        transaction.txWitHash = string(transaction.txHash);
    }
    blockFile.seekg(endPosTx, ios_base::beg);

    return transaction;
}
  