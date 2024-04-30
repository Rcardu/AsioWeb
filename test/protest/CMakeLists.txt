project(test_proto)
set(EXECUTABLE_OUTPUT_PATH ${EPATH})

include_directories(${HPATH})
# include_directories(${Protobuf_PATH})
# http://www.cmake.org/Wiki/CMake_FAQ#How_can_I_build_my_MSVC_application_with_a_static_runtime.3F
# if(protobuf_MSVC_STATIC_RUNTIME) foreach(flag_var CMAKE_CXX_FLAGS
# CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL
# CMAKE_CXX_FLAGS_RELWITHDEBINFO) if(${flag_var} MATCHES "-D_DLL") string(REGEX
# REPLACE "-D_DLL" "-D_STATIC" ${flag_var} "${${flag_var}}") endif()
# endforeach() endif()
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
  # set_target_properties(${executable_name} PROPERTIES COMPILE_OPTIONS "/MTd")
  # target_compile_definitions(${executable_name} PRIVATE _MT)
  if(protobuf_MODULE_COMPATIBLE) # Legacy mode
    target_include_directories(${executable_name}
                               PUBLIC ${PROTOBUF_INCLUDE_DIRS})
    target_link_libraries(${executable_name} ${PROTOBUF_LIBRARIES})
  else()
    target_link_libraries(${executable_name} protobuf::libprotobuf)
    protobuf_generate(TARGET ${executable_name})
  endif()

endforeach()
