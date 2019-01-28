[![AGPL v3](https://img.shields.io/badge/license-AGPL%20v3-brightgreen.svg)](./LICENSE)
# UC Project
UChain is the first public infrastructure blockchain specifically designed for the global sharing economy. Along with other sharing economy enterprises, UChain aims to build its underlying blockchain network to solve the current problems of trust and data abuse. Exercising API's and SDK's provided by UChain , all sharing economy enterprises are able to issue their own token and build their application on top of the UChain network, together making UChain a better global autonomous sharing economy ecosystem.
<br>UC is implemented based on [libbitcoin project](https://github.com/libbitcoin) and [ICE project](https://github.com/zeroc-ice/ice).
# UC Achitect
![image](https://raw.githubusercontent.com/wiki/yangguanglu/pics/uchainachitect.jpeg)

# Build UC

## Compiler requirements
| Compilier | Minimum Version |  
| ---------| ---------------- | 
| gcc/g++ |   5.0             |  
| clang++ |   3.4 (8.0.0)     |  

C++ compiler support [C++14](http://en.cppreference.com/w/cpp/compiler_support). 
Using `c++ -v` to check c++ version.
- [Upgrade guide for Debian/ubuntuu](https://github.com/libbitcoin/libbitcoin#debianubuntu)
- [Upgrade guide for OSX](https://github.com/libbitcoin/libbitcoin#macintosh)
- [Upgrade guide for windows](https://github.com/libbitcoin/libbitcoin#windows)

Dependencies of UC are **static linked** (including libstdc++). 
Thus, there is no extra dependency after compilation.
Recommends Ubuntu 16.04/CentOS 7.2/MinGW to develop/debug/build UC.

## Toolchain requirements
- cmake 3.0+
- git
- automake (speck256k1/ZeroMQ required)
- make ([MinGW for Windows](http://repo.msys2.org/distrib/x86_64/msys2-x86_64-20180531.exe) is supported)

```bash
$ yum/brew/apt-get install git cmake
$ yum/brew/apt-get install autoconf automake libtool pkg-config
```

# Library Dependencies
*Needs to configure Library Dependencies firstly.*
Installing by bash script (sudo required).
```bash
$ chmod +x ./install_thirdlibrary
$ sudo ./install_thirdlibrary
```
By default, `./install_thirdlibrary` will install `ZeroMQ` `secp256k1`.  
You can install more by specify arguments, for example:
```bash
# --build-upnpc is needed is you want UPnP supporting.
$ sudo ./install_thirdlibrary --build-boost --build-upnpc
```

## boost 1.60(boost is required and 1.60 is recommended)
```bash
$ sudo yum/brew/apt-get install libboost-all-dev
```

## ZeroMQ 4.2.5+(required)
Modules server/explorer required.

```bash
$ wget https://github.com/zeromq/libzmq/releases/download/v4.2.5/zeromq-4.2.5.tar.gz
$ tar -xzvf zeromq-4.2.5.tar.gz
$ cd zeromq-4.2.5
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install && sudo ldconfig
```

## secp256k1(required) 
Modules blockchain/database required.

```bash
$ git clone https://github.com/UCHAIN-WORLD/secp256k1
$ cd secp256k1
$ ./autogen.sh
$ ./configure --enable-module-recovery
$ make
$ sudo make install && sudo ldconfig
```

## miniupnpc(if needed)
Modules blockchain/network with UPnP function required.

```bash
$ wget http://miniupnp.tuxfamily.org/files/miniupnpc-2.0.tar.gz
$ tar -xzvf miniupnpc-2.0.tar.gz
$ cd miniupnpc-2.0
$ make
$ sudo INSTALLPREFIX=/usr/local make install && sudo ldconfig
```

## Build UC
```bash
$ git clone https://github.com/UCHAIN-WORLD/uchain-fullnode.git
$ cd UChain && mkdir build && cd build
$ cmake ..
$ make
$ make install
```
If you do not need UPnP support, you can use `"cmake -DUSE_UPNP=OFF .."` to disable it.
<br>And `"make -j4`" may be better (-j4 is not always the rigth parameter... could be j2 or j8 it depends by the cpu).
<br>Also `"make install-strip`" may be better(it strips).


# Run UC
After UC is built successfully, there are two executable files in the _bin_ directory:

 - **ucd** - server program  
   Runs a full UChain node in the global peer-to-peer network.

 - **uc-cli** - client program  
   Sent your request to the server, the server will process it and return response to your client.

Go to _bin_ diretory, and run the program.
More information please reference to [Command line usage]( https://github.com/UCHAIN-WORLD/uchain-fullnode/wiki/commands).
```bash
$ cd bin
$ ./ucd
$ ./uc-cli $command $params $options
```
