project(test_proto)
set(EXECUTABLE_OUTPUT_PATH ${EPATH})

include_directories(${HPATH})
foreach(example ptrotc_t)
  set(${example}_SRCS ${example}.cc)
  set(${example}_PROTOS msg.proto)

  # Code Generation
  if(protobuf_MODULE_COMPATIBLE) # Legacy Support
    protobuf_generate_cpp(${example}_PROTO_SRCS ${example}_PROTO_HDRS
                          ${${example}_PROTOS})
    list(APPEND ${example}_SRCS ${${example}_PROTO_SRCS}
         ${${example}_PROTO_HDRS})
  endif()

  # Executable setup
  set(executable_name ${example}_cpp)
  add_executable(${executable_name} ${${example}_SRCS} ${${example}_PROTOS})
  if(protobuf_MODULE_COMPATIBLE) # Legacy mode
    target_include_directories(${executable_name}
                               PUBLIC ${PROTOBUF_INCLUDE_DIRS})
    target_link_libraries(${executable_name} ${PROTOBUF_LIBRARIES})
  else()
    target_link_libraries(${executable_name} protobuf::libprotobuf)
    protobuf_generate(TARGET ${executable_name})
  endif()

endforeach()
