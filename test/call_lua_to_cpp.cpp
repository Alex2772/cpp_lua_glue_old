
#include <boost/test/unit_test.hpp>
#include <clg/clg.hpp>

BOOST_AUTO_TEST_SUITE(call_lua_to_cpp)


bool called;
void func_call() {
    called = true;
}

BOOST_AUTO_TEST_CASE(call) {
    clg::vm v;
    v.register_function<func_call>("call");
    v.do_string<void>("call()");

    BOOST_TEST(called);
}

int func_call_return() {
    return 123;
}

BOOST_AUTO_TEST_CASE(call_return) {
    clg::vm v;
    v.register_function<func_call_return>("call");
    int result = v.do_string<int>("return call()");

    BOOST_CHECK_EQUAL(result, 123);
}

int func_call_return_args1(int a, int b) {
    return a + b;
}

BOOST_AUTO_TEST_CASE(call_return_args1) {
    clg::vm v;
    v.register_function<func_call_return_args1>("call");
    int result = v.do_string<int>("return call(3, 5)");

    BOOST_CHECK_EQUAL(result, 8);
}

int func_call_return_args2(int a, int b) {
    return a - b;
}

BOOST_AUTO_TEST_CASE(call_return_args2) {
    clg::vm v;
    v.register_function<func_call_return_args2>("call");
    int result = v.do_string<int>("return call(12, 3)");

    BOOST_CHECK_EQUAL(result, 9);
}

std::string func_call_return_args3(const std::string& hello) {
    return hello + " world";
}

BOOST_AUTO_TEST_CASE(call_return_args3) {
    clg::vm v;
    v.register_function<func_call_return_args3>("call");
    std::string result = v.do_string<std::string>("return call(\"hello\")");

    BOOST_CHECK_EQUAL(result, "hello world");
}

std::string func_call_return_args4_a(const std::string& hello) {
    return hello + " world";
}
std::string func_call_return_args4_b(const std::string& hello) {
    return ", " + hello + "!";
}

BOOST_AUTO_TEST_CASE(call_multiple_functions) {
    clg::vm v;
    v.register_function<func_call_return_args4_a>("call_a");
    v.register_function<func_call_return_args4_b>("call_b");
    std::string result = v.do_string<std::string>("return call_a(\"hello\")..call_b(\"loshara\")");

    BOOST_CHECK_EQUAL(result, "hello world, loshara!");
}


std::tuple<int, int> func_return_1_2() {
    return { 1, 2 };
}

BOOST_AUTO_TEST_CASE(call_return_multiple) {
    clg::vm v;
    v.register_function<func_return_1_2>("call");
    std::string result = v.do_string<std::string>(
            "local a,b = call()\n"
            "return a..b"
            );


    BOOST_CHECK_EQUAL(result, "12");
}

void func_call_many_args1(int v1, int v2, int v3, int v4, int v5, int v6, int v7, int v8) {
    BOOST_CHECK_EQUAL(v1, 1);
    BOOST_CHECK_EQUAL(v2, 2);
    BOOST_CHECK_EQUAL(v3, 3);
    BOOST_CHECK_EQUAL(v4, 4);
    BOOST_CHECK_EQUAL(v5, 5);
    BOOST_CHECK_EQUAL(v6, 6);
    BOOST_CHECK_EQUAL(v7, 7);
    BOOST_CHECK_EQUAL(v8, 8);
}

BOOST_AUTO_TEST_CASE(call_many_args1) {
    clg::vm v;
    v.register_function<func_call_many_args1>("call");
    v.do_string<void>("call(1,2,3,4,5,6,7,8)");
}

void func_exception() {
    throw std::runtime_error("ti loh");
}

BOOST_AUTO_TEST_CASE(call_exception) {
    clg::vm v;
    v.register_function<func_exception>("call");
    BOOST_CHECK_THROW(v.do_string<void>("call()"), clg::lua_exception);
    try {
        v.do_string<void>("call()");
    } catch (const std::exception& e) {
        std::cout << "exception: " << e.what() << std::endl;
    }
}


void func_call_many_args2(int v1, int v2, const char* v3, int v4, int v5, int v6, int v7, int v8) {
    BOOST_CHECK_EQUAL(v1, 60);
    BOOST_CHECK_EQUAL(v2, 6);
    BOOST_CHECK_EQUAL(std::string(v3), "texture/ui/life.png");
    BOOST_CHECK_EQUAL(v4, 60);
    BOOST_CHECK_EQUAL(v5, 5);
    BOOST_CHECK_EQUAL(v6, 0);
    BOOST_CHECK_EQUAL(v7, 0);
    BOOST_CHECK_EQUAL(v8, 1);
}

BOOST_AUTO_TEST_CASE(call_many_args2) {
    clg::vm v;
    v.register_function<func_call_many_args2>("call");
    //v.do_string<void>("TRUE = 1\ncall(1,2,'loh',4,5,6,7,8,TRUE)");

    v.do_string<void>("call( 60          ,6,     \"texture/ui/life.png\", 60, 5       , 0, 0, true )");
}


BOOST_AUTO_TEST_SUITE_END()