cmake_minimum_required(VERSION 3.0)
project(kangaru)
set(KANGARU_VERSION "3.0.0")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory(examples)

set(KANGARU_INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/kangaru")
install(DIRECTORY include/kangaru DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING
	PATTERN "*.hpp"
	PATTERN "*.h"
	PATTERN "build" EXCLUDE
	PATTERN ".*" EXCLUDE
	PATTERN "*~" EXCLUDE
	PATTERN "build/" EXCLUDE
	PATTERN ".svn/" EXCLUDE
	PATTERN ".git/" EXCLUDE
)

# the following is inspired from the glm CMakeLists.txt
# You can find it here: https://github.com/g-truc/glm/blob/master/CMakeLists.txt

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/kangaruVersion.cmake"
    VERSION ${KANGARU_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# build tree package config
configure_file(
    cmake/kangaruBuildConfig.cmake.in
    kangaruConfig.cmake
    @ONLY
)

# install tree package config
configure_package_config_file(
    cmake/kangaruConfig.cmake.in
    ${KANGARU_INSTALL_CONFIGDIR}/kangaruConfig.cmake
    INSTALL_DESTINATION ${KANGARU_INSTALL_CONFIGDIR}
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(
	FILES 
		"${CMAKE_CURRENT_BINARY_DIR}/${KANGARU_INSTALL_CONFIGDIR}/kangaruConfig.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/kangaruVersion.cmake"
	DESTINATION ${KANGARU_INSTALL_CONFIGDIR}
)

add_library(kangaru INTERFACE)
target_include_directories(kangaru INTERFACE
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
install(TARGETS kangaru EXPORT kangaruTargets)

export(
	EXPORT kangaruTargets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/kangaruTargets.cmake"
)

install(
	EXPORT kangaruTargets FILE kangaruTargets.cmake
	DESTINATION ${KANGARU_INSTALL_CONFIGDIR}
)

export(PACKAGE kangaru)
