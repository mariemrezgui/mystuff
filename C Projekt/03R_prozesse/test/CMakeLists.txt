cmake_minimum_required(VERSION 3.00)

project(test LANGUAGES "C")

set(MYSHELL_SOURCES
  ../myshell/src/plist.c
  ../myshell/src/shellutils.c
  ../myshell/src/shellfunctions.c)

add_executable(myshell_test "")

target_sources(myshell_test
  PRIVATE
  myshell_test.c
  ${MYSHELL_SOURCES}
  )

target_include_directories(myshell_test
  PRIVATE
  ../myshell/include)

install(TARGETS myshell_test EXPORT myshell_test_targets
  RUNTIME DESTINATION ${CMAKE_BINARY_DIR}/TESTING
  )

# export(EXPORT myshell_test_targets
#   FILE ${CMAKE_BINARY_DIR}/myshell_test_targets.cmake)
