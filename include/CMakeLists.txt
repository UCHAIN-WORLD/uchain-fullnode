INSTALL(DIRECTORY "${PROJECT_SOURCE_DIR}/include/UChain"
                  DESTINATION include
                  FILES_MATCHING
                  PATTERN "*.h"
                  PATTERN "*.hpp")
INSTALL(DIRECTORY "${PROJECT_SOURCE_DIR}/thirdparty/mongoose"
                  DESTINATION include/UChain
                  FILES_MATCHING
                  PATTERN "*.h"
                  PATTERN "*.hpp")
INSTALL(DIRECTORY "${PROJECT_SOURCE_DIR}/thirdparty/json"
                  DESTINATION include/UChain
                  FILES_MATCHING
                  PATTERN "*.h"
                  PATTERN "*.hpp")
