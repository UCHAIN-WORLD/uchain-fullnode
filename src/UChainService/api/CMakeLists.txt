FILE(GLOB_RECURSE api_SOURCES "*.cpp")

ADD_DEFINITIONS(-DBCX_STATIC=1)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

ADD_LIBRARY(api_static STATIC ${api_SOURCES})
SET_TARGET_PROPERTIES(api_static PROPERTIES OUTPUT_NAME uc_api)
TARGET_LINK_LIBRARIES(api_static zmq_static ${Boost_LIBRARIES}
    ${txs_LIBRARY} ${explorer_LIBRARY} ${bitcoin_LIBRARY} ${network_LIBRARY} ${protocol_LIBRARY} ${client_LIBRARY}
    ${blockchain_LIBRARY} ${jsoncpp_LIBRARY} ${cryptojs_LIBRARY})
INSTALL(TARGETS api_static DESTINATION lib)

IF(ENABLE_SHARED_LIBS)
    ADD_DEFINITIONS(-DBCX_DLL=1)
  ADD_LIBRARY(api_shared SHARED ${api_SOURCES})
  SET_TARGET_PROPERTIES(api_shared PROPERTIES OUTPUT_NAME uc_api)
  TARGET_LINK_LIBRARIES(api_shared ${ZeroMQ_LIBRARIES} ${Boost_LIBRARIES} 
    ${txs_LIBRARY} ${explorer_LIBRARY} ${bitcoin_LIBRARY} ${network_LIBRARY} ${protocol_LIBRARY} ${client_LIBRARY} 
    ${blockchain_LIBRARY} ${jsoncpp_LIBRARY} ${cryptojs_LIBRARY})
  INSTALL(TARGETS api_shared DESTINATION lib)
ENDIF()
