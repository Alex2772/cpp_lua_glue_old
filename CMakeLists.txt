cmake_minimum_required(VERSION 3.17)
project(cpp_lua_glue)

set(ENABLE_TESTS false CACHE BOOL "Use tests")

set(CMAKE_CXX_STANDARD 17)

add_library(cpp_lua_glue INTERFACE)
target_include_directories(cpp_lua_glue INTERFACE include)

if(NOT TARGET lua)
    file(GLOB RESULT lua)
    list(LENGTH RESULT RES_LEN)
    if(RES_LEN EQUAL 0)
        # DIR is not empty, link lua
        message(FATAL "You have not imported lua submodule! run 'git submodule update --init --recursive' in order to enable tests.")
    else()
        set(LUA_SOURCE_DIR_PATH "lua")
        add_library(lua STATIC
                "${LUA_SOURCE_DIR_PATH}/lapi.c"
                "${LUA_SOURCE_DIR_PATH}/lcode.c"
                "${LUA_SOURCE_DIR_PATH}/lctype.c"
                "${LUA_SOURCE_DIR_PATH}/ldebug.c"
                "${LUA_SOURCE_DIR_PATH}/ldo.c"
                "${LUA_SOURCE_DIR_PATH}/ldump.c"
                "${LUA_SOURCE_DIR_PATH}/lfunc.c"
                "${LUA_SOURCE_DIR_PATH}/lgc.c"
                "${LUA_SOURCE_DIR_PATH}/llex.c"
                "${LUA_SOURCE_DIR_PATH}/lmem.c"
                "${LUA_SOURCE_DIR_PATH}/lobject.c"
                "${LUA_SOURCE_DIR_PATH}/lopcodes.c"
                "${LUA_SOURCE_DIR_PATH}/lparser.c"
                "${LUA_SOURCE_DIR_PATH}/lstate.c"
                "${LUA_SOURCE_DIR_PATH}/lstring.c"
                "${LUA_SOURCE_DIR_PATH}/ltable.c"
                "${LUA_SOURCE_DIR_PATH}/ltm.c"
                "${LUA_SOURCE_DIR_PATH}/lundump.c"
                "${LUA_SOURCE_DIR_PATH}/lvm.c"
                "${LUA_SOURCE_DIR_PATH}/lzio.c"
                "${LUA_SOURCE_DIR_PATH}/lauxlib.c"
                "${LUA_SOURCE_DIR_PATH}/lbaselib.c"
                "${LUA_SOURCE_DIR_PATH}/lcorolib.c"
                "${LUA_SOURCE_DIR_PATH}/ldblib.c"
                "${LUA_SOURCE_DIR_PATH}/liolib.c"
                "${LUA_SOURCE_DIR_PATH}/lmathlib.c"
                "${LUA_SOURCE_DIR_PATH}/loadlib.c"
                "${LUA_SOURCE_DIR_PATH}/loslib.c"
                "${LUA_SOURCE_DIR_PATH}/lstrlib.c"
                "${LUA_SOURCE_DIR_PATH}/ltablib.c"
                "${LUA_SOURCE_DIR_PATH}/lutf8lib.c"
                "${LUA_SOURCE_DIR_PATH}/linit.c")
        target_include_directories(lua PUBLIC lua/)
    endif()
endif()
target_link_libraries(cpp_lua_glue INTERFACE lua)

if(ENABLE_TESTS)
    # test
    find_package(Boost 1.66.0 COMPONENTS system thread unit_test_framework REQUIRED)
    file(GLOB_RECURSE TEST_SRCS test/*.cpp)

    add_definitions(-DBOOST_ALL_NO_LIB)
    add_executable(cpp_lua_glue_test ${TEST_SRCS})
    target_include_directories(cpp_lua_glue_test PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_directories(cpp_lua_glue_test PRIVATE ${Boost_LIBRARY_DIRS})
    target_link_libraries(cpp_lua_glue_test PUBLIC ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

    target_link_libraries(cpp_lua_glue_test PUBLIC cpp_lua_glue)
    enable_testing()
    add_test(cpp_lua_glue cpp_lua_glue_test)
endif()