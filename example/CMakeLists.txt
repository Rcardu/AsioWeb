cmake_minimum_required(VERSION 3.15)
project(AsioSet)

set(EXECUTABLE_OUTPUT_PATH ${EPATH})

set(Boost_ROOT "D:/software/VsCode/Boost")
message(STATUS ${Boost_INCLUDE_DIRS})

add_executable(coroutine Coroutine.cc)
target_include_directories(coroutine PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(coroutine PRIVATE ${Boost_LIBRARIES})

target_compile_definitions(coroutine PRIVATE

  # 使用Clang作为前端编译器时，需要手动开启此宏使用Asio相关协程
  BOOST_ASIO_HAS_CO_AWAIT

  # 使用MSVC作为编译器时，需要手动开启此宏以启用C++20协程库
  BOOST_ASIO_HAS_STD_COROUTINE)

add_executable(ClientCoro Coroutine_Cliene.cc)
target_include_directories(ClientCoro PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(ClientCoro PRIVATE ${Boost_LIBRARIES})

target_compile_definitions(ClientCoro PRIVATE

  # 使用Clang作为前端编译器时，需要手动开启此宏使用Asio相关协程
  BOOST_ASIO_HAS_CO_AWAIT

  # 使用MSVC作为编译器时，需要手动开启此宏以启用C++20协程库
  BOOST_ASIO_HAS_STD_COROUTINE)

if(MSVC)
  foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    set(${flag_var} "${${flag_var}} /await")
  endforeach()
endif()
