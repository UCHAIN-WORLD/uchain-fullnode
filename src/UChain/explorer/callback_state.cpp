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

#include <UChain/explorer/callback_state.hpp>

#include <iostream>
#include <cstdint>
#include <string>
#include <boost/format.hpp>
#include <UChain/explorer/config/encoding.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/json_helper.hpp>
#include <UChain/explorer/utility.hpp>

namespace libbitcoin
{
namespace explorer
{

callback_state::callback_state(std::ostream &error, std::ostream &output,
                               const encoding_engine engine)
    : stopped_(true), refcount_(0), result_(console_result::okay),
      engine_(engine), error_(error), output_(output)

{
}

callback_state::callback_state(std::ostream &error, std::ostream &output)
    : callback_state(error, output, encoding_engine::info)
{
}

// std::endl adds "/n" and flushes the stream.
void callback_state::error(const Json::Value &tree)
{
    write_stream(error_, tree, engine_);
}

// std::endl adds "/n" and flushes the stream.
void callback_state::error(const format &message)
{
    error_ << message;
    error_.flush();
}

void callback_state::error(const std::string &message)
{
    error(format(message));
}

void callback_state::output(const Json::Value &tree)
{
    write_stream(output_, tree, engine_);
}

// std::endl adds "/n" and flushes the stream.
void callback_state::output(const format &message)
{
    output_ << message;
    output_.flush();
}

void callback_state::output(const std::string &message)
{
    output(format(message));
}

void callback_state::output(uint64_t value)
{
    output(format("%1%") % value);
}

void callback_state::start()
{
    refcount_ = 1;
    stopped_ = false;
    result_ = console_result::okay;
}

void callback_state::stop(console_result result)
{
    refcount_ = 0;
    stopped_ = true;
    result_ = result;
}

bool &callback_state::stopped()
{
    return stopped_;
}

bool callback_state::succeeded(const code &ec, const std::string &format)
{
    if (ec)
    {
        // May want to change the behavior to decrement vs. zeroizing refs.
        error(boost::format(format) % ec.message());
        stop(console_result::failure);
        return false;
    }

    return true;
}

encoding_engine callback_state::get_engine()
{
    return engine_;
}

console_result callback_state::get_result()
{
    return result_;
}

void callback_state::set_result(console_result result)
{
    result_ = result;
}

size_t callback_state::increment()
{
    if (++refcount_ != 0)
        stopped_ = false;

    return refcount_;
}

size_t callback_state::decrement()
{
    if (--refcount_ == 0)
        stopped_ = true;

    return refcount_;
}

callback_state::operator size_t() const
{
    return refcount_;
}

callback_state &callback_state::operator++()
{
    increment();
    return *this;
}

callback_state &callback_state::operator--()
{
    decrement();
    return *this;
}

} // namespace explorer
} // namespace libbitcoin
