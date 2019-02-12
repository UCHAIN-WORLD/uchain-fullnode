/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-explorer.
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
#ifndef BX_PROPERTY_TREE_HPP
#define BX_PROPERTY_TREE_HPP

#include <map>
#include <string>
#include <vector>
#include <UChain/bitcoin.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/config/header.hpp>
#include <UChain/explorer/config/input.hpp>
#include <UChain/explorer/config/output.hpp>
#include <UChain/explorer/config/point.hpp>
#include <UChain/explorer/config/transaction.hpp>
#include <UChain/explorer/config/wrapper.hpp>
#include <UChainService/txs/wallet/wallet.hpp>

#include <jsoncpp/json/json.h>

namespace libbitcoin
{
namespace explorer
{
namespace config
{

class base2;
class header;
class input;
class output;
class transaction;
class wrapper;

/**
* A tuple to represent settings and serialized values.
*/
typedef std::map<std::string, std::string> settings_list;

template <typename Value>
Json::Value &operator+=(Json::Value &a, const Value &b);

/**
* Create a string of value elements.
* @param      <Value>  The element type.
* @returns             A string of the element.
*/
template <typename Value>
std::string operator+(const Value &value);

struct json_helper
{

    explicit json_helper(int ver = 1) : version_(ver) {}

    /**
 * Create a property tree array of property tree elements.
 * The child elements of Value contain no arrays.
 * @param      <Values>  The array element type.
 * @param[in]  name      The name of the list elements.
 * @param[in]  values    The enumerable with elements of type Values.
 * @param[in]  json      Use json array formating.
 * @returns              A new property tree containing the list.
 */
    template <typename Values>
    Json::Value prop_tree_list(const std::string &name, const Values &values,
                               bool json);

    /**
 * Create a property tree array of property tree elements.
 * The child elements of Value contain arrays.
 * @param      <Values>  The array element type.
 * @param[in]  name      The name of the list elements.
 * @param[in]  values    The enumerable with elements of type Values.
 * @param[in]  json      Use json array formating.
 * @returns              A new property tree containing the list.
 */
    template <typename Values>
    Json::Value prop_tree_list_of_lists(const std::string &name,
                                        const Values &values, bool json);

    /**
 * Create a property tree array of value elements.
 * @param      <Values>  The array element type.
 * @param[in]  name      The name of the list elements.
 * @param[in]  values    The enumerable with elements of type Values.
 * @param[in]  json      Use json array formating.
 * @returns              A new property tree containing the list.
 */
    template <typename Values>
    Json::Value prop_value_list(const std::string &name, const Values &values,
                                bool json);

    /**
 * Generate a property list for a block header.
 * @param[in]  header  The header.
 * @return             A property list.
 */
    BCX_API Json::Value prop_list(const header &header);

    /**
 * Generate a property tree for a block header.
 * @param[in]  header  The header.
 * @return             A property tree.
 */
    BCX_API Json::Value prop_tree(const header &header);

    /**
 * Generate a property tree for a set of headers.
 * @param[in]  headers  The set of headers.
 * @return              A property tree.
 */
    BCX_API Json::Value prop_tree(const std::vector<header> &headers, bool json);

    /**
* Generate a property list for a history row.
* @param[in]  row  The history row.
* @return          A property list.
*/
    BCX_API Json::Value prop_list(const chain::history &row);

    /**
 * Generate a property tree for a history row.
 * @param[in]  row  The history row.
 * @return          A property tree.
 */
    BCX_API Json::Value prop_tree(const chain::history &row);

    /**
 * Generate a property tree for a set of history rows.
 *
 * @param[in]  rows  The set of history rows.
 * @param[in]  json  Use json array formatting.
 * @return           A property tree.
 */
    BCX_API Json::Value prop_tree(const chain::history::list &rows, bool json);

    /**
 * Generate a property list from balance rows for an address.
 * This doesn't require array formatting because it summarizes the rows.
 * @param[in]  rows             The set of balance rows.
 * @param[in]  balance_address  The payment address for the balance rows.
 * @return                      A property list.
 */
    BCX_API Json::Value prop_list(const chain::history::list &rows,
                                  const bc::wallet::payment_address &balance_address);

