# 设置运行CMake最低版本 3.15以上会自动生成CMAKE_CXX_FLAGS的值 3.14以下需要手动设置CMAKE_CXX_FLAGS
cmake_minimum_required(VERSION 3.14)
# 项目名称
project(AsioWeb)

# C++版本
set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_BUILD_TYPE DEBUG) 设置clangd索引生成
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置Windows宏
set(_WIN32_WINNT 0x601)
add_definitions(-D_WIN32_WINNT=0x0601)

# 设置变量头文件路径
set(HPATH ${CMAKE_CURRENT_SOURCE_DIR}/include)
# 设置变量库文件路径
set(LPATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
# 设置变量可执行文件路径
set(EPATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# 设置Boost库的头文件路径
set(Boost_ROOT "D:/software/VsCode/Boost")
# 查找Boost库文件
find_package(Boost 1.84.0 REQUIRED)

set(CMAKE_PREFIX_PATH "D:/software/VsCode/Cppsoft/protobuf/stage")
find_package(protobuf CONFIG REQUIRED)

set(jsoncpp_ROOT "D:/software/VsCode/Cppsoft/jsoncpp-1.9.5/destage")
set(jsoncpp_INCLUDE_DIR ${jsoncpp_ROOT}/include)
set(jsoncpp_LIBRARY_DIR ${jsoncpp_ROOT}/lib)

# set(CMAKE_MODULE_PATH${CMAKE_MODULE_PATH}
# ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules) include(basic_env)

# 开启CTest
enable_testing()

# 设置库名
set(BSIOLIB BsioWeb)

# 添加子目录
add_subdirectory(src)
add_subdirectory(test)
