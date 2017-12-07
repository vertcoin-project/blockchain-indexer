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
#include "httpserver.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <memory>
#include <cstdlib>
#include <restbed>
#include "json.hpp"
#include "utility.h"
using namespace std;
using namespace restbed;
using json = nlohmann::json;


VtcBlockIndexer::HttpServer::HttpServer(shared_ptr<leveldb::DB> db, shared_ptr<VtcBlockIndexer::MempoolMonitor> mempoolMonitor, string blocksDir) {
    this->db = db;
    this->blocksDir = blocksDir;
    this->mempoolMonitor = mempoolMonitor;
    blockReader.reset(new VtcBlockIndexer::BlockReader(blocksDir));
    scriptSolver = std::make_unique<VtcBlockIndexer::ScriptSolver>();
    httpClient.reset(new jsonrpc::HttpClient("http://middleware:middleware@" + std::string(std::getenv("COIND_HOST")) + ":8332"));
    vertcoind.reset(new VertcoinClient(*httpClient));
}



void VtcBlockIndexer::HttpServer::getTransaction(const shared_ptr<Session> session) {
    const auto request = session->get_request();
    
    cout << "Looking up txid " << request->get_path_parameter("id") << endl;
    
    try {
        const Json::Value tx = vertcoind->getrawtransaction(request->get_path_parameter("id"), true);
        
        stringstream body;
        body << tx.toStyledString();
        
        session->close(OK, body.str(), {{"Content-Type","application/json"},{"Content-Length",  std::to_string(body.str().size())}});
    } catch(const jsonrpc::JsonRpcException& e) {
        const std::string message(e.what());
        cout << "Not found " << message << endl;
        session->close(404, message, {{"Content-Type","application/json"},{"Content-Length",  std::to_string(message.size())}});
    }
}


void VtcBlockIndexer::HttpServer::getTransactionProof(const shared_ptr<Session> session) {
    const auto request = session->get_request();
    
    std::string blockHash;
    std::string txId = request->get_path_parameter("id","");
    leveldb::Status s = this->db->Get(leveldb::ReadOptions(), "tx-" + txId + "-block", &blockHash);
    if(!s.ok()) // no key found
    {
        const std::string message("TX not found");
        session->close(404, message, {{"Content-Length",  std::to_string(message.size())}});
        return;
    }

    std::string blockHeightString;
    s = this->db->Get(leveldb::ReadOptions(), "block-hash-" + blockHash, &blockHeightString);
    if(!s.ok()) // no key found
    {
        const std::string message("Block not found");
        session->close(404, message, {{"Content-Length",  std::to_string(message.size())}});
        return;
    }
    uint64_t blockHeight = stoll(blockHeightString);
    json j;
    j["txHash"] = txId;
    j["blockHash"] = blockHash;
    j["blockHeight"] = blockHeight;
    json chain = json::array();
    for(uint64_t i = blockHeight+1; --i > 0 && i > blockHeight-10;) {
        stringstream blockKey;
        blockKey << "block-filePosition-" << setw(8) << setfill('0') << i;
   
        std::string filePosition;
        s = this->db->Get(leveldb::ReadOptions(), blockKey.str(), &filePosition);
        if(!s.ok()) // no key found
        {
            const std::string message("Block not found");
            session->close(404, message, {{"Content-Length",  std::to_string(message.size())}});
            return;
        }
       
        Block block = this->blockReader->readBlock(filePosition.substr(0,12),stoll(filePosition.substr(12,12)),i,true);

        json jsonBlock;
        jsonBlock["blockHash"] = block.blockHash;
        jsonBlock["previousBlockHash"] = block.previousBlockHash;
        jsonBlock["merkleRoot"] = block.merkleRoot;
        jsonBlock["version"] = block.version;
        jsonBlock["time"] = block.time;
        jsonBlock["bits"] = block.bits;
        jsonBlock["nonce"] = block.nonce;
        jsonBlock["height"] = block.height;
        chain.push_back(jsonBlock);

    }
    j["chain"] = chain;
    string body = j.dump();
    
   session->close( OK, body, { { "Content-Type",  "application/json" }, { "Content-Length",  std::to_string(body.size()) } } );
}