    /**
 * Generate a property tree from balance rows for an address.
 * This doesn't require array formatting because it summarizes the rows.
 * @param[in]  rows             The set of balance rows.
 * @param[in]  balance_address  The payment address for the balance rows.
 * @return                      A property tree.
 */
    BCX_API Json::Value prop_tree(const chain::history::list &rows,
                                  const bc::wallet::payment_address &balance_address);

    /**
 * Generate a property list for a transaction input.
 * @param[in]  tx_input  The input.
 * @return               A property list.
 */
    BCX_API Json::Value prop_list(const tx_input_type &tx_input);

    /**
 * Generate a property tree for a transaction input.
 * @param[in]  tx_input  The input.
 * @return               A property tree.
 */
    BCX_API Json::Value prop_tree(const tx_input_type &tx_input);

    /**
 * Generate a property tree for a set of transaction inputs.
 * @param[in]  tx_inputs  The set of transaction inputs.
 * @param[in]  json       Use json array formatting.
 * @return                A property tree.
 */
    BCX_API Json::Value prop_tree(const tx_input_type::list &tx_inputs, bool json);

    /**
 * Generate a property list for an input.
 * @param[in]  input  The input.
 * @return            A property list.
 */
    BCX_API Json::Value prop_list(const explorer::config::input &input);

    /**
 * Generate a property tree for an input.
 * @param[in]  input  The input.
 * @return            A property tree.
 */
    BCX_API Json::Value prop_tree(const explorer::config::input &input);

    /**
 * Generate a property tree for a set of inputs.
 * @param[in]  inputs  The set of inputs.
 * @param[in]  json    Use json array formatting.
 * @return             A property tree.
 */
    BCX_API Json::Value prop_tree(
        const std::vector<explorer::config::input> &inputs, bool json);

    /**
 * Generate a property list for a transaction output.
 * @param[in]  tx_output  The transaction output.
 * @return                A property list.
 */
    BCX_API Json::Value prop_list(const tx_output_type &tx_output);
    /**
 * Generate a property list for a asset.
 * @param[in]  output_attach  The asset in output.
 * @return                A property list.
 */
    BCX_API Json::Value prop_list(const tx_output_type &tx_output, uint32_t index);

    BCX_API Json::Value prop_list(bc::chain::asset &output_attach);
    /**
 * Generate a property tree for a transaction output.
 * @param[in]  tx_output  The transaction output.
 * @return                A property tree.
 */
    BCX_API Json::Value prop_tree(const tx_output_type &tx_output);

    /**
 * Generate a property tree for a set of transaction outputs.
 * @param[in]  tx_outputs  The set of transaction outputs.
 * @param[in]  json        Use json array formatting.
 * @return                 A property tree.
 */
    BCX_API Json::Value prop_tree(const tx_output_type::list &tx_outputs,
                                  bool json);

    /**
 * Generate a property list for a point.
 * @param[in]  p          The point.
 * @return                A property list.
 */
    BCX_API Json::Value prop_list(const chain::point &point);

    /**
 * Generate a property tree for a set of points.
 * @param[in]  points  The set of points.
 * @param[in]  json        Use json array formatting.
 * @return                 A property tree.
 */
    BCX_API Json::Value prop_tree(const chain::point::list &points, bool json);

    /**
 * Generate a property tree for a points_info.
 * @param[in]  p_info       The points_info.
 * @param[in]  json         Use json array formatting.
 * @return                  A property tree.
 */
    BCX_API Json::Value prop_tree(const chain::points_info &points_info, bool json);

    /**
 * Generate a property list for a transaction.
 * @param[in]  transaction  The transaction.
 * @param[in]  json         Use json array formatting.
 * @return                  A property list.
 */
    BCX_API Json::Value prop_list(const transaction &transaction, bool json);
    BCX_API Json::Value prop_list(const transaction &transaction, uint64_t tx_height, bool json);

    BCX_API Json::Value prop_list_of_rawtx(const transaction &transaction, bool with_hash, bool ignore_compatibility = false);

