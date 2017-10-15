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
        string start(prefix + "-txo-00000001");
        string limit(prefix + "-txo-99999999");
        
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

bool VtcBlockIndexer::BlockIndexer::indexBlock(Block block) {
    stringstream ss;
    ss << "block-" << setw(8) << setfill('0') << block.height;
    db->Put(leveldb::WriteOptions(), ss.str(), block.blockHash);

    // TODO: Verify block integrity
    for(VtcBlockIndexer::Transaction tx : block.transactions) {
        for(VtcBlockIndexer::TransactionOutput out : tx.outputs) {
            vector<string> addresses = scriptSolver->getAddressesFromScript(out.script);
            for(string address : addresses) {
                int nextIndex = getNextTxoIndex(address);
                stringstream txoKey;
                txoKey << address << "-txo-" << setw(8) << setfill('0') << nextIndex;
                stringstream txoValue;
                txoValue << tx.txHash << setw(8) << setfill('0') << out.index;
                db->Put(leveldb::WriteOptions(), txoKey.str(), txoValue.str());
            }
        }

        for(VtcBlockIndexer::TransactionInput txi : tx.inputs) {
            stringstream txSpentKey;
            txSpentKey << "txo-" << txi.txHash << "-" << setw(8) << setfill('0') << txi.txoIndex << "-spent";
            db->Put(leveldb::WriteOptions(), txSpentKey.str(), block.blockHash);
        }
    }



    return true;
}

