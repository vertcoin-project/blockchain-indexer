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
#include "blockchaintypes.h"
#include <iostream>
#include <sstream>
#include "leveldb/db.h"

using namespace std;

leveldb::DB* db;

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

bool VtcBlockIndexer::BlockIndexer::indexBlock(Block block) {
    stringstream ss;
    ss << "block-" << block.height;
    db->Put(leveldb::WriteOptions(), ss.str(), block.blockHash);
    return true;
}
  