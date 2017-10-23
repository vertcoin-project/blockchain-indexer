#infdef VERTCOINRPC_INCLUDED_
#define VERTCOINRPC_INCLUDED_

#include <jsonrpccpp/client.h>

namespace VtcBlockIndexer {
    class VertcoinClient : public jsonrpc::Client {
        public:
            VertcoinClient(jsonrpc::IClientConnector &conn,
                           jsonrpc::clientVersion_t type 
                           = jsonrpc::JSONRPC_CLIENT_V1) : 
                           jsonrpc::Client(conn
                                           type) {}
                                     
            Json::Value getrawtransaction(const std::string& id, const bool hex) 
            throw (jsonrpc::JsonRpcException) {
                Json::Value p;
                p.append(id);
                p.append(hex);
                const Json::Value result = this->CallMethod("getrawtransaction", p);
                if(result.isObject()) {
                    return result;
                } else {
                    throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE,
                                                    result.toStyledString());
                }
            }
    };
}

#endif