void VtcBlockIndexer::HttpServer::sync(const shared_ptr<Session> session) {
    json j;

    const auto request = session->get_request( );

    string highestBlockString;
    this->db->Get(leveldb::ReadOptions(),"highestblock",&highestBlockString);

    j["error"] = nullptr;
    j["height"] = stoll(highestBlockString);
    try {
        const Json::Value blockCount = vertcoind->getblockcount();
        
        j["blockChainHeight"] = blockCount.asInt();
    } catch(const jsonrpc::JsonRpcException& e) {
        const std::string message(e.what());
        j["error"] = message;
    }

    float progress = (float)j["height"].get<int>() / (float)j["blockChainHeight"].get<int>();
    progress *= 100;
    j["syncPercentage"] = progress;
    if(progress >= 100) {
        j["status"] = "finished";
    } else {
        j["status"] = "indexing";
    }

    string body = j.dump();
    session->close( OK, body, { { "Content-Type",  "application/json" }, { "Content-Length",  std::to_string(body.size()) } } );
}

void VtcBlockIndexer::HttpServer::getBlocks(const shared_ptr<Session> session) {
    json j = json::array();

    const auto request = session->get_request( );

    string highestBlockString;
    this->db->Get(leveldb::ReadOptions(),"highestblock",&highestBlockString);
    
   
    long long limitParam = stoi(request->get_query_parameter("limit","0"));
    if(limitParam == 0 || limitParam > 100)
        limitParam = 100;

    long long lowestBlock = stoll(highestBlockString)-limitParam;
    stringstream lowestBlockString;
    lowestBlockString << setw(8) << setfill('0') << lowestBlock;

    string start("block-" + highestBlockString);
    string limit("block-" + lowestBlockString.str());
    
    leveldb::Iterator* it = this->db->NewIterator(leveldb::ReadOptions());
    for (it->Seek(start);
            it->Valid() && it->key().ToString() > limit;
            it->Prev()) {
        json blockObj;
        string blockHeightString = it->key().ToString().substr(6);
        blockObj["hash"] = it->value().ToString();
        string blockSizeString;
        string blockTxesString;
        string blockTimeString;
        this->db->Get(leveldb::ReadOptions(),"block-size-" + blockHeightString,&blockSizeString);
        this->db->Get(leveldb::ReadOptions(),"block-txcount-" + blockHeightString,&blockTxesString);
        this->db->Get(leveldb::ReadOptions(),"block-time-" + blockHeightString,&blockTimeString);
        blockObj["height"] = stoll(blockHeightString);
        blockObj["size"] = stoll(blockSizeString);
        blockObj["time"] = stoll(blockTimeString);
        blockObj["txlength"] = stoll(blockTxesString);
        blockObj["poolInfo"] = nullptr;
        j.push_back(blockObj);
    }

    string body = j.dump();
    
   session->close( OK, body, { { "Content-Type",  "application/json" }, { "Content-Length",  std::to_string(body.size()) } } );

}

void VtcBlockIndexer::HttpServer::addressBalance( const shared_ptr< Session > session )
{
    long long balance = 0;
    int txoCount = 0;
    const auto request = session->get_request( );
    
    cout << "Checking balance for address " << request->get_path_parameter( "address" ) << endl;

    string start(request->get_path_parameter( "address" ) + "-txo-00000001");
    string limit(request->get_path_parameter( "address" ) + "-txo-99999999");
    
    leveldb::Iterator* it = this->db->NewIterator(leveldb::ReadOptions());
    
    for (it->Seek(start);
            it->Valid() && it->key().ToString() < limit;
            it->Next()) {

        string spentTx;
        txoCount++;
        string txo = it->value().ToString();

        leveldb::Status s = this->db->Get(leveldb::ReadOptions(), "txo-" + txo.substr(0,64) + "-" + txo.substr(64,8) + "-spent", &spentTx);
        if(!s.ok()) // no key found, not spent. Add balance.
        {
            // check mempool for spenders
            string spender = mempoolMonitor->outpointSpend(txo.substr(0,64), stol(txo.substr(64,8)));
            if(spender.compare("") == 0) {
                balance += stoll(txo.substr(80));
            }
        }
    }
    assert(it->status().ok());  // Check for any errors found during the scan
    delete it;

    cout << "Analyzed " << txoCount << " TXOs - Balance is " << balance << endl;
 
    // Add mempool transactions
    vector<VtcBlockIndexer::TransactionOutput> mempoolOutputs = mempoolMonitor->getTxos(request->get_path_parameter( "address" ));
    for (VtcBlockIndexer::TransactionOutput txo : mempoolOutputs) {
        txoCount++;
        string spender = mempoolMonitor->outpointSpend(txo.txHash, txo.index);
        cout << "Spender for " << txo.txHash << "/" << txo.index << " = " << spender;
        if(spender.compare("") == 0) {
            balance += txo.value;
        }
    }

    cout << "Including mempool: Analyzed " << txoCount << " TXOs - Balance is " << balance << endl;
    

    stringstream body;
    body << balance;
    
    session->close( OK, body.str(), { {"Content-Type","text/plain"}, { "Content-Length",  std::to_string(body.str().size()) } } );
}

