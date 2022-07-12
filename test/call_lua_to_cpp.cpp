
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
    try {
        v.do_string<void>("call()");
        BOOST_FAIL("do_string has not thrown an exception");
    } catch (...) {
        auto e =std::current_exception();
        std::rethrow_exception(e);
    }
}


void func_call_invalid_args(int f, double d) {
    BOOST_CHECK_EQUAL(f, 16);
    BOOST_CHECK_CLOSE(d, 13, 0.01);
}

BOOST_AUTO_TEST_CASE(call_invalid_args) {
    clg::vm v;
    v.register_function<func_call_invalid_args>("call");

    v.do_string<void>("call(16.1, 13)");
}

void check228(int v) {
    called = true;
    BOOST_CHECK_EQUAL(v, 228);
}

BOOST_AUTO_TEST_CASE(lambda) {
    clg::vm v;
    called = false;
    v.register_function("sum", [](int a, int b) {
        return a + b;
    });
    v.register_function<check228>("check228");
    v.do_string<void>("check228(sum(227, 1))");

    BOOST_TEST(called);
}

BOOST_AUTO_TEST_CASE(lambda_with_capture) {
    clg::vm v;
    called = false;
    int s = 0;
    v.register_function("sum", [&](int a, int b) {
        return s = (a + b);
    });
    v.register_function<check228>("check228");
    v.do_string<void>("check228(sum(227, 1))");

    BOOST_CHECK_EQUAL(s, 228);
    BOOST_TEST(called);
}
BOOST_AUTO_TEST_CASE(vararg) {
    clg::vm v;
    called = false;
    v.register_function("call", [&](clg::vararg args) {
        called = true;
        BOOST_CHECK_EQUAL(args.size(), 3);
        BOOST_CHECK_EQUAL(args[0].as<int>(), 0);
        BOOST_CHECK_EQUAL(args[1].as<int>(), 1);
        BOOST_CHECK_EQUAL(args[2].as<std::string>(), "hello");
    });
    v.do_string("call(0, 1, 'hello')");

    BOOST_TEST(called);
}

BOOST_AUTO_TEST_CASE(overloading1) {
    clg::vm v;

    bool called1 = false;
    bool called2 = false;

    v.register_function_overloaded("call", [&](int v1) {
        BOOST_TEST(!called1);
        BOOST_TEST(!called2);
        called1 = true;
        BOOST_CHECK_EQUAL(v1, 123);
    }, [&](int v1, int v2) {
        BOOST_TEST(called1);
        BOOST_TEST(!called2);
        called2 = true;

        BOOST_CHECK_EQUAL(v1, 456);
        BOOST_CHECK_EQUAL(v2, 789);
    });

    v.do_string(R"(
call(123)
call(456, 789)
)");

    BOOST_TEST(called1);
    BOOST_TEST(called2);
}



BOOST_AUTO_TEST_SUITE_END()