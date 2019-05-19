/**
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-api.
 *
 * UChain-explorer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <UChain/coin.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/command.hpp>
#include <UChainService/txs/token/token_detail.hpp> // used for createtoken
#include <UChainApp/ucd/server_node.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{

struct prikey_amount
{
    std::string first;
    uint64_t second;
};

struct prikey_ucn_amount
{
    std::string key;
    uint64_t value;
    uint64_t token_amount;
    output_point output;
};

struct utxo_attach_info
{
    //target:1:token-transfer:symbol:amount
    std::string target;
    uint32_t version;
    std::string type;
    std::string symbol;
    uint64_t amount;
    uint64_t value;            // ucn
    std::string output_option; // used by get_tx_encode
};

template <class T1, class T2>
class BCX_API colon_delimited2_item
{
  public:
    /**
     * Default constructor.
     */
    colon_delimited2_item(){};

    colon_delimited2_item(T1 first, T2 second)
        : first_(first), second_(second){};

    /**
     * Initialization constructor.
     * @param[in]  tuple  The value to initialize with.
     */
    colon_delimited2_item(const std::string &tuple)
    {
        std::stringstream(tuple) >> *this;
    };

    static bool decode_colon_delimited(colon_delimited2_item<T1, T2> &height, const std::string &tuple)
    {
        const auto tokens = split(tuple, BX_TX_POINT_DELIMITER);
        if (tokens.size() != 2)
            return false;

        deserialize(height.first_, tokens[0], true);
        deserialize(height.second_, tokens[1], true);

        return true;
    };

    // colon_delimited2_item is currently a private encoding in bx.
    static std::string encode_colon_delimited(const colon_delimited2_item<T1, T2> &height)
    {
        std::stringstream result;
        result << height.first_ << BX_TX_POINT_DELIMITER << height.second_;
        return result.str();
    };

    /**
     * Overload stream in. Throws if colon_delimited2_item is invalid.
     * @param[in]   colon_delimited2_item     The colon_delimited2_item stream to read the value from.
     * @param[out]  argument  The object to receive the read value.
     * @return                The colon_delimited2_item stream reference.
     */
    friend std::istream &operator>>(std::istream &stream, colon_delimited2_item &argument)
    {
        std::string tuple;
        stream >> tuple;

        if (!decode_colon_delimited(argument, tuple))
        {
            throw std::logic_error{"invalid option " + tuple};
        }

        return stream;
    };

    /**
     * Overload stream out.
     * @param[in]   output    The output stream to write the value to.
     * @param[out]  argument  The object from which to obtain the value.
     * @return                The output stream reference.
     */
    friend std::ostream &operator<<(std::ostream &output, const colon_delimited2_item &argument)
    {
        output << encode_colon_delimited(argument);
        return output;
    };

    // get method
    T1 first()
    {
        return first_;
    };

    void set_first(const T1 &first)
    {
        first_ = first;
    };

    T2 second()
    {
        return second_;
    };

    void set_second(const T2 &second)
    {
        second_ = second;
    };

  private:
    /**
     * The state of this object. only for uint64_t
     */
    T1 first_;
    T2 second_;
};

class command_extension : public command
{
  public:
    virtual console_result invoke(Json::Value &jv_output,
                                  libbitcoin::server::server_node &node)
    {
        return console_result::failure;
    }

  protected:
    struct argument_base
    {
        std::string name;
        std::string auth;
    } auth_;
};

class send_command : public command_extension
{
  public:
    virtual bool is_block_height_fullfilled(uint64_t height) override
    {
        if (height >= minimum_block_height())
        {
            return true;
        }
        return false;
    }

    virtual uint64_t minimum_block_height() override
    {
        return 610000;
    }
};

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