void VtcBlockIndexer::HttpServer::addressTxos( const shared_ptr< Session > session )
{
    json j = json::array();

    const auto request = session->get_request( );

    long long sinceBlock = stoll(request->get_path_parameter( "sinceBlock", "0" ));
    
    int txHashOnly = stoi(request->get_query_parameter("txHashOnly","0"));
    int raw = stoi(request->get_query_parameter("raw","0"));
    int unspent = stoi(request->get_query_parameter("unspent","0"));
    int unconfirmed = stoi(request->get_query_parameter("unconfirmed","0"));
    
    cout << "Fetching address txos for address " << request->get_path_parameter( "address" ) << endl;
   
    string start(request->get_path_parameter( "address" ) + "-txo-00000001");
    string limit(request->get_path_parameter( "address" ) + "-txo-99999999");
    
    leveldb::Iterator* it = this->db->NewIterator(leveldb::ReadOptions());
    
    for (it->Seek(start);
            it->Valid() && it->key().ToString() < limit;
            it->Next()) {

        string spentTx;
        string txo = it->value().ToString();

        leveldb::Status s = this->db->Get(leveldb::ReadOptions(), "txo-" + txo.substr(0,64) + "-" + txo.substr(64,8) + "-spent", &spentTx);
        long long block = stoll(txo.substr(72,8));
        if(block >= sinceBlock) {
            json txoObj;
            txoObj["height"] = block;

            if(!s.ok()) {
                if(unconfirmed) {
                    string spender = mempoolMonitor->outpointSpend(txo.substr(0,64), stol(txo.substr(64,8)));
                    if(spender.compare("") == 0) {
                        txoObj["spender"] = nullptr;
                    } else {
                        if(unspent == 1) continue;
                        txoObj["spender"] = spender;
                    }
                } else { 
                    txoObj["spender"] = nullptr;
                }
            } else {
                if(unspent == 1) continue;
                txoObj["spender"] = spentTx.substr(65, 64);

            }

            if(raw != 0) {
                try {
                    const Json::Value tx = vertcoind->getrawtransaction(txo.substr(0,64), false);
                    txoObj["tx"] = tx.asString();
                } catch(const jsonrpc::JsonRpcException& e) {
                    const std::string message(e.what());
                    cout << "Not found " << message << endl;
                }
            }

           

            if(raw != 0 && txoObj["spender"].is_string()) {
                try {
                    const Json::Value tx = vertcoind->getrawtransaction(txoObj["spender"].get<string>(), false);
                    txoObj["spender"] = tx.asString();
                } catch(const jsonrpc::JsonRpcException& e) {
                    const std::string message(e.what());
                    cout << "Not found " << message << endl;
                }
            }

            if(raw == 0) {
                txoObj["txhash"] = txo.substr(0,64);
            }
            if(txHashOnly == 0 && raw == 0) {
                txoObj["vout"] = stoll(txo.substr(64,8));
                txoObj["value"] = stoll(txo.substr(80));
            }
            
            j.push_back(txoObj);
        }
    }
    assert(it->status().ok());  // Check for any errors found during the scan
    delete it;

    if(unconfirmed == 1) {
        // Add mempool transactions
        vector<VtcBlockIndexer::TransactionOutput> mempoolOutputs = mempoolMonitor->getTxos(request->get_path_parameter( "address" ));
        for (VtcBlockIndexer::TransactionOutput txo : mempoolOutputs) {
            json txoObj;
            txoObj["txhash"] = txo.txHash;
            txoObj["vout"] = txo.index;
            txoObj["value"] = txo.value;
            txoObj["block"] = 0;
            string spender = mempoolMonitor->outpointSpend(txo.txHash, txo.index);
            if(spender.compare("") != 0) {
                txoObj["spender"] = spender;
            } else {
                txoObj["spender"] = nullptr;
            }
            j.push_back(txoObj);
        }
    }

    string body = j.dump();
     
    session->close( OK, body, { { "Content-Type",  "application/json" }, { "Content-Length",  std::to_string(body.size()) } } );
}