    /**
 * Generate a property tree for a transaction.
 * @param[in]  transaction  The transaction.
 * @param[in]  json         Use json array formatting.
 * @return                  A property tree.
 */
    BCX_API Json::Value prop_tree(const transaction &transaction, bool json);

    /**
 * Generate a property tree for a set of transactions.
 * @param[in]  transactions  The set of transactions.
 * @param[in]  json          Use json array formatting.
 * @return                   A property tree.
 */
    BCX_API Json::Value prop_tree(const std::vector<transaction> &transactions,
                                  bool json);

    /**
 * Generate a property list for a wrapper.
 * @param[in]  wrapper  The wrapper instance.
 * @return              A property list.
 */
    BCX_API Json::Value prop_list(const bc::wallet::wrapped_data &wrapper);

    /**
 * Generate a property tree for a wrapper.
 * @param[in]  wrapper  The wrapper instance.
 * @return              A property tree.
 */
    BCX_API Json::Value prop_tree(const bc::wallet::wrapped_data &wrapper);

    /**
 * Generate a property list for transaction with extended data.
 * @param[in]  tx          The transaction.
 * @param[in]  block_hash  The block_hash of the transaction.
 * @param[in]  address     The address used to locate the transaction.
 * @param[in]  json        Use json array formatting.
 * @return                 A property list.
 */
    BCX_API Json::Value prop_list(const tx_type &tx, const hash_digest &block_hash,
                                  const bc::wallet::payment_address &address, bool json);

    /**
 * Generate a property tree for transaction with extended data.
 * @param[in]  tx          The transaction.
 * @param[in]  block_hash  The block_hash of the transaction.
 * @param[in]  address     The address used to locate the transaction.
 * @param[in]  json        Use json array formatting.
 * @return                 A property tree.
 */
    BCX_API Json::Value prop_tree(const tx_type &tx, const hash_digest &block_hash,
                                  const bc::wallet::payment_address &address, bool json);

    /**
 * Generate a property list for a stealth address.
 * @param[in]  stealth_address  The stealth address.
 * @param[in]  json             Use json array formatting.
 * @return                      A property list.
 */
    BCX_API Json::Value prop_list(const bc::wallet::stealth_address &stealth, bool json);

    /**
 * Generate a property tree for a stealth address.
 * @param[in]  stealth_address  The stealth address.
 * @param[in]  json             Use json array formatting.
 * @return                      A property tree.
 */
    BCX_API Json::Value prop_tree(const bc::wallet::stealth_address &stealth, bool json);

    /**
 * Generate a property list for a stealth metadata row.
 * @param[in]  rows  The stealth row.
 * @return           A property list.
 */
    BCX_API Json::Value prop_list(const chain::stealth &row);

    /**
 * Generate a property tree for a stealth metadata row.
 * @param[in]  rows  The stealth row.
 * @return           A property tree.
 */
    BCX_API Json::Value prop_tree(const chain::stealth &row);

    /**
 * Generate a property tree from stealth metadata rows.
 * @param[in]  rows    The set of stealth rows.
 * @param[in]  json    Use json array formatting.
 * @return             A property tree.
 */
    BCX_API Json::Value prop_tree(const chain::stealth::list &rows, bool json);

    /**
 * Create a property list for the fetch-tx-index command.
 * @param[in]  hash    The block hash.
 * @param[in]  height  The block height.
 * @param[in]  index   The tx index.
 * @returns            A new property list containing the list.
 */
    BCX_API Json::Value prop_list(const bc::hash_digest &hash, size_t height,
                                  size_t index);

    /**
 * Create a property tree for the fetch-tx-index command.
 * @param[in]  hash    The block hash.
 * @param[in]  height  The block height.
 * @param[in]  index   The tx index.
 * @returns            A new property tree containing the list.
 */
    BCX_API Json::Value prop_tree(const bc::hash_digest &hash, size_t height,
                                  size_t index);

    /**
 * Create a property tree for the settings command.
 * @param[in]  settings   The list of settings.
 * @returns               A new property tree containing the settings.
 */
    BCX_API Json::Value prop_tree(const settings_list &settings);

