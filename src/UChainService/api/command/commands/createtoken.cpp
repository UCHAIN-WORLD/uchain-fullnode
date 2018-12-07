/**
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

#include <UChain/explorer/json_helper.hpp>
#include <UChain/explorer/dispatch.hpp>
#include <UChainService/api/command/commands/createtoken.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChainService/api/command/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;
/************************ createtoken *************************/

void validate(boost::any& v,
    const std::vector<std::string>& values, non_negative_uint64*, int)
{
    using namespace boost::program_options;
    validators::check_first_occurrence(v);

    std::string const& s = validators::get_single_string(values);
    if (s[0] == '-') {
        throw argument_legality_exception{"volume cannot be anegative number."};
    }
    v = boost::any(non_negative_uint64 { boost::lexical_cast<uint64_t>(s) } );
}

console_result createtoken::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(option_.symbol);

    // check token symbol
    check_token_symbol(option_.symbol, true);

    // check uid symbol
    auto issued_uid = option_.issuer;
    check_uid_symbol(issued_uid);

    if (option_.description.length() > TOKEN_DETAIL_DESCRIPTION_FIX_SIZE)
        throw token_description_length_exception{"token description length must be less than 64."};
    auto threshold = option_.registersecondarytoken_threshold;
    if ((threshold < -1) || (threshold > 100)) {
        throw token_secondaryissue_threshold_exception{
            "registersecondarytoken threshold value error, it must be -1 or in the interval 0 to 100."};
    }

    if (option_.decimal_number > 19u)
        throw token_amount_exception{"token decimal number must less than 20."};
    if (option_.maximum_supply.volume == 0u)
        throw argument_legality_exception{"volume cannot be zero."};

    // check uid exists
    if (!blockchain.is_uid_exist(issued_uid)) {
        throw uid_symbol_notfound_exception{
            "The uid '" + issued_uid + "' does not exist on the blockchain, maybe you should registeruid first"};
    }

    // check uid is owned by the account
    if (!blockchain.is_account_owned_uid(auth_.name, issued_uid)) {
        throw uid_symbol_notowned_exception{
            "The uid '" + issued_uid + "' is not owned by " + auth_.name};
    }

    // check token exists
    if (blockchain.is_token_exist(option_.symbol, true))
        throw token_symbol_existed_exception{"symbol is already used."};

    // local database token check
    auto sh_token = blockchain.get_account_unissued_token(auth_.name, option_.symbol);
    if (sh_token) {
        throw token_symbol_duplicate_exception{option_.symbol
            + " already created, you can delete and recreate it."};
    }

    auto acc = std::make_shared<token_detail>();
    acc->set_symbol(option_.symbol);
    acc->set_maximum_supply(option_.maximum_supply.volume);
    acc->set_decimal_number(static_cast<uint8_t>(option_.decimal_number));
    acc->set_issuer(issued_uid);
    acc->set_description(option_.description);
    // use 127 to represent freely secondary issue, and 255 for its secondary issued status.
    acc->set_secondaryissue_threshold((threshold == -1) ?
        token_detail::freely_secondaryissue_threshold : static_cast<uint8_t>(threshold));

    blockchain.store_account_token(acc, auth_.name);

    if (get_api_version() <= 2) {
        Json::Value token_data = config::json_helper(get_api_version()).prop_list(*acc, true);
        token_data["status"] = "unissued";
        jv_output["token"] = token_data;
    }
    else {
        jv_output = config::json_helper(get_api_version()).prop_list(*acc, true);
        jv_output["status"] = "unissued";
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

