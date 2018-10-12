/**
 * Copyright (c) 2018-2020 UChain core developers (see UC-AUTHORS)
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
#include <UChain/bitcoin/utility/path.hpp>
#include <boost/thread/once.hpp>

#ifdef _WIN32
#include <Shlobj.h>
#endif

namespace libbitcoin{

const boost::filesystem::path& default_data_path()
{
    static boost::filesystem::path default_path("");
    static boost::once_flag once = BOOST_ONCE_INIT;
    auto path_init = []() {
        namespace fs = boost::filesystem;
        // Windows < Vista: C:\Documents and Settings\Username\Application Data\UChain
        // Windows >= Vista: C:\Users\Username\AppData\Roaming\UChain
        // Mac: ~/Library/Application Support/UChain
        // Unix: ~/.UChain
#ifdef _WIN32
        // Windows
#ifdef UNICODE
        wchar_t file_path[MAX_PATH] = { 0 };
#else
        char file_path[MAX_PATH] = { 0 };
#endif
        SHGetSpecialFolderPath(NULL, file_path, CSIDL_APPDATA, true);
        fs::path pathRet = boost::filesystem::path(file_path) / "UChain";
        fs::create_directories(pathRet);
        default_path = pathRet;
#else
        fs::path pathRet;
        char* pszHome = getenv("HOME");
        if (pszHome == nullptr || strlen(pszHome) == 0)
            pathRet = fs::path("/");
        else
            pathRet = fs::path(pszHome);
#ifdef MAC_OSX
        // Mac
        pathRet /= "Library/Application Support";
        fs::create_directories(pathRet / "UChain");
        default_path = pathRet / "UChain";
#else
        // Unix
        fs::create_directories(pathRet / ".UChain");
        default_path = pathRet / ".UChain";
#endif
#endif
    };
    boost::call_once(path_init, once);
    return default_path;
}


boost::filesystem::path webpage_path()
{
#ifdef _MSC_VER
#ifdef UNICODE
    wchar_t tmp[MAX_PATH * 2] = { 0 };
#else
    char tmp[MAX_PATH * 2] = { 0 };
#endif
    GetModuleFileName(NULL, tmp, MAX_PATH * 2);
    return boost::filesystem::path(tmp).parent_path() / "uc-htmls";
#else
    return default_data_path() / "uc-htmls";
#endif
}

}//namespace libbitcoin


