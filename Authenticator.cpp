#include "Authenticator.h"
  
  Authenticator::Authenticator() {
        auth_message = R"({
            "jsonrpc": "2.0",
            "id": 9929,
            "method": "public/auth",
            "params": {
                "grant_type": "client_credentials",
                "client_id": ")" + client_id + R"(",
                "client_secret": ")" + client_secret + R"("
            }
        })";
    }

    void Authenticator::send_authcall(client* c, websocketpp::connection_hdl hdl) {
        websocketpp::lib::error_code ec;
        c->send(hdl, auth_message, websocketpp::frame::opcode::text, ec);
        if (ec) {
            std::cout << "Error sending authentication message: " << ec.message() << std::endl;
        }
    }