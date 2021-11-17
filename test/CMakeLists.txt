# test
find_package(Boost 1.66.0 COMPONENTS system thread unit_test_framework REQUIRED)

file(GLOB_RECURSE TEST_SRCS *.cpp)
add_executable(cpp_lua_glue_test ${TEST_SRCS})

add_definitions(-DBOOST_ALL_NO_LIB)

target_include_directories(cpp_lua_glue_test PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(cpp_lua_glue_test
    PUBLIC
        cpp_lua_glue
        Boost::unit_test_framework
)

enable_testing()

add_test(cpp_lua_glue cpp_lua_glue_test)