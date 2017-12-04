cmake_minimum_required(VERSION 3.0)
project(kangaru)

set(KANGARU_VERSION_MAJOR "3")
set(KANGARU_VERSION_MINOR "4")
set(KANGARU_VERSION_PATCH "0")
set(KANGARU_VERSION "${KANGARU_VERSION_MAJOR}.${KANGARU_VERSION_MINOR}.${KANGARU_VERSION_PATCH}")

option(KANGARU_BUILD_EXAMPLES "Build kangaru examples" false)
option(KANGARU_INSTALL "Generate kangaru installation target" true)
option(KANGARU_EXPORT "Export kangaru source tree as package" true)

set(KANGARU_INSTALL_INCLUDE_DIR "include")

add_library(kangaru INTERFACE)

target_include_directories(kangaru INTERFACE
	$<INSTALL_INTERFACE:${KANGARU_INSTALL_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if (KANGARU_BUILD_EXAMPLES)
	add_subdirectory(examples)
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
		cmake/kangaruBuildConfig.cmake.in
		kangaruConfig.cmake
		@ONLY
	)

	# install tree package config
	configure_package_config_file(
		cmake/kangaruConfig.cmake.in
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
