enable_testing()

find_package(catch REQUIRED)

function(add_kgr_test TEST_NAME)
	add_executable("${TEST_NAME}_test" "src/${TEST_NAME}.cpp")
	target_link_libraries("${TEST_NAME}_test" PUBLIC catch kangaru)

	add_test(${TEST_NAME} "${TEST_NAME}_test")
endfunction()

add_kgr_test(basic)
add_kgr_test(dependency)
add_kgr_test(single)
add_kgr_test(virtual)
