cmake_minimum_required(VERSION 3.29)

project(kangaru VERSION 5.0.0 LANGUAGES CXX)

option(KANGARU_BUILD_EXAMPLES "Build kangaru examples" false)
option(KANGARU_REVERSE_DESTRUCTION "Reverse destruction order in the container mimicking a stack" ON)
option(KANGARU_HASH_TYPE_ID "Generate the type IDs using the hashed type names" ON)
option(KANGARU_NO_EXCEPTION "Disable exceptions by replacing throws by asserts" false)
option(KANGARU_TEST "Build test binaries" ON)

set(KGR_KANGARU_NOEXCEPTION ${KANGARU_NO_EXCEPTION})
set(KGR_KANGARU_REVERSE_DESTRUCTION ${KANGARU_REVERSE_DESTRUCTION})
set(KGR_KANGARU_HASH_TYPE_ID ${KANGARU_HASH_TYPE_ID})

if(NOT KGR_KANGARU_REVERSE_DESTRUCTION)
	message(WARNING
		"The recommended behavior is to enable reverse destruction. This option will the the "
		"default behavior for kangaru 5.x.y. Not enabling this option is considered deprecated."
	)
endif()

configure_file(
	cmake/config/config.hpp.in
	${CMAKE_CURRENT_BINARY_DIR}/generated/include/kangaru/detail/config.hpp
	@ONLY
)

if(KANGARU_TEST_CXX14 OR KANGARU_TEST_CXX17 OR KANGARU_TEST)
	set(KANGARU_ENABLE_TESTS true)
endif()


add_library(kangaru INTERFACE)
add_library(kangaru::kangaru ALIAS kangaru)

target_compile_features(kangaru INTERFACE cxx_std_20)
target_include_directories(kangaru INTERFACE
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated/include>
)

if(KANGARU_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

if(KANGARU_ENABLE_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(KANGARU_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/kangaru")
mark_as_advanced(KANGARU_INSTALL_CMAKEDIR)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/kangaru-config-version.cmake"
	COMPATIBILITY SameMajorVersion
	ARCH_INDEPENDENT
)

# build tree package config
configure_file(
	cmake/config/kangaru-config.cmake.in
	kangaru-config.cmake
	@ONLY
)

install(
	DIRECTORY include/kangaru ${CMAKE_CURRENT_BINARY_DIR}/generated/include/kangaru
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.hpp"
)

install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/kangaru-config.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/kangaru-config-version.cmake"
	DESTINATION ${KANGARU_INSTALL_CMAKEDIR}
)

install(TARGETS kangaru EXPORT kangaru-targets)

install(
	EXPORT kangaru-targets
	NAMESPACE kangaru::
	DESTINATION ${KANGARU_INSTALL_CMAKEDIR}
)

export(
	EXPORT kangaru-targets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/kangaru-targets.cmake"
)

install(
	EXPORT kangaru-targets FILE kangaru-targets.cmake
	DESTINATION ${KANGARU_INSTALL_CMAKEDIR}
)
