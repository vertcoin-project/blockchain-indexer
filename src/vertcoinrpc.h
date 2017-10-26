#ifndef VERTCOINRPC_INCLUDED_
#define VERTCOINRPC_INCLUDED_

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
    };
}

#endif