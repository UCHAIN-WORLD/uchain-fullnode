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
#include <UChain/network/hosts.hpp>

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>
#include <UChainService/txs/utility/path.hpp>
#include <UChain/network/settings.hpp>

namespace libbitcoin
{
namespace network
{

#define NAME "hosts"

hosts::hosts(threadpool &pool, const settings &settings)
    : stopped_(true),
      dispatch_(pool, NAME),
      file_path_(settings.hosts_file == "hosts.cache" ? (default_data_path() / settings.hosts_file) : settings.hosts_file),
      disabled_(settings.host_pool_capacity == 0),
      pool_(pool),
      seed_count(settings.seeds.size())
{
    //    buffer_.reserve(std::max(settings.host_pool_capacity, 1u));
}

// private
hosts::iterator hosts::find(const address &host)
{
    const auto found = [&host](const address &entry) {
        return entry.port == host.port && entry.ip == host.ip;
    };
    return buffer_.find(host);
    //    return std::find_if(buffer_.begin(), buffer_.end(), found);
}

size_t hosts::count() const
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    shared_lock lock(mutex_);

    return buffer_.size();
    ///////////////////////////////////////////////////////////////////////////
}

static std::atomic<uint64_t> fetch_times{0};

static std::vector<config::authority> hosts_{config::authority("198.199.84.199:5252")};

code hosts::fetch(address &out, const config::authority::list &excluded_list)
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    shared_lock lock(mutex_);
    fetch_times++;
    config::authority::list addresses;
    list *buffer = nullptr;
    {

        if (stopped_)
        {
            return error::service_stopped;
        }

        if (fetch_times % 5 == 4 && !buffer_.empty())
            buffer = &buffer_;
        else
            buffer = &inactive_;
    }

    for (auto entry : *buffer)
    {
        auto iter = std::find(excluded_list.begin(), excluded_list.end(), config::authority(entry));
        if (iter == excluded_list.end())
        {
            addresses.push_back(config::authority(entry));
        }
    }

    if (addresses.empty())
    {
        if (inactive_.empty())
        {
            return error::not_found;
        }
        const auto index = static_cast<size_t>(pseudo_random() % inactive_.size());

        size_t i = 0;
        for (const auto &entry : inactive_)
        {
            if (i == index)
            {
                out = entry;
                break;
            }
            i++;
        }

        return error::success;
    }

    const auto index = static_cast<size_t>(pseudo_random() % addresses.size());
    out = addresses[index].to_network_address();

    //    const auto index = static_cast<size_t>(pseudo_random() % hosts_.size());
    //    out = hosts_[index].to_network_address();
    return error::success;
    ///////////////////////////////////////////////////////////////////////////
}

hosts::address::list hosts::copy()
{
    address::list copy;

    shared_lock lock{mutex_};
    copy.reserve(buffer_.size());
    for (auto &h : buffer_)
    {
        copy.push_back(h);
    }
    return copy;
}

void hosts::handle_timer(const code &ec)
{
    if (ec.value() != error::success)
    {
        return;
    }

    mutex_.lock_upgrade();

    if (stopped_)
    {
        mutex_.unlock_upgrade();
        return;
    }

    mutex_.unlock_upgrade_and_lock();
    bc::ofstream file(file_path_.string());
    const auto file_error = file.bad();

    if (!file_error)
    {
        log::debug(LOG_NETWORK) << "sync hosts to file(" << file_path_.string() << "), active hosts size is "
                                << buffer_.size() << " hosts found, inactive hosts size is " << inactive_.size();
        for (const auto &entry : buffer_)
            file << config::authority(entry) << std::endl;
        for (const auto &entry : inactive_)
            file << config::authority(entry) << std::endl;
    }
    else
    {
        log::error(LOG_NETWORK) << "hosts file (" << file_path_.string() << ") open failed";
        mutex_.unlock();
        return;
    }

    mutex_.unlock();
    snap_timer_->start(std::bind(&hosts::handle_timer, shared_from_this(), std::placeholders::_1));
}

