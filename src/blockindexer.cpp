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
#include "blockindexer.h"
#include "scriptsolver.h"
#include "blockchaintypes.h"
#include <iostream>
#include <sstream>
#include "leveldb/db.h"
//#include "hashing.h"
#include <memory>
#include <iomanip>
#include <unordered_map>
#include "leveldb/write_batch.h"

using namespace std;

unordered_map<string, int> nextTxoIndex;
leveldb::DB* db;
VtcBlockIndexer::ScriptSolver* scriptSolver;

VtcBlockIndexer::BlockIndexer::BlockIndexer() {
    
}

bool VtcBlockIndexer::BlockIndexer::open() {
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
    if(!status.ok()) {
        cerr << status.ToString() << endl;
    }
    return status.ok();
}

bool VtcBlockIndexer::BlockIndexer::close() {
    delete db;
    return true;
}

int VtcBlockIndexer::BlockIndexer::getNextTxoIndex(string prefix) {
    if(nextTxoIndex.find(prefix) == nextTxoIndex.end()) {
        leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
        nextTxoIndex[prefix] = 1;
        string start(prefix + "-00000001");
        string limit(prefix + "-99999999");
        
        for (it->Seek(start);
                it->Valid() && it->key().ToString() < limit;
                it->Next()) {
                    nextTxoIndex[prefix]++;
        }
        assert(it->status().ok());  // Check for any errors found during the scan
        delete it;
    } else {
        nextTxoIndex[prefix]++;    
    }
    
    return nextTxoIndex[prefix];
}

bool VtcBlockIndexer::BlockIndexer::clearBlockTxos(string blockHash) {
    leveldb::WriteBatch batch;
    
    string start(blockHash + "-txo-00000001");
    string limit(blockHash + "-txo-99999999");
    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    for (it->Seek(start);
            it->Valid() && it->key().ToString() < limit;
            it->Next()) {
        cout << "Reorg. Deleting key [" << it->value().ToString() << "]";
        batch.Delete(it->value().ToString());           
    }
    assert(it->status().ok());  // Check for any errors found during the scan
    delete it;

    string spentStart(blockHash + "-txospent-00000001");
    string spentLimit(blockHash + "-txospent-99999999");
    it = db->NewIterator(leveldb::ReadOptions());
    for (it->Seek(spentStart);
            it->Valid() && it->key().ToString() < spentLimit;
            it->Next()) {
        cout << "Reorg. Deleting key [" << it->value().ToString() << "]";
        batch.Delete(it->value().ToString());           
    }
    assert(it->status().ok());  // Check for any errors found during the scan
    delete it;

    leveldb::Status s = db->Write(leveldb::WriteOptions(), &batch);
    return s.ok();
}

bool VtcBlockIndexer::BlockIndexer::hasIndexedBlock(string blockHash, int blockHeight)
{
    stringstream ss;
    ss << "block-" << setw(8) << setfill('0') << blockHeight;

    string existingBlockHash;
    leveldb::Status s = db->Get(leveldb::ReadOptions(), ss.str(), &existingBlockHash);
    if(s.ok() && existingBlockHash == blockHash) {
        return true;
    }
    
    return false;
}

bool VtcBlockIndexer::BlockIndexer::indexBlock(Block block) {
    stringstream ss;
    ss << "block-" << setw(8) << setfill('0') << block.height;

    string existingBlockHash;
    leveldb::Status s = db->Get(leveldb::ReadOptions(), ss.str(), &existingBlockHash);
    if(s.ok() && existingBlockHash == block.blockHash) {
        // Block found in database and matches. This block is indexed already, so skip.
        return true;
    } else if (s.ok()) {
        // There was a different block at this height. Ditch the TXOs from the old block.
        clearBlockTxos(existingBlockHash);
    }
    

    db->Put(leveldb::WriteOptions(), ss.str(), block.blockHash);

    // TODO: Verify block integrity
    for(VtcBlockIndexer::Transaction tx : block.transactions) {
        for(VtcBlockIndexer::TransactionOutput out : tx.outputs) {
            vector<string> addresses = scriptSolver->getAddressesFromScript(out.script);
            for(string address : addresses) {
                int nextIndex = getNextTxoIndex(address + "-txo");
                stringstream txoKey;
                txoKey << address << "-txo-" << setw(8) << setfill('0') << nextIndex;
                stringstream txoValue;
                txoValue << tx.txHash << setw(8) << setfill('0') << out.index << setw(8) << setfill('0') << block.height;
                db->Put(leveldb::WriteOptions(), txoKey.str(), txoValue.str());

                nextIndex = getNextTxoIndex(block.blockHash + "-txo");
                stringstream blockTxoKey;
                blockTxoKey << block.blockHash << "-txo-" << setw(8) << setfill('0') << nextIndex;
                db->Put(leveldb::WriteOptions(), blockTxoKey.str(), txoKey.str());
            }
        }

        for(VtcBlockIndexer::TransactionInput txi : tx.inputs) {
            if(!txi.coinbase)
            {
                stringstream txSpentKey;
                txSpentKey << "txo-" << txi.txHash << "-" << setw(8) << setfill('0') << txi.txoIndex << "-spent";
                db->Put(leveldb::WriteOptions(), txSpentKey.str(), block.blockHash);

                int nextIndex = getNextTxoIndex(block.blockHash + "-txospent");
                stringstream blockTxoSpentKey;
                blockTxoSpentKey << block.blockHash << "-txospent-" << setw(8) << setfill('0') << nextIndex;
                db->Put(leveldb::WriteOptions(), blockTxoSpentKey.str(), txSpentKey.str());
            }
        }
    }



    return true;
}

