project(test_json)
set(EXECUTABLE_OUTPUT_PATH ${EPATH})

set(jsnocpp_INCLUDE_PATH ${jsoncpp_ROOT}/include)
set(jsnocpp_LIBRARY_PATH ${jsoncpp_ROOT}/lib)

add_executable(jsontest jsontest_t.cc)
target_include_directories(jsontest PRIVATE ${jsnocpp_INCLUDE_PATH})
target_link_directories(jsontest PRIVATE ${jsnocpp_LIBRARY_PATH})
target_link_libraries(jsontest jsoncpp)
