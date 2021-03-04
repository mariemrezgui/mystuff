cmake_minimum_required(VERSION 3.00)

# must be defined before first project command
# otherwise an infinite loop will ensue
set(CMAKE_C_COMPILER "/usr/bin/clang")

project(3R-Material LANGUAGES "C")

add_subdirectory(myshell ${CMAKE_BINARY_DIR}/shell)
add_subdirectory(test)
add_custom_target(merge_builds ALL)

add_custom_command(TARGET merge_builds
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:myshell> ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:myshell_test> ${CMAKE_BINARY_DIR}
)
