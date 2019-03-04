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
#include <UChain/explorer/config/output.hpp>

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <UChain/bitcoin.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/config/script.hpp>
#include <UChain/explorer/utility.hpp>

using namespace po;

namespace libbitcoin
{
namespace explorer
{
namespace config
{

output::output()
    : amount_(0), version_(0), script_(), pay_to_hash_(null_short_hash),
      ephemeral_data_({})
{
}

output::output(const std::string &tuple)
    : output()
{
    std::stringstream(tuple) >> *this;
}

uint64_t output::amount() const
{
    return amount_;
}

uint8_t output::version() const
{
    return version_;
}

const chain::script &output::script() const
{
    return script_;
}

const short_hash &output::pay_to_hash() const
{
    return pay_to_hash_;
}

const data_chunk &output::ephemeral_data() const
{
    return ephemeral_data_;
}

std::istream &operator>>(std::istream &input, output &argument)
{
    std::string tuple;
    input >> tuple;

    const auto tokens = split(tuple, BX_TX_POINT_DELIMITER);
    if (tokens.size() < 2 || tokens.size() > 3)
    {
        BOOST_THROW_EXCEPTION(invalid_option_value(tuple));
    }

    uint64_t amount;
    deserialize(amount, tokens[1], true);
    if (amount > max_money())
    {
        BOOST_THROW_EXCEPTION(invalid_option_value(tuple));
    }

    argument.amount_ = amount;
    const auto &target = tokens.front();

    // Is the target a payment address?
    const bc::wallet::payment_address payment(target);
    if (payment)
    {
        argument.version_ = payment.version();
        argument.pay_to_hash_ = payment.hash();
        return input;
    }

    // Is the target a stealth address?
    const bc::wallet::stealth_address stealth(target);
    if (stealth)
    {
        // TODO: finish stealth multisig implemetation (p2sh and !p2sh).

        if (stealth.spend_keys().size() != 1 || tokens.size() != 3)
        {
            BOOST_THROW_EXCEPTION(invalid_option_value(target));
        }

        data_chunk seed;
        if (!decode_base16(seed, tokens[2]) || seed.size() < minimum_seed_size)
        {
            BOOST_THROW_EXCEPTION(invalid_option_value(target));
        }

        ec_secret ephemeral_secret;
        if (!create_stealth_data(argument.ephemeral_data_, ephemeral_secret,
                                 stealth.filter(), seed))
        {
            BOOST_THROW_EXCEPTION(invalid_option_value(target));
        }

        ec_compressed stealth_key;
        if (!uncover_stealth(stealth_key, stealth.scan_key(), ephemeral_secret,
                             stealth.spend_keys().front()))
        {
            BOOST_THROW_EXCEPTION(invalid_option_value(target));
        }

        argument.pay_to_hash_ = bitcoin_short_hash(stealth_key);
        argument.version_ = stealth.version();
        return input;
    }

    // The target must be a serialized script.
    data_chunk decoded;
    if (!decode_base16(decoded, target))
    {
        BOOST_THROW_EXCEPTION(invalid_option_value(target));
    }

    argument.script_ = script(decoded);
    return input;
}

} // namespace config
} // namespace explorer
} // namespace libbitcoin
