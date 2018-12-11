find_path(iphlpapi_ROOT_DIR
    NAMES include/iphlpapi.h
	)
find_library(iphlpapi_LIBRARIES
    NAMES iphlpapi libiphlpapi
    HINTS ${iphlpapi_ROOT_DIR}/lib)

find_path(iphlpapi_INCLUDE_DIRS
	NAMES iphlpapi.h
	HINTS ${iphlpapi_ROOT_DIR}/include
)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(iphlpapi DEFAULT_MSG
	iphlpapi_LIBRARIES
	iphlpapi_INCLUDE_DIRS
)
mark_as_advanced(
	iphlpapi_ROOT_DIR
	iphlpapi_LIBRARIES
	iphlpapi_INCLUDE_DIRS
)
