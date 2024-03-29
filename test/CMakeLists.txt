find_package(Catch2 QUIET)

add_library(kangaru_test_common INTERFACE)

target_link_libraries(kangaru_test_common INTERFACE Catch2::Catch2)
target_compile_features(kangaru_test_common INTERFACE cxx_std_11)
target_compile_definitions(kangaru_test_common INTERFACE CATCH_CONFIG_MAIN)

target_include_directories(kangaru_test_common INTERFACE $<TARGET_PROPERTY:kangaru::kangaru,INTERFACE_INCLUDE_DIRECTORIES>)

target_compile_options(kangaru_test_common INTERFACE
	$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wall -Wextra>
	$<$<CXX_COMPILER_ID:MSVC>:/W3>
)

function(add_kgr_error_test testname error_message)
	add_executable(error_${testname}_test src/error.cpp)
	target_link_libraries(error_${testname}_test PRIVATE kangaru_test_common)
	target_compile_definitions(error_${testname}_test PRIVATE kgr_error_test_${testname})
	
	set_target_properties(error_${testname}_test
	  PROPERTIES EXCLUDE_FROM_ALL true
	  EXCLUDE_FROM_DEFAULT_BUILD true
	)
	
	add_test(NAME error_${testname} COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target error_${testname}_test --config $<CONFIGURATION>)
	set_tests_properties(error_${testname} PROPERTIES PASS_REGULAR_EXPRESSION "${error_message}")
endfunction()

if (NOT Catch2_FOUND)
	add_subdirectory(third_party/Catch2)
	mark_as_advanced(Catch2_DIR)
	mark_as_advanced(CATCH_USE_VALGRIND)
	mark_as_advanced(CATCH_BUILD_TESTING)
	mark_as_advanced(CATCH_BUILD_EXAMPLES)
	mark_as_advanced(CATCH_BUILD_EXTRA_TESTS)
	mark_as_advanced(CATCH_ENABLE_COVERAGE)
	mark_as_advanced(CATCH_ENABLE_WERROR)
	mark_as_advanced(CATCH_INSTALL_DOCS)
	mark_as_advanced(CATCH_INSTALL_HELPERS)
	mark_as_advanced(BUILD_TESTING)
endif()

function(add_kgr_test TEST_NAME)
	if(${ARGC} GREATER_EQUAL 2)
		set(TEST_FILE "${ARGV1}")
	else()
		set(TEST_FILE "${TEST_NAME}.cpp")
	endif()
	
	add_executable("${TEST_NAME}_test" "src/${TEST_FILE}")
	target_link_libraries("${TEST_NAME}_test" PRIVATE kangaru_test_common)
	add_test(${TEST_NAME} "${TEST_NAME}_test")
endfunction()

if(KANGARU_TEST)
	add_kgr_test(autocall)
	add_kgr_test(autowire)
	add_kgr_test(basic)
	add_kgr_test(container)
	add_kgr_test(default_services)
	add_kgr_test(definition)
	add_kgr_test(dependency)
	add_kgr_test(invoke)
	add_kgr_test(noexcept_compiler_disabled_supplied noexcept.cpp)
	add_kgr_test(noexcept_compiler_disabled_abstract noexcept.cpp)
	add_kgr_test(noexcept_macro_disabled_supplied noexcept.cpp)
	add_kgr_test(noexcept_macro_disabled_abstract noexcept.cpp)
	add_kgr_test(operator)
	add_kgr_test(service_map)
	add_kgr_test(service_range)
	add_kgr_test(single)
	add_kgr_test(virtual)
	add_kgr_error_test(not_service_no_forward "The type sent to kgr::container::service\\(\\.\\.\\.\\) is not a service\\.")
	add_kgr_error_test(not_service_has_forward "The service type must not contain any virtual functions or virtual inheritance\\.")
endif()

target_compile_options(noexcept_compiler_disabled_supplied_test PRIVATE
	$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-fno-exceptions>
	$<$<CXX_COMPILER_ID:MSVC>:/EHs-c->
)

target_compile_options(noexcept_compiler_disabled_abstract_test PRIVATE
	$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-fno-exceptions>
	$<$<CXX_COMPILER_ID:MSVC>:/EHs-c->
)

target_compile_definitions(noexcept_macro_disabled_supplied_test PRIVATE KGR_KANGARU_NOEXCEPTION KGR_KANGARU_TEST_SUPPLIED_ABORT)
target_compile_definitions(noexcept_macro_disabled_abstract_test PRIVATE KGR_KANGARU_NOEXCEPTION KGR_KANGARU_TEST_ABSTRACT_ABORT)
target_compile_definitions(noexcept_compiler_disabled_supplied_test PRIVATE KGR_KANGARU_TEST_SUPPLIED_ABORT)
target_compile_definitions(noexcept_compiler_disabled_abstract_test PRIVATE KGR_KANGARU_TEST_ABSTRACT_ABORT)

if(KANGARU_TEST_CXX14)
	add_kgr_test(generic_lambdas)
	set_property(TARGET generic_lambdas_test PROPERTY CXX_STANDARD 14)
	target_compile_features(generic_lambdas_test PRIVATE cxx_std_14)
endif()

if(KANGARU_TEST_CXX14 AND KANGARU_HASH_TYPE_ID)
	add_kgr_test(type_id_cxx14)
	set_property(TARGET type_id_cxx14_test PROPERTY CXX_STANDARD 14)
	target_compile_features(type_id_cxx14_test PRIVATE cxx_std_14)
endif()

if(KANGARU_TEST_CXX17)
	add_kgr_test(noexcept_invoke)
	set_property(TARGET noexcept_invoke_test PROPERTY CXX_STANDARD 17)
	target_compile_features(noexcept_invoke_test PRIVATE cxx_std_17)
endif()
