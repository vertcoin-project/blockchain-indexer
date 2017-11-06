#ifndef VERTCOINRPC_INCLUDED_
#define VERTCOINRPC_INCLUDED_

#include <iostream>

#include <jsonrpccpp/client.h>

namespace VtcBlockIndexer {
    class VertcoinClient : public jsonrpc::Client {
        public:
            VertcoinClient(jsonrpc::IClientConnector &conn,
                           jsonrpc::clientVersion_t type 
                           = jsonrpc::JSONRPC_CLIENT_V1) : 
                           jsonrpc::Client(conn, type) {}
                                     
            Json::Value getrawtransaction(const std::string& id, const bool verbose) 
            throw (jsonrpc::JsonRpcException) {
                Json::Value p;
                p.append(id);
                p.append(verbose);
                const Json::Value result = this->CallMethod("getrawtransaction", p);
                if((result.isObject() && verbose) || (result.isString() && !verbose)) {
                    return result;
                } else {
                    throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE,
                                                    result.toStyledString());
                }
            }

            Json::Value getrawmempool() 
            throw (jsonrpc::JsonRpcException) {
                Json::Value p;
                const Json::Value result = this->CallMethod("getrawmempool", p);
                if(result.isArray()) {
                    return result;
                } else {
                    throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE,
                                                    result.toStyledString());
                }
            }
            
            std::string sendrawtransaction(const std::string& rawTx) 
            throw (jsonrpc::JsonRpcException) {
                Json::Value p;
                p.append(rawTx);
                const Json::Value result = this->CallMethod("sendrawtransaction", p);
                if(result.isString()) {
                    return result.asString();
                } else {
                    throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE,
                                                    result.toStyledString());
                }
            }
            
            Json::Value getblock(const std::string& id, const bool verbose) 
            throw (jsonrpc::JsonRpcException) {
                Json::Value p;
                p.append(id);
                p.append(verbose);
                const Json::Value result = this->CallMethod("getblock", p);
                if((result.isObject() && verbose) || (result.isString() && !verbose)) {
                    return result;
                } else {
                    throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE,
                                                    result.toStyledString());
                }
            }
            
            std::string getblockhash(const unsigned int height) 
            throw (jsonrpc::JsonRpcException) {
                Json::Value p;
                p.append(height);
                const Json::Value result = this->CallMethod("getblockhash", p);
                if(result.isString()) {
                    return result.asString();
                } else {
                    throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE,
                                                    result.toStyledString());
                }
            }
    };
}

#endif