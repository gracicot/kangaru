cmake_minimum_required(VERSION 3.0)
project(kangaru)

set(CMAKE_CXX_STANDARD 11)

set(KANGARU_VERSION_MAJOR "4")
set(KANGARU_VERSION_MINOR "1")
set(KANGARU_VERSION_PATCH "0")

set(KANGARU_VERSION "${KANGARU_VERSION_MAJOR}.${KANGARU_VERSION_MINOR}.${KANGARU_VERSION_PATCH}")

option(KANGARU_BUILD_EXAMPLES "Build kangaru examples" false)
option(KANGARU_INSTALL "Generate kangaru installation target" true)
option(KANGARU_EXPORT "Export kangaru source tree as package" true)
option(KANGARU_TEST "Build test binaries" false)
option(KANGARU_TEST_CXX14 "Build C++14 test binaries" false)
option(KANGARU_TEST_CXX17 "Build C++17 test binaries" false)

if(${KANGARU_TEST_CXX14} OR ${KANGARU_TEST})
	set(KANGARU_ENABLE_TESTS true)
endif()

set(KANGARU_INSTALL_INCLUDE_DIR "include")

add_library(kangaru INTERFACE)

target_include_directories(kangaru INTERFACE
	$<INSTALL_INTERFACE:${KANGARU_INSTALL_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

if (KANGARU_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

if (KANGARU_ENABLE_TESTS)
	enable_testing()
    add_subdirectory(test)
endif()

if (KANGARU_EXPORT)
	export(PACKAGE kangaru)
endif()

if (KANGARU_INSTALL)
	set(KANGARU_CONFIG_PATH "lib/cmake/kangaru")
	
	include(CMakePackageConfigHelpers)
	
	write_basic_package_version_file(
		"${CMAKE_CURRENT_BINARY_DIR}/kangaruConfigVersion.cmake"
		VERSION ${KANGARU_VERSION}
		COMPATIBILITY SameMajorVersion
	)

	# build tree package config
	configure_file(
		cmake/config/kangaruBuildConfig.cmake.in
		kangaruConfig.cmake
		@ONLY
	)

	# install tree package config
	configure_package_config_file(
		cmake/config/kangaruConfig.cmake.in
		${KANGARU_CONFIG_PATH}/kangaruConfig.cmake
		INSTALL_DESTINATION ${KANGARU_CONFIG_PATH}
		NO_CHECK_REQUIRED_COMPONENTS_MACRO
	)

	install(DIRECTORY include/kangaru DESTINATION ${KANGARU_INSTALL_INCLUDE_DIR} FILES_MATCHING PATTERN "*.hpp")
	
	install(
		FILES 
			"${CMAKE_CURRENT_BINARY_DIR}/${KANGARU_CONFIG_PATH}/kangaruConfig.cmake"
			"${CMAKE_CURRENT_BINARY_DIR}/kangaruConfigVersion.cmake"
		DESTINATION ${KANGARU_CONFIG_PATH}
	)
	
	install(TARGETS kangaru EXPORT kangaruTargets)

	export(
		EXPORT kangaruTargets
		FILE "${CMAKE_CURRENT_BINARY_DIR}/kangaruTargets.cmake"
	)

	install(
		EXPORT kangaruTargets FILE kangaruTargets.cmake
		DESTINATION ${KANGARU_CONFIG_PATH}
	)
endif()
