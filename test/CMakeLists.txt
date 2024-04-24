set(EXECUTABLE_OUTPUT_PATH ${EPATH})

include_directories(${Boost_INCLUDE_DIR})
include_directories(${HPATH})

link_directories(${LPATH})
add_executable(bsioser bsioser_t.cc)
target_link_libraries(bsioser ${BSIOLIB})

add_executable(client1 client_t.cc)
