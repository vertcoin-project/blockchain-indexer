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

#include "vertcoinrpc.h"
#include <jsonrpccpp/client/connectors/httpclient.h>
#include <memory>
#include "blockreader.h"
#include "scriptsolver.h"
#include <unordered_map>
#ifndef MEMPOOLMONITOR_H_INCLUDED
#define MEMPOOLMONITOR_H_INCLUDED

using namespace std;

namespace VtcBlockIndexer {

/**
 * The BlockFileWatcher class provides methods to watch and scan a blocks directory
 * and process the blockfiles when changes occur.
 */

class MempoolMonitor {
public:
    /** Constructs a MempoolMonitor instance
     */
    MempoolMonitor();

    /** Starts watching the mempool for new transactions */
    void startWatcher();

    /** Notify a transaction has been indexed - remove it from the mempool */
    void transactionIndexed(string txid);

    /** Returns the spender txid if an outpoint is spent */
    string outpointSpend(string txid, uint32_t vout);

    /** Returns TXOs in the memorypool matching an address */
    vector<VtcBlockIndexer::TransactionOutput> getTxos(string address);

private:
    unique_ptr<VertcoinClient> vertcoind;
    unique_ptr<jsonrpc::HttpClient> httpClient;
    unordered_map<string, VtcBlockIndexer::Transaction> mempoolTransactions;
    unordered_map<string, vector<VtcBlockIndexer::TransactionOutput>> addressMempoolTransactions;
    unique_ptr<VtcBlockIndexer::BlockReader> blockReader;
    unique_ptr<VtcBlockIndexer::ScriptSolver> scriptSolver;
}; 

}

#endif // MEMPOOLMONITOR_H_INCLUDED