// load
code hosts::start()
{
    if (disabled_)
        return error::success;
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    if (!stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return error::operation_failed;
    }

    mutex_.unlock_upgrade_and_lock();
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    snap_timer_ = std::make_shared<deadline>(pool_, asio::seconds(60));
    snap_timer_->start(std::bind(&hosts::handle_timer, shared_from_this(), std::placeholders::_1));
    stopped_ = false;
    bc::ifstream file(file_path_.string());
    const auto file_error = file.bad();

    if (!file_error)
    {
        std::string line;

        while (std::getline(file, line))
        {
            config::authority host(line);

            if (host.port() != 0)
            {
                auto network_address = host.to_network_address();
                if (network_address.is_routable())
                {
                    inactive_.insert(network_address);
                }
                else
                {
                    log::debug(LOG_NETWORK) << "host start is not routable," << config::authority{network_address};
                }
            }
        }
    }

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    if (file_error)
    {
        log::debug(LOG_NETWORK)
            << "Failed to save hosts file.";
        return error::file_system;
    }

    return error::success;
}

// load
code hosts::stop()
{
    if (disabled_)
        return error::success;

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    if (stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return error::success;
    }

    mutex_.unlock_upgrade_and_lock();
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    snap_timer_->stop();
    stopped_ = true;
    bc::ofstream file(file_path_.string());
    const auto file_error = file.bad();

    if (!file_error)
    {
        for (const auto &entry : buffer_)
            file << config::authority(entry) << std::endl;

        buffer_.clear();
    }

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    if (file_error)
    {
        log::debug(LOG_NETWORK)
            << "Failed to load hosts file.";
        return error::file_system;
    }

    return error::success;
}

code hosts::clear()
{
    // Critical Section
    mutex_.lock_upgrade();

    if (stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return error::service_stopped;
    }

    mutex_.unlock_upgrade_and_lock();

    // if the buffer is already moved to backup, call this function again will lead to the loss of backup.
    // backup_ = std::move( buffer_ );
    for (auto &host : buffer_)
    {
        backup_.insert(host);
    }
    buffer_.clear();

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    return error::success;
}

code hosts::after_reseeding()
{
    mutex_.lock_upgrade();

    if (stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return error::service_stopped;
    }

    mutex_.unlock_upgrade_and_lock();
    //re-seeding failed and recover the buffer with backup one
    if (buffer_.size() <= seed_count)
    {
        log::warning(LOG_NETWORK) << "Reseeding finished, but got address list: " << buffer_.size() << ", less than seed count: "
                                  << seed_count << ", roll back the hosts cache.";
        buffer_ = std::move(backup_);
    }
    else
    {
        // filter inactive hosts
        for (auto &host : inactive_)
        {
            auto iter = buffer_.find(host);
            if (iter != buffer_.end())
            {
                buffer_.erase(iter);
            }
        }

        // clear the backup
        backup_.clear();
    }

    log::debug(LOG_NETWORK) << "Reseeding finished, and got addresses of count: " << buffer_.size();

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    return error::success;
}

code hosts::remove(const address &host)
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    if (stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return error::service_stopped;
    }

    auto it = find(host);

    if (it != buffer_.end())
    {
        mutex_.unlock_upgrade_and_lock();
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        buffer_.erase(it);

        mutex_.unlock();
        //---------------------------------------------------------------------
        return error::success;
    }

    mutex_.unlock_upgrade_and_lock();
    auto iter = inactive_.find(host);
    if (iter == inactive_.end())
    {
        inactive_.insert(host);
    }
    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    return error::success;
}

code hosts::store(const address &host)
{
    if (!host.is_routable())
    {
        // We don't treat invalid address as an error, just log it.
        return error::success;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    if (stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return error::service_stopped;
    }

    if (find(host) == buffer_.end())
    {
        mutex_.unlock_upgrade_and_lock();
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        buffer_.insert(host);
        auto iter = inactive_.find(host);
        if (iter != inactive_.end())
        {
            inactive_.erase(iter);
        }

        mutex_.unlock();
        //---------------------------------------------------------------------
        return error::success;
    }

    mutex_.unlock_upgrade();
    ///////////////////////////////////////////////////////////////////////////

    //    log::trace(LOG_NETWORK)
    //        << "Redundant host address from peer";

    // We don't treat redundant address as an error, just log it.
    return error::success;
}

// private
void hosts::do_store(const address &host, result_handler handler)
{
    handler(store(host));
}

// The handler is invoked once all calls to do_store are completed.
// We disperse here to allow other addresses messages to interleave hosts.
void hosts::store(const address::list &hosts, result_handler handler)
{
    if (stopped_)
        return;

    dispatch_.parallel(hosts, "hosts", handler,
                       &hosts::do_store, shared_from_this());
}

} // namespace network
} // namespace libbitcoin
