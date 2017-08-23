set(CATCH_SEARCH_PATHS
	"${PROJECT_SOURCE_DIR}/test/catch"
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)

unset(CATCH_INCLUDE_DIR CACHE)

find_path(CATCH_INCLUDE_DIR
	NAMES catch.hpp
	HINTS ${CATCH_SEARCH_PATHS}
	PATH_SUFFIXES catch/include Catch/include catch/
)

set(CATCH_INCLUDE_DIRS ${CATCH_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	CATCH
	REQUIRED_VARS CATCH_INCLUDE_DIRS
	VERSION_VAR CATCH_VERSION_STRING
)

mark_as_advanced(CATCH_INCLUDE_DIR)

add_library(catch IMPORTED INTERFACE)

set_target_properties(catch PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CATCH_INCLUDE_DIR})
