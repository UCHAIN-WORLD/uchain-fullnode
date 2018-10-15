
FILE(GLOB_RECURSE uc-cli_SOURCES "*.cpp")

ADD_DEFINITIONS(-DBCX_STATIC=1)

ADD_EXECUTABLE(uc-cli ${uc-cli_SOURCES})

# for experimental/string_view
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

IF(ENABLE_SHARED_LIBS)
    TARGET_LINK_LIBRARIES(uc-cli ${mongoose_LIBRARY} ${jsoncpp_LIBRARY}
        ${txs_LIBRARY} ${bitcoin_LIBRARY} ${explorer_LIBRARY})
ELSE()
    TARGET_LINK_LIBRARIES(uc-cli ${mongoose_LIBRARY} ${jsoncpp_LIBRARY}
        ${txs_LIBRARY} ${bitcoin_LIBRARY} ${explorer_LIBRARY})
ENDIF()

INSTALL(TARGETS uc-cli DESTINATION bin)
