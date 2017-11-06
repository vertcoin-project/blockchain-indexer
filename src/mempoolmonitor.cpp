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
#include "mempoolmonitor.h"
#include "scriptsolver.h"
#include "blockchaintypes.h"
#include <unordered_map>
#include <chrono>
#include <thread>
#include <time.h>
using namespace std;

// This map keeps the memorypool transactions deserialized in memory.
unordered_map<string, VtcBlockIndexer::Transaction> mempoolTransactions;

VtcBlockIndexer::MempoolMonitor::MempoolMonitor() {
    httpClient.reset(new jsonrpc::HttpClient("http://middleware:middleware@vertcoind:8332"));
    vertcoind.reset(new VertcoinClient(*httpClient));
}

void VtcBlockIndexer::MempoolMonitor::startWatcher() {
    while(true) {
        try {
            const Json::Value mempool = vertcoind->getrawmempool();
            for ( uint index = 0; index < mempool.size(); ++index )
            {
                cout << "Found mempool tx: " << mempool[index].asString() << endl;
            }
        } catch(const jsonrpc::JsonRpcException& e) {
            const std::string message(e.what());
            cout << "Error reading mempool " << message << endl;
        }
        
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}