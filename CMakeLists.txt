cmake_minimum_required(VERSION 3.31)

project(kangaru VERSION 5.0.0 LANGUAGES CXX)

option(KANGARU_BUILD_EXAMPLES "Build kangaru examples" OFF)
option(KANGARU_TEST "Build test binaries" ON)

# Variable to track module support
set(KANGARU_HAS_CPP_MODULES OFF)
mark_as_advanced(KANGARU_HAS_CPP_MODULES)

# Check compiler and version
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19)
		set(KANGARU_HAS_CPP_MODULES ON)
	endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
	set(KANGARU_HAS_CPP_MODULES OFF)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16)
		set(KANGARU_HAS_CPP_MODULES ON)
	endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	if(MSVC_VERSION GREATER_EQUAL 1942)
		set(KANGARU_HAS_CPP_MODULES ON)
	endif()
endif()

if(KANGARU_HAS_CPP_MODULES)
	add_library(kangaru)
else()
	add_library(kangaru INTERFACE)
endif()

add_library(kangaru::kangaru ALIAS kangaru)

target_sources(kangaru
PUBLIC FILE_SET HEADERS
	BASE_DIRS
		include
	FILES
		include/kangaru/kangaru.hpp
		include/kangaru/short.hpp
		include/kangaru/detail/allocator.hpp
		include/kangaru/detail/any_source_of.hpp
		include/kangaru/detail/attributes.hpp
		include/kangaru/detail/cache.hpp
		include/kangaru/detail/cache_types.hpp
		include/kangaru/detail/concepts.hpp
		include/kangaru/detail/constructor.hpp
		include/kangaru/detail/container.hpp
		include/kangaru/detail/container_common.hpp
		include/kangaru/detail/ctti.hpp
		include/kangaru/detail/deducer.hpp
		include/kangaru/detail/define.hpp
		include/kangaru/detail/exceptions.hpp
		include/kangaru/detail/heap_storage.hpp
		include/kangaru/detail/injector.hpp
		include/kangaru/detail/lazy.hpp
		include/kangaru/detail/murmur.hpp
		include/kangaru/detail/optional.hpp
		include/kangaru/detail/polymorphic_container.hpp
		include/kangaru/detail/polymorphic_source.hpp
		include/kangaru/detail/recursive_source.hpp
		include/kangaru/detail/source.hpp
		include/kangaru/detail/source_rebind.hpp
		include/kangaru/detail/source_reference_wrapper.hpp
		include/kangaru/detail/source_traits.hpp
		include/kangaru/detail/source_types.hpp
		include/kangaru/detail/type_traits.hpp
		include/kangaru/detail/undef.hpp
		include/kangaru/detail/utility.hpp
)

if(KANGARU_HAS_CPP_MODULES)
	target_sources(kangaru
		PUBLIC FILE_SET CXX_MODULES
			FILES
				src/kangaru.cpp
	)
	target_compile_features(kangaru PUBLIC cxx_std_20)
	target_compile_options(kangaru PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
endif()

target_compile_features(kangaru INTERFACE cxx_std_20)
target_include_directories(kangaru INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated/include>
)

if(KANGARU_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

if(KANGARU_TEST)
	enable_testing()
	add_subdirectory(tests)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(KANGARU_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/kangaru")
mark_as_advanced(KANGARU_INSTALL_CMAKEDIR)

if(KANGARU_HAS_CPP_MODULES)
	write_basic_package_version_file(
		"${CMAKE_CURRENT_BINARY_DIR}/kangaru-config-version.cmake"
		COMPATIBILITY SameMajorVersion
	)
	
	install(TARGETS kangaru
		EXPORT kangaru-config
		FILE_SET HEADERS
		FILE_SET CXX_MODULES
			DESTINATION ${KANGARU_INSTALL_CMAKEDIR}/cxx-modules
	)
else()
	write_basic_package_version_file(
		"${CMAKE_CURRENT_BINARY_DIR}/kangaru-config-version.cmake"
		COMPATIBILITY SameMajorVersion
		ARCH_INDEPENDENT
	)
	
	install(TARGETS kangaru
		EXPORT kangaru-config
		FILE_SET HEADERS
	)
endif()

install(
	EXPORT kangaru-config
	NAMESPACE kangaru::
	DESTINATION ${KANGARU_INSTALL_CMAKEDIR}
)

export(
	EXPORT kangaru-config
	NAMESPACE kangaru::
	FILE "${CMAKE_CURRENT_BINARY_DIR}/kangaru-config.cmake"
)

