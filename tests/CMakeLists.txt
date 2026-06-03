find_package(Catch2 REQUIRED)
find_package(fmt REQUIRED)

add_library(kangaru_test_common INTERFACE)

target_link_libraries(kangaru_test_common INTERFACE kangaru::kangaru Catch2::Catch2 Catch2::Catch2WithMain fmt::fmt)

function(kangaru_add_compilation_failure testname error_message)
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

function(kangaru_add_test TEST_NAME)
	if(${ARGC} GREATER_EQUAL 2)
		set(TEST_FILE "${ARGV1}")
	else()
		set(TEST_FILE "${TEST_NAME}.cpp")
	endif()
	
	add_executable("${TEST_NAME}_test" "src/${TEST_FILE}")
	target_link_libraries("${TEST_NAME}_test" PRIVATE kangaru_test_common)
	add_test(${TEST_NAME} "${TEST_NAME}_test")
endfunction()

function(kangaru_add_template_test TEST_NAME TEST_TYPE)
	set(TEST_FILE "${TEST_NAME}.cpp")
	
	add_executable("${TEST_NAME}_${TEST_TYPE}_test" "src/${TEST_FILE}")
	target_link_libraries("${TEST_NAME}_${TEST_TYPE}_test" PRIVATE kangaru_test_common)
	target_compile_definitions("${TEST_NAME}_${TEST_TYPE}_test" PRIVATE KANGARU5_TEST_TYPE=${TEST_TYPE})
	add_test("${TEST_NAME}_${TEST_TYPE}" "${TEST_NAME}_${TEST_TYPE}_test")
endfunction()

if(KANGARU_TEST)
	kangaru_add_test(5-allocator)
	kangaru_add_test(5-any-source-of)
	kangaru_add_test(5-attributes)
	kangaru_add_test(5-cache-source-heap-storage)
	kangaru_add_test(5-concepts)
	kangaru_add_test(5-reference-deducer)
	kangaru_add_test(5-container)
	kangaru_add_template_test(5-container-provide provide_test_self)
	kangaru_add_template_test(5-container-provide provide_test_polymorphic_self)
	kangaru_add_template_test(5-container-provide provide_test_mapped_ref)
	kangaru_add_template_test(5-container-provide provide_test_polymorphic_mapped_ref)
	kangaru_add_template_test(5-container-provide provide_test_mapped_value_base_object_source)
	kangaru_add_template_test(5-container-provide provide_test_polymorphic_mapped_value_base_object_source)
	kangaru_add_template_test(5-container-provide provide_test_mapped_value_base_object_source_source)
	kangaru_add_template_test(5-container-provide provide_test_polymorphic_mapped_value_base_object_source_source)
	kangaru_add_template_test(5-container-provide provide_test_mapped_value_base_reference_source)
	kangaru_add_template_test(5-container-provide provide_test_polymorphic_mapped_value_base_reference_source)
	kangaru_add_template_test(5-container-provide provide_test_mapped_value_base_reference_source_source)
	kangaru_add_template_test(5-container-provide provide_test_polymorphic_mapped_value_base_reference_source_source)
	kangaru_add_template_test(5-container-provide provide_test_mapped_has_empty)
	kangaru_add_template_test(5-container-provide provide_test_polymorphic_mapped_has_empty)
	kangaru_add_template_test(5-container-provide provide_test_unmapped_has_empty)
	kangaru_add_template_test(5-container-provide provide_test_polymorphic_unmapped_has_empty)
	kangaru_add_template_test(5-container-provide provide_test_empty_injectable)
	kangaru_add_template_test(5-container-provide provide_test_polymorphic_empty_injectable)
	kangaru_add_test(5-constructor)
	if(KANGARU_HAS_CPP_MODULES)
		kangaru_add_test(5-modules)
	endif()
	kangaru_add_test(5-deducer-source)
	kangaru_add_test(5-compose-injector)
	kangaru_add_test(5-recursive-source)
	kangaru_add_test(5-compile-time)
	kangaru_add_test(5-playground)
	kangaru_add_test(5-modular-source)
	kangaru_add_test(5-two-step-init)
	kangaru_add_test(5-type-id)
	kangaru_add_test(autocall)
	kangaru_add_test(autowire)
	kangaru_add_test(basic)
	kangaru_add_test(container)
	kangaru_add_test(default_services)
	kangaru_add_test(definition)
	kangaru_add_test(dependency)
	kangaru_add_test(invoke)
	kangaru_add_test(noexcept_compiler_disabled_supplied noexcept.cpp)
	kangaru_add_test(noexcept_compiler_disabled_abstract noexcept.cpp)
	kangaru_add_test(noexcept_macro_disabled_supplied noexcept.cpp)
	kangaru_add_test(noexcept_macro_disabled_abstract noexcept.cpp)
	kangaru_add_test(operator)
	kangaru_add_test(service_map)
	kangaru_add_test(service_range)
	kangaru_add_test(single)
	kangaru_add_test(virtual)
	kangaru_add_compilation_failure(not_service_no_forward "The type sent to kgr::container::service\\(\\.\\.\\.\\) is not a service\\.")
	kangaru_add_compilation_failure(not_service_has_forward "The service type must not contain any virtual functions or virtual inheritance\\.")
endif()

target_compile_options(noexcept_compiler_disabled_supplied_test PRIVATE
	$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-fno-exceptions>
	$<$<CXX_COMPILER_ID:MSVC>:/EHs-c->
)

target_compile_options(noexcept_compiler_disabled_abstract_test PRIVATE
	$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-fno-exceptions>
	$<$<CXX_COMPILER_ID:MSVC>:/EHs-c->
)

target_compile_definitions(noexcept_compiler_disabled_supplied_test PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:_HAS_EXCEPTIONS=0>
)

target_compile_definitions(noexcept_compiler_disabled_abstract_test PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:_HAS_EXCEPTIONS=0>
)

target_compile_definitions(noexcept_macro_disabled_supplied_test PRIVATE KGR_KANGARU_NOEXCEPTION KGR_KANGARU_TEST_SUPPLIED_ABORT)
target_compile_definitions(noexcept_macro_disabled_abstract_test PRIVATE KGR_KANGARU_NOEXCEPTION KGR_KANGARU_TEST_ABSTRACT_ABORT)
target_compile_definitions(noexcept_compiler_disabled_supplied_test PRIVATE KGR_KANGARU_TEST_SUPPLIED_ABORT)
target_compile_definitions(noexcept_compiler_disabled_abstract_test PRIVATE KGR_KANGARU_TEST_ABSTRACT_ABORT)

kangaru_add_test(generic_lambdas)
kangaru_add_test(type_id_cxx14)
kangaru_add_test(noexcept_invoke)
