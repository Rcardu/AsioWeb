project(test_app)
set(EXECUTABLE_OUTPUT_PATH ${EPATH})

add_executable(bsioser bsioser_t.cc)
target_include_directories(bsioser PRIVATE ${HPATH})
target_link_directories(bsioser PRIVATE ${LPATH})
target_link_libraries(bsioser PRIVATE ${BSIOLIB})
target_compile_definitions(bsioser PRIVATE

  C_THREAD_POOL

  # C_SERVER_POOL
)

add_executable(client1 client_t.cc)
target_include_directories(client1 PRIVATE ${HPATH})
target_link_directories(client1 PRIVATE ${LPATH})
target_link_libraries(client1 PRIVATE ${BSIOLIB})
target_compile_definitions(client1 PRIVATE # PROTO_SEND_S

  # JSON_SEND_S
  # THREAD_SEND_S
)

add_executable(bytearray bytearray_t.cc)
target_include_directories(bytearray PRIVATE ${HPATH})
target_link_directories(bytearray PRIVATE ${LPATH})
target_link_libraries(bytearray PRIVATE ${BSIOLIB})
target_compile_definitions(bytearray PRIVATE # PROTO_SEND_S
  JSON_SEND_S

  # THREAD_SEND_S
)

if(protobuf_MSVC_STATIC_RUNTIME)
  foreach(flag_var
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "-D_DLL")
      string(REGEX REPLACE "-D_DLL" "-D_STATIC" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()
endif()

if(MSVC AND protobuf_MSVC_STATIC_RUNTIME)
  foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()
endif()

add_subdirectory(protest)
add_subdirectory(jsontest)
