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
#ifndef UC_CONFIG_DIRECTORY_HPP
#define UC_CONFIG_DIRECTORY_HPP

#include <string>
#include <boost/filesystem.hpp>
#include <UChain/coin/define.hpp>

namespace libbitcoin
{
namespace config
{

// Declare config_default_path() via BC_DECLARE_CONFIG_DEFAULT_PATH(relative).
#define CONFIG_DEFAULT_PATH(directory, subdirectory)     \
    static boost::filesystem::path config_default_path() \
    {                                                    \
        const boost::filesystem::path folder(directory); \
        return folder / subdirectory;                    \
    }

// The SYSCONFDIR symbol must be defined at compile for the project.
// Therefore this must be compiled directly into the relevant project(s).
#ifdef _MSC_VER
#define BC_DECLARE_CONFIG_DEFAULT_PATH(relative) \
    CONFIG_DEFAULT_PATH(bc::config::windows_config_directory(), relative)
#else
#define SYSCONFDIR "conf"
#define BC_DECLARE_CONFIG_DEFAULT_PATH(relative) \
    CONFIG_DEFAULT_PATH(SYSCONFDIR, relative)
#endif

/**
 * Get the windows configuration directory.
 * @return Path or empty string if unable to retrieve.
 */
BC_API std::string windows_config_directory();

} // namespace config
} // namespace libbitcoin

#endif