void VtcBlockIndexer::HttpServer::outpointSpend( const shared_ptr< Session > session )
{
    json j;
    j["error"] = false;
    const auto request = session->get_request( );
    int raw = stoi(request->get_query_parameter("raw","0"));
    int unconfirmed = stoi(request->get_query_parameter("unconfirmed","0"));
    
    long long vout = stoll(request->get_path_parameter( "vout", "0" ));
    string txid = request->get_path_parameter("txid", "");
    stringstream txBlockKey;
    string txBlock;
    txBlockKey << "tx-" << txid << "-block";
    leveldb::Status s = this->db->Get(leveldb::ReadOptions(), txBlockKey.str(), &txBlock);
    if(!s.ok()) {
        j["error"] = true;
        j["errorDescription"] = "Transaction ID not found";
    }
    else 
    {
        stringstream txoId;
        txoId << "txo-" << txid << "-" << setw(8) << setfill('0') << vout << "-spent";
        cout << "Checking outpoint spent " << txoId.str() << endl;
        string spentTx;

        s = this->db->Get(leveldb::ReadOptions(), txoId.str(), &spentTx);
        j["spent"] = s.ok();
        if(s.ok()) {
            j["spender"] = spentTx.substr(65, 64);
        } else if(unconfirmed != 0) {
            string mempoolSpend = mempoolMonitor->outpointSpend(txid, vout);
            if(mempoolSpend.compare("") != 0) {
                j["spent"] = true;
                j["spender"] = mempoolSpend;
            }
        }

        if(raw != 0 && j["spender"].is_string()) {
            try {
                const Json::Value tx = vertcoind->getrawtransaction(j["spender"].get<string>(), false);
                j["spenderRaw"] = tx.asString();
                j["spender"] = nullptr;
            } catch(const jsonrpc::JsonRpcException& e) {
                const std::string message(e.what());
                cout << "Not found " << message << endl;
            }
        }




    }
   
    string body = j.dump();
     
    session->close( OK, body, { { "Content-Type",  "application/json" }, { "Content-Length",  std::to_string(body.size()) } } );
} 


void VtcBlockIndexer::HttpServer::outpointSpends( const shared_ptr< Session > session )
{
    const auto request = session->get_request( );
    size_t content_length = 0;
    content_length = request->get_header( "Content-Length", 0);
    
    
    
    session->fetch( content_length, [ request, this ]( const shared_ptr< Session > session, const Bytes & body )
    {
        const auto request = session->get_request( );
        int raw = stoi(request->get_query_parameter("raw","0"));
        int unconfirmed = stoi(request->get_query_parameter("unconfirmed","0"));
        

        string content =string(body.begin(), body.end());
        json output = json::array();
        json input = json::parse(content);
        if(!input.is_null()) {
            for (auto& txo : input) {
                if(txo.is_object() && txo["txid"].is_string() && txo["vout"].is_number()) {
                    stringstream txoId;
                    txoId << "txo-" << txo["txid"].get<string>() << "-" << setw(8) << setfill('0') << txo["vout"].get<int>() << "-spent";
                    cout << "Checking outpoint spent " << txoId.str() << endl;
            
                    json j;
                    j["txid"] = txo["txid"];
                    j["vout"] = txo["vout"];
                    j["error"] = false;
                    stringstream txBlockKey;
                    string txBlock;
                    txBlockKey << "tx-" << txo["txid"].get<string>() << "-block";
                    leveldb::Status s = this->db->Get(leveldb::ReadOptions(), txBlockKey.str(), &txBlock);
                    if(!s.ok()) {
                        j["error"] = true;
                        j["errorDescription"] = "Transaction ID not found";
                    }
                    else 
                    {
                        string spentTx;
                        s = this->db->Get(leveldb::ReadOptions(), txoId.str(), &spentTx);
                        if(s.ok()) {
                            j["spender"] = spentTx.substr(65, 64);
                            j["spent"] = true;
                        } else if(unconfirmed != 0) {
                            string mempoolSpend = mempoolMonitor->outpointSpend( txo["txid"].get<string>(), txo["vout"].get<int>());
                            if(mempoolSpend.compare("") != 0) {
                                json j;
                                j["spender"] = mempoolSpend;
                                j["spent"] = true;
                            } else {
                                j["spent"] = false;
                            }
                        }
                    }

                    if(raw != 0 && j["spender"].is_string()) {
                        try {
                            const Json::Value tx = vertcoind->getrawtransaction(j["spender"].get<string>(), false);
                            j["spenderRaw"] = tx.asString();
                            j["spender"] = nullptr;
                        } catch(const jsonrpc::JsonRpcException& e) {
                            const std::string message(e.what());
                            cout << "Not found " << message << endl;
                        }
                    }

                    output.push_back(j);
                    
                }
            }
        }
    
        string resultBody = output.dump();
        session->close( OK, resultBody, { { "Content-Type",  "application/json" }, { "Content-Length",  std::to_string(resultBody.size()) } } );
    } );
} 

