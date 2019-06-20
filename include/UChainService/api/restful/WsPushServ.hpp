/*
* Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS).
* Copyright (C) 2013-2018 Swirly Cloud Limited.
*
* This program is free software; you can redistribute it and/or modify it under the terms of the
* GNU General Public License as published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
* even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program; if
* not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/
#ifndef UCD_WS_PUSH_SERV_HPP
#define UCD_WS_PUSH_SERV_HPP

#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <UChain/coin.hpp>
#include <UChainService/api/restful/MgServer.hpp>

namespace libbitcoin
{
namespace server
{
class server_node;
}
} // namespace libbitcoin

namespace mgbubble
{

class WsEvent : public std::enable_shared_from_this<WsEvent>
{
  public:
    explicit WsEvent(const std::function<void(uint64_t)> &&handler)
        : callback_(std::move(handler))
    {
    }

    WsEvent *hook()
    {
        self_ = this->shared_from_this();
        return this;
    }

    void unhook()
    {
        self_.reset();
    }

    virtual void operator()(uint64_t id)
    {
        callback_(id);
        self_.reset();
    }

  private:
    std::shared_ptr<WsEvent> self_;

    // called on mongoose thread
    std::function<void(uint64_t id)> callback_;
};

class WsPushServ : public MgServer
{
    typedef bc::chain::point::indexes index_list;
    typedef bc::message::block_msg::ptr_list block_list;
    typedef MgServer base;

  public:
    explicit WsPushServ(libbitcoin::server::server_node &node, const std::string &srv_addr)
        : node_(node), MgServer(srv_addr)
    {
    }

    ~WsPushServ() noexcept { stop(); };

    bool start() override;

    void spawn_to_mongoose(const std::function<void(uint64_t)> &&handler);

  protected:
    bool handle_blockchain_reorganization(
        const bc::code &ec, uint64_t fork_point,
        const block_list &new_blocks, const block_list &);
    bool handle_tx_pool(
        const bc::code &ec, const index_list &,
        bc::message::transaction_message::ptr tx);

    void notify_blocks(uint32_t fork_point, const block_list &blocks);
    void notify_block(uint32_t height, const bc::chain::block::ptr block);

    void notify_block_impl(uint32_t height, const bc::chain::block::ptr block);
    void notify_transaction(
        uint32_t height, const bc::hash_digest &block_hash,
        const bc::chain::transaction &tx);

  protected:
    void send_bad_response(
        struct mg_connection &nc, const char *message = nullptr,
        int code = 1000001, Json::Value data = Json::nullValue);
    void send_response(
        struct mg_connection &nc, const std::string &event,
        const std::string &channel, Json::Value data = Json::nullValue);

    void refresh_connections();

  protected:
    void run() override;

    void on_ws_handshake_done_handler(struct mg_connection &nc) override;
    void on_ws_frame_handler(struct mg_connection &nc, websocket_message &msg) override;
    void on_close_handler(struct mg_connection &nc) override;
    void on_broadcast(struct mg_connection &nc, const char *ev_data) override;
    void on_send_handler(struct mg_connection &nc, int bytes_transfered) override;
    void on_notify_handler(struct mg_connection &nc, struct mg_event &ev) override;

  private:
    typedef std::vector<std::string> string_vector;
    typedef std::map<std::weak_ptr<mg_connection>, string_vector,
                     std::owner_less<std::weak_ptr<mg_connection>>>
        connection_string_map;

    void do_notify(
        const std::vector<std::weak_ptr<mg_connection>> &notify_cons,
        Json::Value &value,
        std::shared_ptr<connection_string_map> topic_map = nullptr);

  private:
    libbitcoin::server::server_node &node_;
    std::unordered_map<void *, std::shared_ptr<mg_connection>> map_connections_;
    connection_string_map subscribers_;
    std::mutex subscribers_lock_;

    connection_string_map block_subscribers_;
    std::mutex block_subscribers_lock_;
};
} // namespace mgbubble

#endif