[![AGPL v3](https://img.shields.io/badge/license-AGPL%20v3-brightgreen.svg)](./LICENSE)
# UC Achitect
![image](https://raw.githubusercontent.com/wiki/yangguanglu/pics/uchainachitect.jpeg)
# UC Project
UC is implemented based on [libbitcoin project](https://github.com/libbitcoin).


# Build UC

## Compiler requirements
| Compilier | Minimum Version |  
| ---------| ---------------- | 
| gcc/g++ |   5.0             |  
| clang++ |   3.4 (8.0.0)     |  

C++ compiler support [C++14](http://en.cppreference.com/w/cpp/compiler_support). 
Using `c++ -v` to check c++ version.
- [Simple guide to upgrade GCC](http://docs.uc.org/helpdoc/upgrade-gcc.html).
- [Upgrade guide for Debian/ubuntuu](https://github.com/libbitcoin/libbitcoin#debianubuntu)
- [Upgrade guide for OSX](https://github.com/libbitcoin/libbitcoin#macintosh)
- [Upgrade guide for windows](https://github.com/libbitcoin/libbitcoin#windows)

Dependencies of UC are **static linked** (including libstdc++). 
Thus, there is no extra dependency after compilation.
Recommends Ubuntu 16.04/CentOS 7.2/Visual Studio 2015 to develop/debug/build UC.

## Toolchain requirements
- cmake 3.0+
- git
- automake (speck256k1/ZeroMQ required)

```bash
$ yum/brew/apt-get install git cmake
$ yum/brew/apt-get install autoconf automake libtool pkg-config
```

## Build UC
```bash
$ git clone https://github.com/UCHAIN-WORLD/UChain.git
$ cd UChain && mkdir build && cd build
$ cmake ..
$ make -j4
$ make install
```
If you do not need UPnP support, you can use `"cmake -DUSE_UPNP=OFF .."` to disable it.

optional:
```bash
$ make test (should install boost_unit_test_framework)
$ make doc  (should install doxygen and graphviz)
```
*Needs to configure Library Dependencies firstly.*

# Library Dependencies

Installing by bash script (sudo required).
```bash
$ sudo ./install_dependencies.sh
```
By default, `./install_dependencies.sh` will install `ZeroMQ` `secp256k1`.  
You can install more by specify arguments, for example:
```bash
# --build-upnpc is needed is you want UPnP supporting.
$ sudo ./install_dependencies.sh --build-boost --build-upnpc
```

## boost 1.60(recommended)
```bash
$ sudo yum/brew/apt-get install libboost-all-dev
```

## ZeroMQ 4.2.5+
Modules server/explorer required.

```bash
$ wget https://github.com/zeromq/libzmq/releases/download/v4.2.5/zeromq-4.2.5.tar.gz
$ tar -xzvf zeromq-4.2.5.tar.gz
$ cd zeromq-4.2.5
$ ./autogen.sh
$ ./configure
$ make -j4
$ sudo make install && sudo ldconfig
```

## secp256k1 
Modules blockchain/database required.

```bash
$ git clone https://github.com/UCHAIN-WORLD/secp256k1
$ cd secp256k1
$ ./autogen.sh
$ ./configure --enable-module-recovery
$ make -j4
$ sudo make install && sudo ldconfig
```

## miniupnpc
Modules blockchain/network with UPnP function required.

```bash
$ wget http://miniupnp.tuxfamily.org/files/miniupnpc-2.0.tar.gz
$ tar -xzvf miniupnpc-2.0.tar.gz
$ cd miniupnpc-2.0
$ make -j4
$ sudo INSTALLPREFIX=/usr/local make install && sudo ldconfig
```

# Run UC
After UC is built successfully, there are two executable files in the _bin_ directory:

 - **ucd** - server program  
   Runs a full UChain node in the global peer-to-peer network.

 - **uc-cli** - client program  
   Sent your request to the server, the server will process it and return response to your client.

Go to _bin_ diretory, and run the program.
More information please reference to [Command line usage](https://github.com/UCHAIN-WORLD/uchain-fullnode/wiki) and [Configuration file](https://github.com/UCHAIN-WORLD/uchain-fullnode/wiki/config-file.md).
```bash
$ cd bin
$ ./ucd
$ ./uc-cli $command $params $options
```
