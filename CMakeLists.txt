# ----------------- Project Title ----------------
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(UChain)

# ----------------- Project Build Type ----------------
SET(CMAKE_VERBOSE_MAKEFILE 1)
SET(ENABLE_SHARED_LIBS OFF CACHE BOOL   "Enable shared libs.")
SET(MG_ENABLE_DEBUG    OFF CACHE BOOL   "Enable Mongoose debug.")

IF(NOT CMAKE_BUILD_TYPE)
    #SET(CMAKE_BUILD_TYPE DEBUG)
    SET(CMAKE_BUILD_TYPE RELEASE)
    #SET(CMAKE_BUILD_TYPE RELWITHDEBINFO)
    #SET(CMAKE_BUILD_TYPE MINSIZEREL)
ENDIF()
STRING(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)

# Enable for use with clang-tidy.
IF(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
    SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
ENDIF()

# --------------- Check os verson---------------------
if ( NOT WINDOWS )	

	if (${CMAKE_SYSTEM_PROCESSOR} MATCHES i386|i586|i686)
		set ( BIT_MODE "32")
	else ()
		set ( BIT_MODE "64")
	endif ()


	if(EXISTS "/etc/debian_version")
	   set ( PLATFORM "Debian")
	endif(EXISTS "/etc/debian_version")

	if(EXISTS "/etc/redhat-release")
	   set ( PLATFORM "Redhat")
	endif(EXISTS "/etc/redhat-release")

	string (REGEX MATCH "Linux-([0-9]*)" _ ${CMAKE_SYSTEM})
    set(os_version_suffix ${CMAKE_MATCH_1})
    message("-- os_version_suffix:      ${os_version_suffix}")

endif ( NOT WINDOWS )	

# --------------- Compiler Settings ------------------
# Common Definitions 

IF(${CMAKE_CXX_COMPILER} MATCHES .*android.*)
    SET(ANDROID 1)
ENDIF()

SET(COMMON_WARN "-Wall -Wextra -Wstrict-aliasing=2 -Wno-unused-parameter -Wno-unused-variable -Wno-type-limits")
#if (NOT ANDROID) 
#		SET(COMMON_WARN "${COMMON_WARN} -Werror")
#endif()
SET(COMMON_FLAGS "-fstrict-aliasing -fvisibility=hidden")
SET(COMMON_ARGS "${COMMON_FLAGS} ${COMMON_WARN}")

IF (APPLE)
    ADD_DEFINITIONS(-DMAC_OSX=1)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
ELSE()
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
ENDIF()

# GXX/Clang settings
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 ${COMMON_ARGS}")
    #libbitcoin has too many ignored-qualifiers, and TODOs
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 ${COMMON_ARGS} -pthread -fno-enforce-eh-specs -fnothrow-opt -Wno-reorder -Wno-ignored-qualifiers -Wno-unused-function -Wno-unused-but-set-variable -Wno-sign-compare -Wno-unused-but-set-parameter -Wno-implicit-fallthrough")

ELSEIF("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    SET(CMAKE_C_FLAGS "-std=c11 ${COMMON_ARGS}")
    SET(CMAKE_CXX_FLAGS "-std=c++14 ${COMMON_ARGS} -Wno-reorder -Wno-ignored-qualifiers -Wno-inconsistent-missing-override -Wno-missing-braces -Wno-mismatched-tags -Wno-overloaded-virtual -Wno-sometimes-uninitialized -Wno-macro-redefined -Wno-uninitialized -Wno-unused-private-field -Wno-unused-function -Wno-implicit-fallthrough")
ENDIF()

if(MINGW OR MSYS)
	set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -lwsock32 -lws2_32 -lgmp -static")
	set(CMAKE_C_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES} -lwsock32 -lws2_32 -lgmp -static")
ENDIF()
	
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC")

# --------------- Macro Definitions ------------------
#ADD_DEFINITIONS(-DBOOST_NO_AUTO_PTR=1 -DBOOST_NO_RTTI=1 -DBOOST_NO_TYPEID=1)
IF(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    ADD_DEFINITIONS(-DUC_DEBUG=1)
ENDIF()

# --------------- Outputs ---------------------
SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")
SET(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")

FILE(MAKE_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}")
FILE(MAKE_DIRECTORY "${LIBRARY_OUTPUT_PATH}")
FILE(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/share/doc/html")

SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/etc")
SET(CMAKE_INSTALL_PREFIX "/usr/local/")

# --------------- Libraries Dependencies ---------------------
IF(ENABLE_SHARED_LIBS)
    SET(Boost_USE_STATIC_LIBS   OFF)
ELSE()
    SET(Boost_USE_STATIC_LIBS   ON)
ENDIF()

# use UPNP
SET(USE_UPNP ON CACHE BOOL "Use UPNP.")
IF(USE_UPNP)
    ADD_DEFINITIONS(-DUSE_UPNP=1)
    FIND_PACKAGE(miniupnpc REQUIRED)
    INCLUDE_DIRECTORIES("${miniupnpc_INCLUDE_DIRS}")
    IF(ENABLE_SHARED_LIBS)
        SET(miniupnpc_LIBRARY miniupnpc_shared)
    ELSE()
        SET(miniupnpc_LIBRARY miniupnpc_static)
    ENDIF()
ENDIF()

FIND_PACKAGE(Boost 1.56 REQUIRED COMPONENTS date_time filesystem system
program_options regex thread)
#set(Boost_LIBRARIES ${Boost_LIBRARIES} icui18n icuuc icudata pthread dl)

FIND_PACKAGE(secp256k1 REQUIRED)
FIND_PACKAGE(ZeroMQ 4.2.0 REQUIRED)

# ---------------- Doxygen --------------------
FIND_PROGRAM(ENV_EXECUTABLE env QUIET)
FIND_PROGRAM(DOT_EXECUTABLE dot QUIET)
FIND_PACKAGE(Doxygen QUIET)          # Optional.

IF(DOXYGEN_FOUND)
  CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/Doxyfile.in"
    "${PROJECT_BINARY_DIR}/Doxyfile")

  ADD_CUSTOM_TARGET(doc
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${PROJECT_BINARY_DIR}/share"
    COMMAND "${DOXYGEN_EXECUTABLE}" "${PROJECT_BINARY_DIR}/Doxyfile"
    SOURCES "${PROJECT_BINARY_DIR}/Doxyfile")

  ADD_CUSTOM_COMMAND(TARGET doc POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "${PROJECT_SOURCE_DIR}/doc/CNAME"
            "${PROJECT_BINARY_DIR}/share/doc/html/")

  INSTALL(DIRECTORY "${PROJECT_BINARY_DIR}/share/doc/" DESTINATION share/doc)
ENDIF()

# ------------------ UC Includes --------------------
INCLUDE_DIRECTORIES("${Boost_INCLUDE_DIRS}")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/thirdparty")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include")
INCLUDE_DIRECTORIES("${secp256k1_INCLUDE_DIRS}")
INCLUDE_DIRECTORIES("${ZeroMQ_INCLUDE_DIRS}")

# ------------------ UC Libraries --------------------
IF(ENABLE_SHARED_LIBS)
    SET(mongoose_LIBRARY mongoose_shared)
    SET(jsoncpp_LIBRARY jsoncpp_shared)
    SET(cryptojs_LIBRARY cryptojs_shared)
    SET(bitcoin_LIBRARY bitcoin_shared)
    SET(txs_LIBRARY txs_shared)
    SET(bitcoinmath_LIBRARY bitcoinmath_shared)
    SET(network_LIBRARY network_shared)
    SET(consensus_LIBRARY consensus_shared)
    SET(database_LIBRARY database_shared)
    SET(data_LIBRARY data_shared)
    SET(node_LIBRARY node_shared)
    SET(protocol_LIBRARY protocol_shared)
    SET(client_LIBRARY client_shared)
    SET(explorer_LIBRARY explorer_shared)
    SET(api_LIBRARY api_shared)
ELSE()
    SET(mongoose_LIBRARY mongoose_static)
    SET(jsoncpp_LIBRARY jsoncpp_static)
    SET(cryptojs_LIBRARY cryptojs_static)
    SET(bitcoin_LIBRARY bitcoin_static)
    SET(txs_LIBRARY txs_static)
    SET(bitcoinmath_LIBRARY bitcoinmath_static)
    SET(network_LIBRARY network_static)
    SET(consensus_LIBRARY consensus_static)
    SET(database_LIBRARY database_static)
    SET(data_LIBRARY data_static)
    SET(blockchain_LIBRARY blockchain_static)
    SET(node_LIBRARY node_static)
    SET(protocol_LIBRARY protocol_static)
    SET(client_LIBRARY client_static)
    SET(explorer_LIBRARY explorer_static)
    SET(api_LIBRARY api_static)
ENDIF()

ENABLE_TESTING()

# ------------------ UC src --------------------
ADD_SUBDIRECTORY(etc)
ADD_SUBDIRECTORY(thirdparty)
ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(src)
#ADD_SUBDIRECTORY(test)
