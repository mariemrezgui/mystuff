cmake_minimum_required(VERSION 3.0)
project(myshell C)
# Warning: this makefile is slightly different than the one provided to the students!

add_compile_options(
    -O0
    -g
    -pedantic
    -Wall
    -Wextra
    -Werror
    -fcolor-diagnostics
)

# take all .c files from src folder and add them to the SOURCE_FILE variable
file(GLOB SOURCE_FILES src/*.c)

# take all .h files from include folder and add them to the HEADER_FILES variable
file(GLOB HEADER_FILES include/*.h)

# exclude main.c from SOURCE_FILES
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")

include_directories(include)
add_library(myshellObj OBJECT ${SOURCE_FILES} ${HEADER_FILES})
add_executable(myshell  $<TARGET_OBJECTS:myshellObj> src/main.c)

# collect all ressources
file(GLOB RESSOURCES ressources/*)

# add ressources
add_custom_target(myshell_ressources SOURCES
    ${RESSOURCES}
)

# copy ressources into build folder
#add_custom_command(TARGET myshell
#    POST_BUILD
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#    COMMAND cp ${RESSOURCES} ${CMAKE_BINARY_DIR}
#)