    /**
 * Create a property tree for a parsed bitcoin uri.
 * @param[in]  uri   The parsed uri.
 * @returns          A new property tree containing the settings.
 */
    BCX_API Json::Value prop_tree(const bc::wallet::bitcoin_uri &uri);

    /**
 * Create a property tree for a parsed bitcoin uri.
 * @param[in]  block The parsed block.
 * @param[in]  json  json output.
 * @param[in]  tx_json   json output for tx within this block.
 * @returns          A new property tree containing the settings.
 */
    BCX_API Json::Value prop_tree(const block &block, bool json, bool tx_json);

    /**
 * Generate a property list for a token detail.
 * @param[in]  detail_info          The token detail.
 * @param[in]  is_maximum_supply    The token amount means maximum_supply or quantity.
 * @param[in]  show_address         A boolean value indicate whether show address.
 * @return             A property list.
 */
    BCX_API Json::Value prop_list(const bc::chain::token_detail &detail_info, bool is_maximum_supply, bool show_address = true);

    /**
 * Generate a property list for a token transfer with detail.
 * @param[in]  balance_info The token balance info.
 * @param[in]  issued_info  The issued token detail. include the other info.
 * @param[in]  show_address A boolean value indicate whether show address.
 * @return             A property list.
 */
    BCX_API Json::Value prop_list(const bc::chain::token_balances &balance_info, const bc::chain::token_detail &issued_info, bool show_address = true);

    /**
 * Generate a property list for a token transfer with detail.
 * @param[in]  balance_info The token balance info.
 * @return             A property list.
 */
    BCX_API Json::Value prop_list(const bc::chain::token_balances &balance_info);

    /**
 * Generate a property list for a token transfer with detail.
 * @param[in]  balance_info The deoisuted token balance info.
 * @param[in]  issued_info  The issued token detail. include the other info.
 * @param[in]  show_address A boolean value indicate whether show address.
 * @return             A property list.
 */
    BCX_API Json::Value prop_list(const bc::chain::token_deposited_balance &balance_info, const bc::chain::token_detail &issued_info, bool show_address = true);

    /**
 * Generate a property list for a token transfer.
 * @param[in]  trans_info        The token transfer.
 * @param[in]  decimal_number    The token transfer unit. if equals max_uint8, do not add "decimal_number" index.
 * @return             A property list.
 */
    BCX_API Json::Value prop_list(const bc::chain::token_transfer &trans_info, uint8_t decimal_number = max_uint8);

    /**
 * Generate a property list for a token cert.
 * @param[in]  cert_info        The token cert.
 * @return             A property list.
 */
    BCX_API Json::Value prop_list(const bc::chain::token_cert &cert_info);

    /**
 * Generate a property list for an identifiable token.
 * @param[in]  cert_info        The identifiable token.
 * @return             A property list.
 */
    BCX_API Json::Value prop_list(const bc::chain::candidate &candidate_info, bool always_show_content = false);
    BCX_API Json::Value prop_list(const bc::chain::candidate_info &candidate_info, bool always_show_content = false, bool show_vote = false);

    /**
 * Generate a property list for a multisign.
 * @param[in]  multisign        The multisign.
 * @return             A property list.
 */
    BCX_API Json::Value prop_list(const bc::chain::wallet_multisig &multisign);

    /**
 * Generate a property list for a attenuation_model_param.
 * @param[in]  param            The parameter of attenuation model.
 * @return             A property list.
 */
    BCX_API Json::Value prop_attenuation_model_param(const data_chunk &param);
    BCX_API Json::Value prop_attenuation_model_param(const std::string &param);

    /**
 * Generate a property list for an wallet.
 * @param[in]  acc        The wallet.
 * @return             A property list.
 */
    typedef std::tuple<std::string, std::string, Json::Value> wallet_info;
    BCX_API Json::Value prop_list(const wallet_info &acc);

  private:
    uint8_t version_{1}; //1 - api v1; 2 - api v2;
};

} // namespace config
} // namespace explorer
} // namespace libbitcoin

#include <UChain/explorer/impl/json_helper.ipp>

#endif
