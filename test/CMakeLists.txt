include_directories(${${PROJECT_NAME}_SOURCE_DIR}/src ${${PROJECT_NAME}_SOURCE_DIR}/test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-parentheses")

set(TEST_FILES main-test.cpp tar_create.cpp format.cpp integration.cpp tarball.cpp ../src/tarpp/tar.h)
add_executable(${PROJECT_NAME}_tests ${TEST_FILES})
target_link_libraries(${PROJECT_NAME}_tests
        ${SDN_LIBS}
        ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
        )