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
#ifndef UC_TRACK_IPP
#define UC_TRACK_IPP

#include <atomic>
#include <cstddef>
#include <string>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/utility/assert.hpp>
#include <UChain/bitcoin/utility/log.hpp>
#define RESOURCE_INSCREASE
template <class Shared>
std::atomic<size_t> track<Shared>::instances(0);

template <class Shared>
track<Shared>::track(const std::string &DEBUG_ONLY(class_name))
#ifndef NDEBUG
    : class_(class_name)
#endif
{
#ifndef NDEBUG
#ifndef RESOURCE_INSCREASE
    count_ = ++instances;
    bc::log::trace(LOG_SYSTEM)
        << class_ << "(" << count_ << ")";
#else
    bc::log::trace(LOG_SYSTEM)
        << class_ << "(" << ++instances << ")";
#endif
#endif
}

template <class Shared>
track<Shared>::~track()
{
#ifndef NDEBUG
    bc::log::trace(LOG_SYSTEM)
#ifndef RESOURCE_INSCREASE
        << "~" << class_ << "(" << count_ << ")";
#else
        << "~" << class_ << "(" << --instances << ")";
#endif
#endif
}

#endif
