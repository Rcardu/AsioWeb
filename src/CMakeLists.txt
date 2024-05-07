set(LIBRARY_OUTPUT_PATH ${LPATH})

foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                 CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
  if(${flag_var} MATCHES "-D_DLL")
    string(REGEX REPLACE "-D_DLL" "-D_STATIC" ${flag_var} "${${flag_var}}")
  endif()
endforeach()

aux_source_directory(./ SRC)
set(msgproto msg.proto)
add_library(${BSIOLIB} STATIC ${SRC} ${msgproto})

target_include_directories(${BSIOLIB} PUBLIC ${HPATH} ${Boost_INCLUDE_DIRS}
                                             ${jsoncpp_INCLUDE_DIR})
target_link_directories(${BSIOLIB} PUBLIC ${Boost_LIBRARY_DIRS}
                        ${jsoncpp_LIBRARY_DIR})
target_link_libraries(${BSIOLIB} PUBLIC ${Boost_LIBRARIES}
                                        protobuf::libprotobuf jsoncpp)
target_compile_definitions(
  ${BSIOLIB} PUBLIC 
  #THREAD_SEND_S 
  #PROTO_SEND_S
  #JSON_SEND_S
  #READ_HEAD_MSG
  READ_ALL_MSG 
  )
protobuf_generate(TARGET ${BSIOLIB})

# add_executable(SRC1 msg.pb.cc)
