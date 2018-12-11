FILE(GLOB_RECURSE explorer_SOURCES "*.cpp")

ADD_DEFINITIONS(-DBCX_STATIC=1)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

if(MINGW OR MSYS)
	FIND_PACKAGE(iphlpapi REQUIRED)
	ADD_LIBRARY(iphlpapi_static STATIC IMPORTED)                                         
	SET_TARGET_PROPERTIES(iphlpapi_static PROPERTIES IMPORTED_LOCATION ${Iphlpapi_ROOT_DIR}/lib/libiphlpapi.a)
ENDIF()

ADD_LIBRARY(zmq_static STATIC IMPORTED)                                         
SET_TARGET_PROPERTIES(zmq_static PROPERTIES IMPORTED_LOCATION ${ZeroMQ_ROOT_DIR}/lib/libzmq.a)

ADD_LIBRARY(explorer_static STATIC ${explorer_SOURCES})
SET_TARGET_PROPERTIES(explorer_static PROPERTIES OUTPUT_NAME uc_explorer)
if(MINGW OR MSYS)
    TARGET_LINK_LIBRARIES(explorer_static iphlpapi_static)
ENDIF()
TARGET_LINK_LIBRARIES(explorer_static zmq_static ${Boost_LIBRARIES}
    ${txs_LIBRARY} ${api_LIBRARY} ${bitcoin_LIBRARY} ${network_LIBRARY} ${protocol_LIBRARY} ${client_LIBRARY}
    ${blockchain_LIBRARY} ${jsoncpp_LIBRARY})
INSTALL(TARGETS explorer_static DESTINATION lib)

IF(ENABLE_SHARED_LIBS)
    ADD_DEFINITIONS(-DBCX_DLL=1)
  ADD_LIBRARY(explorer_shared SHARED ${explorer_SOURCES})
  SET_TARGET_PROPERTIES(explorer_shared PROPERTIES OUTPUT_NAME uc_explorer)
  TARGET_LINK_LIBRARIES(explorer_shared ${ZeroMQ_LIBRARIES} ${Boost_LIBRARIES} 
    ${txs_LIBRARY} ${api_LIBRARY} ${bitcoin_LIBRARY} ${network_LIBRARY} ${protocol_LIBRARY} ${client_LIBRARY} 
    ${blockchain_LIBRARY} ${jsoncpp_LIBRARY})
  INSTALL(TARGETS explorer_shared DESTINATION lib)
ENDIF()
