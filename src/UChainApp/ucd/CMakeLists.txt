FILE(GLOB_RECURSE ucd_SOURCES "*.cpp")

ADD_EXECUTABLE(ucd ${ucd_SOURCES})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

IF(ENABLE_SHARED_LIBS)
    ADD_DEFINITIONS(-DBCS_DLL=1)
    TARGET_LINK_LIBRARIES(ucd ${Boost_LIBRARIES} ${network_LIBRARY} ${data_LIBRARY} ${database_LIBRARY} ${consensus_LIBRARY}
    ${blockchain_LIBRARY} ${txs_LIBRARY} ${bitcoin_LIBRARY} ${mongoose_LIBRARY} ${node_LIBRARY}
    ${protocol_LIBRARY} ${client_LIBRARY} ${api_LIBRARY} ${explorer_LIBRARY} ${cryptojs_LIBRARY} )
ELSE()
    ADD_DEFINITIONS(-DBCS_STATIC=1)
    TARGET_LINK_LIBRARIES(ucd ${Boost_LIBRARIES} ${network_LIBRARY} ${data_LIBRARY} ${database_LIBRARY} ${consensus_LIBRARY}
    ${blockchain_LIBRARY} ${txs_LIBRARY} ${bitcoin_LIBRARY} ${mongoose_LIBRARY} ${node_LIBRARY}
    ${protocol_LIBRARY} ${client_LIBRARY} ${api_LIBRARY} ${explorer_LIBRARY} ${cryptojs_LIBRARY})
ENDIF()
if(MINGW OR MSYS)
    FIND_PACKAGE(Boost 1.56 REQUIRED random)
	TARGET_LINK_LIBRARIES(ucd ${Boost_RANDOM_LIBRARIES})
ENDIF()
INSTALL(TARGETS ucd DESTINATION bin)
