cmake_minimum_required(VERSION 3.5)
project(tar)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wno-missing-braces -fno-omit-frame-pointer -fsanitize=address")

install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/" DESTINATION "include"
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp")

add_subdirectory(test)
enable_testing()
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME}_tests)