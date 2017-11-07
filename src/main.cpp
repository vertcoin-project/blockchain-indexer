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

#include <unordered_map>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <string.h>
#include <memory>
#include <vector>
#include <ctime>
#include "leveldb/db.h"
#include "leveldb/write_batch.h"
#include "leveldb/filter_policy.h"
#include "leveldb/cache.h"
#include "utility.h"
#include "blockchaintypes.h"
#include "httpserver.h"
#include "mempoolmonitor.h"
#include "blockfilewatcher.h"
#include <thread>

using namespace std;


leveldb::DB *db;
VtcBlockIndexer::HttpServer httpServer(nullptr,nullptr,"");
VtcBlockIndexer::BlockFileWatcher blockFileWatcher("",nullptr, nullptr);
VtcBlockIndexer::MempoolMonitor mempoolMonitor;

void runBlockfileWatcher(string blocksDir) {
    cout << "Starting blockfile watcher..." << endl;
    blockFileWatcher = VtcBlockIndexer::BlockFileWatcher(blocksDir, db, &mempoolMonitor);
    blockFileWatcher.startWatcher();
}

void runMempoolMonitor() {
    cout << "Starting mempool monitor..." << endl;
    mempoolMonitor.startWatcher();
}


int main(int argc, char* argv[]) {
     // If user did not supply the command line parameter, show the usage and exit.
     if(argc < 2) {
        cerr << "Usage: vtc_indexer [blocks_dir]" << endl;
        exit(0);
    }

    leveldb::Options options;
    options.create_if_missing = true;
    options.block_cache = leveldb::NewLRUCache(300 * 1024 * 1024);
    options.filter_policy = leveldb::NewBloomFilterPolicy(10);
    leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
    assert(status.ok());

    // Start blockfile watcher on separate thread
    std::thread watcherThread(runBlockfileWatcher, string(argv[1]));   
    
    // Start blockfile watcher on separate thread
    std::thread mempoolThread(runMempoolMonitor);   
    
    // Start webserver on main thread.
    httpServer = VtcBlockIndexer::HttpServer(db, &mempoolMonitor, string(argv[1]));
    httpServer.run(); 
}
