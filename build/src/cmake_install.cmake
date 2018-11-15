# Install script for directory: /Users/cary/Projects/uchain-fullnode/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local/")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "RELEASE")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/cary/Projects/uchain-fullnode/build/src/UChain/bitcoin/cmake_install.cmake")
  include("/Users/cary/Projects/uchain-fullnode/build/src/UChain/network/cmake_install.cmake")
  include("/Users/cary/Projects/uchain-fullnode/build/src/UChain/database/cmake_install.cmake")
  include("/Users/cary/Projects/uchain-fullnode/build/src/UChain/blockchain/cmake_install.cmake")
  include("/Users/cary/Projects/uchain-fullnode/build/src/UChain/node/cmake_install.cmake")
  include("/Users/cary/Projects/uchain-fullnode/build/src/UChain/protocol/cmake_install.cmake")
  include("/Users/cary/Projects/uchain-fullnode/build/src/UChain/client/cmake_install.cmake")
  include("/Users/cary/Projects/uchain-fullnode/build/src/UChain/explorer/cmake_install.cmake")
  include("/Users/cary/Projects/uchain-fullnode/build/src/UChainService/consensus/cmake_install.cmake")
  include("/Users/cary/Projects/uchain-fullnode/build/src/UChainService/api/cmake_install.cmake")
  include("/Users/cary/Projects/uchain-fullnode/build/src/UChainService/txs/cmake_install.cmake")
  include("/Users/cary/Projects/uchain-fullnode/build/src/UChainService/data/cmake_install.cmake")
  include("/Users/cary/Projects/uchain-fullnode/build/src/UChainApp/ucd/cmake_install.cmake")
  include("/Users/cary/Projects/uchain-fullnode/build/src/UChainApp/uc-cli/cmake_install.cmake")

endif()

