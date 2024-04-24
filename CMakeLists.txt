cmake_minimum_required(VERSION 3.25)
project(AsioWeb)

set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(HPATH ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LPATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(EPATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_definitions(-D_WIN32_WINNT=0x0601)

# 设置Boost库的头文件路径
set(Boost_INCLUDE_DIR "D:/software/VsCode/Boost")
# 设置Boost库的库文件路径
set(Boost_LIBRARY_DIR "D:/software/VsCode/Boost/stage/lib")
# 查找Boost库文件
find_package(Boost 1.84.0 REQUIRED)
if(Boost_FOUND)
  message(Boost\ include\ dir: ${Boost_LIB_VERSION})
  message(Boost\ include\ dir: ${Boost_INCLUDE_DIRS})
  message(Boost\ library\ dir: ${Boost_LIBRARY_DIRS})
endif()
enable_testing()

set(BSIOLIB BsioWeb)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)

add_subdirectory(src)
add_subdirectory(test)
