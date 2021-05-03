cmake_minimum_required(VERSION 3.19)
project(cpp_lua_glue)

set(CMAKE_CXX_STANDARD 14)

add_library(cpp_lua_glue INTERFACE)
target_include_directories(cpp_lua_glue INTERFACE include)