void VtcBlockIndexer::HttpServer::sendRawTransaction( const shared_ptr< Session > session )
{
    const auto request = session->get_request( );
    const size_t content_length = request->get_header( "Content-Length", 0);
    session->fetch( content_length, [ request, this ]( const shared_ptr< Session > session, const Bytes & body )
    {
        const string rawtx = string(body.begin(), body.end());
        
        try {
            const auto txid = vertcoind->sendrawtransaction(rawtx);
            
            session->close(OK, txid, {{"Content-Type","text/plain"}, {"Content-Length",  std::to_string(txid.size())}});
        } catch(const jsonrpc::JsonRpcException& e) {
            const std::string message(e.what());
            session->close(400, message, {{"Content-Type","text/plain"},{"Content-Length",  std::to_string(message.size())}});
        }
    });
} 

void VtcBlockIndexer::HttpServer::run()
{
    auto addressBalanceResource = make_shared< Resource >( );
    addressBalanceResource->set_path( "/addressBalance/{address: .*}" );
    addressBalanceResource->set_method_handler( "GET", bind( &VtcBlockIndexer::HttpServer::addressBalance, this, std::placeholders::_1) );

    auto addressTxosResource = make_shared< Resource >( );
    addressTxosResource->set_path( "/addressTxos/{address: .*}" );
    addressTxosResource->set_method_handler( "GET", bind( &VtcBlockIndexer::HttpServer::addressTxos, this, std::placeholders::_1) );

    auto addressTxosSinceBlockResource = make_shared< Resource >( );
    addressTxosSinceBlockResource->set_path( "/addressTxosSince/{sinceBlock: ^[0-9]*$}/{address: .*}" );
    addressTxosSinceBlockResource->set_method_handler( "GET", bind( &VtcBlockIndexer::HttpServer::addressTxos, this, std::placeholders::_1) );
    
    auto getTransactionResource = make_shared<Resource>();
    getTransactionResource->set_path( "/getTransaction/{id: [0-9a-f]*}" );
    getTransactionResource->set_method_handler("GET", bind(&VtcBlockIndexer::HttpServer::getTransaction, this, std::placeholders::_1) );

    auto getTransactionProofResource = make_shared<Resource>();
    getTransactionProofResource->set_path( "/getTransactionProof/{id: [0-9a-f]*}" );
    getTransactionProofResource->set_method_handler("GET", bind(&VtcBlockIndexer::HttpServer::getTransactionProof, this, std::placeholders::_1) );

    auto outpointSpendResource = make_shared<Resource>();
    outpointSpendResource->set_path( "/outpointSpend/{txid: .*}/{vout: .*}" );
    outpointSpendResource->set_method_handler("GET", bind(&VtcBlockIndexer::HttpServer::outpointSpend, this, std::placeholders::_1) );

    auto outpointSpendsResource = make_shared<Resource>();
    outpointSpendsResource->set_path( "/outpointSpends" );
    outpointSpendsResource->set_method_handler("POST", bind(&VtcBlockIndexer::HttpServer::outpointSpends, this, std::placeholders::_1) );

    auto sendRawTransactionResource = make_shared<Resource>();
    sendRawTransactionResource->set_path( "/sendRawTransaction" );
    sendRawTransactionResource->set_method_handler("POST", bind(&VtcBlockIndexer::HttpServer::sendRawTransaction, this, std::placeholders::_1) );

    auto blocksResource = make_shared<Resource>();
    blocksResource->set_path( "/blocks" );
    blocksResource->set_method_handler("GET", bind(&VtcBlockIndexer::HttpServer::getBlocks, this, std::placeholders::_1) );

    auto syncResource = make_shared<Resource>();
    syncResource->set_path( "/sync" );
    syncResource->set_method_handler("GET", bind(&VtcBlockIndexer::HttpServer::sync, this, std::placeholders::_1) );


    auto settings = make_shared< Settings >( );
    settings->set_port( 8888 );
    settings->set_default_header( "Connection", "close" );

    Service service;
    service.publish( addressBalanceResource );
    service.publish( addressTxosResource );
    service.publish( addressTxosSinceBlockResource );
    service.publish( getTransactionResource );
    service.publish( getTransactionProofResource );
    service.publish( outpointSpendResource );
    service.publish( outpointSpendsResource );
    service.publish( sendRawTransactionResource );
    service.publish( blocksResource );
    service.publish( syncResource );
    service.start( settings );
}
