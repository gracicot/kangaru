find_package(Catch2 QUIET)

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
	target_link_libraries("${TEST_NAME}_test" PUBLIC Catch2::Catch2 kangaru::kangaru)
	target_compile_features("${TEST_NAME}_test" PRIVATE cxx_std_11)
	target_compile_definitions("${TEST_NAME}_test" PRIVATE CATCH_CONFIG_MAIN)
	
	add_test(${TEST_NAME} "${TEST_NAME}_test")
endfunction()

if(${KANGARU_TEST})
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
	add_kgr_test(single)
	add_kgr_test(virtual)
endif()

target_compile_options(noexcept_compiler_disabled_supplied_test PRIVATE
	$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fno-exceptions>
	$<$<CXX_COMPILER_ID:MSVC>:/EHs-c->
)

target_compile_options(noexcept_compiler_disabled_abstract_test PRIVATE
	$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fno-exceptions>
	$<$<CXX_COMPILER_ID:MSVC>:/EHs-c->
)

target_compile_definitions(noexcept_macro_disabled_supplied_test PRIVATE KGR_KANGARU_NOEXCEPTION KGR_KANGARU_TEST_SUPPLIED_ABORT)
target_compile_definitions(noexcept_macro_disabled_abstract_test PRIVATE KGR_KANGARU_NOEXCEPTION KGR_KANGARU_TEST_ABSTRACT_ABORT)
target_compile_definitions(noexcept_compiler_disabled_supplied_test PRIVATE KGR_KANGARU_TEST_SUPPLIED_ABORT)
target_compile_definitions(noexcept_compiler_disabled_abstract_test PRIVATE KGR_KANGARU_TEST_ABSTRACT_ABORT)

if(${KANGARU_TEST_CXX14})
	add_kgr_test(generic_lambdas)
	set_property(TARGET generic_lambdas_test PROPERTY CXX_STANDARD 14)
	target_compile_features(generic_lambdas_test PRIVATE cxx_std_14)
endif()

if(${KANGARU_TEST_CXX17})
	add_kgr_test(noexcept_invoke)
	set_property(TARGET noexcept_invoke_test PROPERTY CXX_STANDARD 17)
	target_compile_features(noexcept_invoke_test PRIVATE cxx_std_17)
endif()
