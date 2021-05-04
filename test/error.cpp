
#include <boost/test/unit_test.hpp>
#include <clg/clg.hpp>

BOOST_AUTO_TEST_SUITE(error)

BOOST_AUTO_TEST_CASE(do_string) {
    clg::vm v;
    BOOST_CHECK_NO_THROW(v.do_string<int>("return 4 + 2"));
    BOOST_CHECK_THROW(v.do_string<int>("retudrn 4 + 2"), clg::lua_exception);
    BOOST_CHECK_THROW(v.do_string<void>("retudrn 4 + 2"), clg::lua_exception);
}


void func(int a, int b) {
    BOOST_FAIL("the function was called");
}

BOOST_AUTO_TEST_CASE(invalid_argument_count1) {
    clg::vm v;
    v.register_function<func>("func");
    BOOST_CHECK_THROW(v.do_string<void>("func(1, 2, 3)"), clg::clg_exception);
}


BOOST_AUTO_TEST_CASE(invalid_argument_count2) {
    clg::vm v;
    v.register_function<func>("func");
    BOOST_CHECK_THROW(v.do_string<void>("func(1)"), clg::clg_exception);
}

BOOST_AUTO_TEST_SUITE_END()