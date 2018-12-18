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
#include <UChainService/txs/utility/path.hpp>
#include <UChain/bitcoin/unicode/ifstream.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <jsoncpp/json/json.h>
#include <UChainService/api/restful/MongooseCli.hpp>
#include <UChain/bitcoin/unicode/unicode.hpp>

BC_USE_UC_MAIN

/**
 * Invoke this program with the raw arguments provided on the command line.
 * All console input and output streams for the application originate here.
 * @param argc  The number of elements in the argv array.
 * @param argv  The array of arguments, including the process.
 * @return      The numeric result to return via console exit.
 */
using namespace mgbubble::cli;
namespace po = boost::program_options;

void my_impl(const http_message* hm)
{
    auto&& reply = std::string(hm->body.p, hm->body.len);
    Json::Reader reader;
    Json::Value root;
    if (reader.parse(reply, root) && root.isObject()) {
        if (root["error"]["code"].isInt() && root["error"]["code"].asInt() != 0) {
            bc::cout << root["error"].toStyledString();
        }
        else if (root["result"].isString()) {
            bc::cout << root["result"].asString() <<std::endl;
        }
        else if (root["result"].isNumeric()){
            bc::cout << root["result"] <<std::endl;
        }
        else if(root["result"].isArray() || root["result"].isObject()) {
            bc::cout << root["result"].toStyledString();
        }
        else {
            bc::cout << reply << std::endl;
        }
    }
    else {
        bc::cout << reply << std::endl;
    }
}

int bc::main(int argc, char* argv[])
{
    auto cur_path = boost::filesystem::current_path();
    //boost::filesystem::remove(cur_path/"conf"); 
    bc::set_utf8_stdout();
    auto work_path = bc::default_data_path();
    int index = 0;
    if(argc > 2)
    {
        std::string op = argv[1], cfg_path = argv[2];
        boost::trim(op);
        boost::trim(cfg_path);
        if(!op.compare("-c"))
        {
            if(!boost::filesystem::exists(cfg_path))
            {
                log::info("config") << "uc.config path is invalid.";
                return -1;
            }
            try{
                boost::filesystem::remove(cur_path/"conf");
                boost::filesystem::create_symlink(boost::filesystem::path(cfg_path), cur_path/"conf");
                if(argc == 3)
                    return 0;
                else
                    index = 2;
            }catch(...){
                log::info("config") << "Please use administrator privilege to allow uc-cli access! ";
                return -1;
            }
        }
        
    }
    auto&& config_file = boost::filesystem::exists(cur_path / "conf") ? cur_path / "conf" : work_path / "uc.conf";
    std::string url{"127.0.0.1:8707/rpc/v3"};

    if (boost::filesystem::exists(config_file)) {
        const auto& path = config_file.string();
        bc::ifstream file(path);

        if (!file.good()) {
            BOOST_THROW_EXCEPTION(po::reading_file(path.c_str()));
        }

        std::string tmp;
        po::options_description desc("");
        desc.add_options()
            ("server.mongoose_listen", po::value<std::string>(&tmp)->default_value("127.0.0.1:8707"));

        po::variables_map vm;
        po::store(po::parse_config_file(file, desc, true), vm);
        po::notify(vm);

        if (vm.count("server.mongoose_listen")) {
            if (!tmp.empty()) {
                // On Windows, client can not connect to 0.0.0.0
                if (tmp.find("0.0.0.0") == 0) {
                    tmp.replace(0, 7, "127.0.0.1");
                }
                url = tmp + "/rpc/v3";
            }
        }
    }

    // HTTP request call commands
    HttpReq req(url, 3000, reply_handler(my_impl));

    
    Json::Value jsonvar;
    Json::Value jsonopt;
    jsonvar["jsonrpc"] = "3.0";
    jsonvar["id"] = 1;
    jsonvar["method"] = (argc < 2) ? "help" : argv[1 +index];
    jsonvar["params"] = Json::arrayValue;

    if (argc > 2+index)
    {
        for (int i = 2+index; i < argc; i++)
        {
            jsonvar["params"].append(argv[i]);
        }
    }

    req.post(jsonvar.toStyledString());
    return 0;
}
