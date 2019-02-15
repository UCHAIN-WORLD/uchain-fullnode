/**
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS) 
 *
 * This file is part of uc-node.
 *
 * UChain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <UChainService/api/restful/Mongoose.hpp>
#include <UChainService/api/restful/MgServer.hpp>
#include <UChainService/api/restful/utility/Stream_buf.hpp>
#include <UChainService/api/restful/utility/Tokeniser.hpp>
#include <UChainService/api/restful/exception/Instances.hpp>

#include <UChain/client.hpp>
#include <UChain/blockchain.hpp>
#include <UChainApp/ucd/services/query_service.hpp> //public_query

namespace libbitcoin
{
namespace server
{
class server_node;
}
} // namespace libbitcoin

namespace mgbubble
{

using namespace bc;

class RestServ : public MgServer
{
    typedef MgServer base;

  public:
    explicit RestServ(const char *webroot, libbitcoin::server::server_node &node, const std::string &srv_addr)
        : node_(node), MgServer(srv_addr)
    {
        document_root_ = webroot;
        set_document_root(document_root_.c_str());
    }
    ~RestServ() noexcept { stop(); };

    // Copy.
    RestServ(const RestServ &rhs) = delete;
    RestServ &operator=(const RestServ &rhs) = delete;

    // Move.
    RestServ(RestServ &&) = delete;
    RestServ &operator=(RestServ &&) = delete;

    void rpc_request(mg_connection &nc, HttpMessage data, uint8_t rpc_version = 1);
    void ws_request(mg_connection &nc, WebsocketMessage ws);

  public:
    void reset(HttpMessage &data) noexcept;

    bool start() override;

    void spawn_to_mongoose(const std::function<void(uint64_t)> &&handler);

  protected:
    void run() override;

    void on_http_req_handler(struct mg_connection &nc, struct http_message &msg) override;
    void on_notify_handler(struct mg_connection &nc, struct mg_event &ev) override;
    void on_ws_handshake_done_handler(struct mg_connection &nc) override;
    void on_ws_frame_handler(struct mg_connection &nc, struct websocket_message &msg) override;

  private:
    enum : int
    {
        // Method values are represented as powers of two for simplicity.
        MethodGet = 1 << 0,
        MethodPost = 1 << 1,
        MethodPut = 1 << 2,
        MethodDelete = 1 << 3,
        // Method value mask.
        MethodMask = MethodGet | MethodPost | MethodPut | MethodDelete,

        // Subsequent bits represent matching components.
        MatchMethod = 1 << 4,
        MatchUri = 1 << 5,
        // Match result mask.
        MatchMask = MatchMethod | MatchUri
    };

    bool isSet(int bs) const noexcept { return (state_ & bs) == bs; }

    // config
    static thread_local OStream out_;
    static thread_local Tokeniser<'/'> uri_;
    static thread_local int state_;
    const char *const servername_{"UChain " UC_VERSION};
    libbitcoin::server::server_node &node_;
    string document_root_;
};

} // namespace mgbubble
