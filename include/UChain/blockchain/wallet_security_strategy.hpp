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

#ifndef UChain_WALLET_SECURITY_STRATEGY_HPP
#define UChain_WALLET_SECURITY_STRATEGY_HPP

#include <cstdint>
#include <string>

#include <UChain/coin/utility/thread.hpp>

namespace libbitcoin
{
namespace blockchain
{
enum class auth_type : uint8_t
{
    AUTH_PASSWD = 0,
    AUTH_LASTWD = 1,

    AUTH_TYPE_CNT,
};

struct WalletInfo
{
    uint8_t counter;
    uint32_t lock_start;

    void lock();
};

class wallet_security_strategy
{
  public:
    static wallet_security_strategy *get_instance();

    void check_locked(const std::string &wallet_name);
    void on_auth_passwd(const std::string &wallet_name, const bool &result);
    void on_auth_lastwd(const std::string &wallet_name, const bool &result);

  private:
    wallet_security_strategy(const uint8_t &passwd_max_try, const uint8_t &lastwd_max_try, const uint32_t &max_lock_time);
    virtual ~wallet_security_strategy();

    wallet_security_strategy(const wallet_security_strategy &) = delete;
    void operator=(const wallet_security_strategy &) = delete;

    void on_auth(const std::string &wallet_name, const bool &result, const auth_type &type);

    const uint8_t MAX_TRY[static_cast<uint8_t>(auth_type::AUTH_TYPE_CNT)];
    const uint32_t MAX_LOCK_TIME; //seconds

    std::map<const std::string, WalletInfo> acc_info_[static_cast<uint8_t>(auth_type::AUTH_TYPE_CNT)];

    mutable shared_mutex mutex_;

    static wallet_security_strategy *instance;
};

} // namespace blockchain
} // namespace libbitcoin
#endif //UChain_WALLET_SECURITY_STRATEGY_HPP
