/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-node.
 *
 * UChain-node is free software: you can redistribute it and/or
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
#ifndef UC_NODE_CONFIGURATION_HPP
#define UC_NODE_CONFIGURATION_HPP

#include <cstddef>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <UChain/blockchain.hpp>
#include <UChain/network.hpp>
#include <UChain/node/define.hpp>
#include <UChain/node/settings.hpp>

namespace libbitcoin
{
namespace node
{

// Not localizable.
#define BN_HELP_VARIABLE "help"
#define BN_SETTINGS_VARIABLE "settings"
#define BN_VERSION_VARIABLE "version"
#define BN_DAEMON_VARIABLE "daemon"
//#define BS_UI_VARIABLE "ui"

// This must be lower case but the env var part can be any case.
#define BN_CONFIG_VARIABLE "config"

// This must match the case of the env var.
#define BN_ENVIRONMENT_VARIABLE_PREFIX "BN_"

/// Full node configuration, thread safe.
class BCN_API configuration
{
  public:
    configuration(bc::settings context);
    configuration(const configuration &other);

    /// Options.
    bool help;
    bool initchain;
    bool settings;
    bool version;
    bool daemon;
    bool use_testnet_rules;
    bool ui;
    bool upnp_map_port;

    /// Options and environment vars.
    boost::filesystem::path file;
    boost::filesystem::path data_dir;

    /// Settings.
    node::settings node;
    blockchain::settings chain;
    database::settings database;
    network::settings network;
};

} // namespace node
} // namespace libbitcoin

#endif
