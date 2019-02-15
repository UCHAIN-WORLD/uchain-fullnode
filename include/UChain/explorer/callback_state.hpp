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
#ifndef BX_CALLBACK_STATE_HPP
#define BX_CALLBACK_STATE_HPP

#include <iostream>
#include <cstdint>
#include <string>
#include <boost/format.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/config/encoding.hpp>

/* NOTE: don't declare 'using namespace foo' in headers. */

namespace libbitcoin
{
namespace explorer
{

/**
 * Shared state wrapper to manage non-global shared call state.
 */
class callback_state
{
  public:
    /**
     * Construct an instance of the callback_state class. The class is
     * initialized with a reference count of zero (0). If the streams
     * references passed here are used elsewhere or in another instance
     * on another thread then stream interleaving cannot be prevented.
     * @param[in]  error   The error stream for the callback handler.
     * @param[in]  output  The output stream for the callback handler.
     * @param[in]  engine  The desired output format.
     */
    BCX_API callback_state(std::ostream &error, std::ostream &output,
                           const encoding_engine engine);

    /**
     * Construct an instance of the callback_state class with native encoding.
     * @param[in]  error   The error stream for the callback handler.
     * @param[in]  output  The output stream for the callback handler.
     */
    BCX_API callback_state(std::ostream &error, std::ostream &output);

    /**
     * Serialize a property tree to output. The stream must be flushed before
     * returning in order to prevent interleaving on the shared stream.
     * @param[in]  tree  The property tree to write to output.
     */
    BCX_API virtual void error(const Json::Value &tree);

    /**
     * Write a line to the error stream. The stream must be flushed before
     * returning in order to prevent interleaving on the shared stream.
     * @param[in]  message  The unterminated error message to write.
     */
    BCX_API virtual void error(const format &message);

    /**
     * Write a line to the error stream. The stream must be flushed before
     * returning in order to prevent interleaving on the shared stream.
     * @param[in]  message  The unterminated error message to write.
     */
    BCX_API virtual void error(const std::string &message);

    /**
     * Serialize a property tree to output. The stream must be flushed before
     * returning in order to prevent interleaving on the shared stream.
     * @param[in]  tree  The property tree to write to output.
     */
    BCX_API virtual void output(const Json::Value &tree);

    /**
     * Write a line to the output stream. The stream must be flushed before
     * returning in order to prevent interleaving on the shared stream.
     * @param[in]  message  The unterminated output message to write.
     */
    BCX_API virtual void output(const format &message);

    /**
     * Write a line to the output stream. The stream must be flushed before
     * returning in order to prevent interleaving on the shared stream.
     * @param[in]  message  The unterminated output message to write.
     */
    BCX_API virtual void output(const std::string &message);

    /**
     * Write a number to the output stream. The stream must be flushed before
     * returning in order to prevent interleaving on the shared stream.
     * @param[in]  value  The numeric value to write.
     */
    BCX_API virtual void output(uint64_t value);

    /**
     * Set the callback refcount to one and reset result to okay.
     */
    BCX_API virtual void start();

    /**
     * Set the callback refcount to zero and assign the result.
     * This overrides any outstanding callback references.
     * @param[in]  result  The desired callback result code, defaults to okay.
     */
    BCX_API virtual void stop(console_result result = console_result::okay);

    /**
     * Get a value indicating whether the callback reference count is zero.
     * @return  True if the reference count is zero.
     */
    BCX_API virtual bool &stopped();

    /**
     * Handle the callback error with standard behavior.
     * @param[in]  ec      The callback error result.
     * @param[in]  format  A single parameter format string or empty/default.
     * @return             True if no error.
     */
    BCX_API virtual bool succeeded(const std::error_code &ec,
                                   const std::string &format = "%1%");

    /**
     * Get the engine enumeration value.
     */
    BCX_API virtual encoding_engine get_engine();

    /**
     * Get the callback result code.
     */
    BCX_API virtual console_result get_result();

    /**
     * Set the callback result code.
     */
    BCX_API virtual void set_result(console_result result);

    /**
     * Increment the callback synchronization reference count.
     * @return  The callback synchronization counter value.
     */
    BCX_API virtual size_t increment();

    /**
     * Decrement the callback synchronization reference count.
     * @return  The callback synchronization counter value.
     */
    BCX_API virtual size_t decrement();

    /**
     * Overload numeric cast to return the reference count.
     */
    BCX_API virtual operator size_t() const;

    /**
     * Overload ++ operator to increment the reference count.
     */
    BCX_API virtual callback_state &operator++();

    /**
     * Overload ++ operator to decrement the reference count.
     */
    BCX_API virtual callback_state &operator--();

  private:
    bool stopped_;
    size_t refcount_;
    console_result result_;
    encoding_engine engine_;
    std::ostream &error_;
    std::ostream &output_;
};

} // namespace explorer
} // namespace libbitcoin

#endif
