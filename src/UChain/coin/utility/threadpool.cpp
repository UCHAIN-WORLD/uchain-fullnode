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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <UChain/coin/utility/threadpool.hpp>

#include <memory>
#include <new>
#include <thread>
#include <UChain/coin/utility/asio.hpp>
#include <UChain/coin/utility/thread.hpp>

namespace libbitcoin
{

threadpool::threadpool(size_t number_threads, thread_priority priority)
{
    spawn(number_threads, priority);
}

threadpool::~threadpool()
{
    shutdown();
    join();
}

void threadpool::spawn(size_t number_threads, thread_priority priority)
{
    for (size_t i = 0; i < number_threads; ++i)
        spawn_once(priority);
}

void threadpool::spawn_once(thread_priority priority)
{
    // In C++14 work should use a unique_ptr.
    // Work prevents the service from running out of work and terminating.
    if (!work_)
        work_ = std::make_shared<asio::service::work>(service_);

    const auto action = [this, priority] {
        set_thread_priority(priority);
        service_.run();
    };

    threads_.push_back(asio::thread(action));
}

void threadpool::abort()
{
    service_.stop();
}

void threadpool::shutdown()
{
    work_ = nullptr;
}

void threadpool::join()
{
    for (auto &thread : threads_)
        if (thread.joinable())
            thread.join();

    // This allows the pool to be cleanly restarted by calling spawn.
    threads_.clear();
    service_.reset();
}

asio::service &threadpool::service()
{
    return service_;
}

const asio::service &threadpool::service() const
{
    return service_;
}

} // namespace libbitcoin
