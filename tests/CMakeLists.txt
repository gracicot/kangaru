find_package(Catch2 REQUIRED)
find_package(fmt REQUIRED)

add_library(kangaru_test_common INTERFACE)

target_link_libraries(kangaru_test_common INTERFACE kangaru::kangaru Catch2::Catch2 Catch2::Catch2WithMain fmt::fmt)

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
	add_kgr_test(5-cache-source-heap-storage)
	add_kgr_test(5-reference-deducer)
	add_kgr_test(5-container)
	add_kgr_test(5-ctti)
	add_kgr_test(5-deducer-source)
	add_kgr_test(5-compose-injector)
	add_kgr_test(5-recursive-source)
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

add_kgr_test(generic_lambdas)
set_property(TARGET generic_lambdas_test PROPERTY CXX_STANDARD 14)
target_compile_features(generic_lambdas_test PRIVATE cxx_std_14)

add_kgr_test(type_id_cxx14)
set_property(TARGET type_id_cxx14_test PROPERTY CXX_STANDARD 14)
target_compile_features(type_id_cxx14_test PRIVATE cxx_std_14)

add_kgr_test(noexcept_invoke)
set_property(TARGET noexcept_invoke_test PROPERTY CXX_STANDARD 17)
target_compile_features(noexcept_invoke_test PRIVATE cxx_std_17)
