add_library(kangaru-example INTERFACE)

target_compile_options(kangaru-example INTERFACE
	$<$<CXX_COMPILER_ID:Clang>:-Werror -Wall -Wextra -Wno-unused-parameter -Wno-unused-private-field -Wno-unused-variable>
	$<$<CXX_COMPILER_ID:GNU>:-Werror -Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-comment>
)

target_link_libraries(kangaru-example INTERFACE kangaru)

add_subdirectory(example1)
add_subdirectory(example2)
add_subdirectory(example3)
add_subdirectory(example4)
add_subdirectory(example5)
add_subdirectory(example6)
add_subdirectory(example7)
add_subdirectory(example8)
add_subdirectory(example9)
add_subdirectory(readme_example)
add_subdirectory(wandbox_example)
