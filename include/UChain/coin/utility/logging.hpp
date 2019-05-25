/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain.
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
#ifndef UC_NETWORK_LOGGING_HPP
#define UC_NETWORK_LOGGING_HPP

#include <fstream>
#include <iostream>
#include <UChain/coin.hpp>
#include <UChain/network/define.hpp>

namespace libbitcoin
{

/// Set up global logging.
BCT_API void initialize_logging(bc::ofstream &debug, bc::ofstream &error,
                                std::ostream &output_stream, std::ostream &error_stream, std::string level = "DEBUG");

/// Class Logger
class Logger
{
#define self Logger

  public:
    self() noexcept
    {
        initialize_logging(debug_log_, error_log_, std::cout, std::cerr);
    }

    self(const self &) = delete;
    self(const self &&) = delete;

    ~self() noexcept
    {
        log::clear();
        debug_log_.close();
        error_log_.close();
    }

  public:
    /// Constant for logging file open mode (append output).
    static BC_CONSTEXPR std::ofstream::openmode log_open_mode =
        std::ofstream::out | std::ofstream::app;

  private:
    bc::ofstream debug_log_{"debug.log", log_open_mode};
    bc::ofstream error_log_{"error.log", log_open_mode};

#undef self
}; // class Logger

} // namespace libbitcoin

#endif
