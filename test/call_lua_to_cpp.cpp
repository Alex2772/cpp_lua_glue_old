
#include <boost/test/unit_test.hpp>
#include <clg/clg.hpp>

BOOST_AUTO_TEST_SUITE(call_cpp_to_lua)

BOOST_AUTO_TEST_CASE(call_return) {
    clg::vm v;
    v.do_string<void>("function get322() return 322 end");
    int x = v["get322"].call<int>();
    BOOST_CHECK_EQUAL(x, 322);
}

BOOST_AUTO_TEST_CASE(call_return_one_arg) {
    clg::vm v;
    v.do_string<void>("function plus2(a) return a + 2 end");
    int x = v["plus2"].call<int>(228);
    BOOST_CHECK_EQUAL(x, 230);
}

BOOST_AUTO_TEST_CASE(call_return_multiple_args) {
    clg::vm v;
    v.do_string<void>("function sum_plus_3(a, b) return a + b + 3 end");
    int x = v["sum_plus_3"].call<int>(228, 322);
    BOOST_CHECK_EQUAL(x, 553);
}
BOOST_AUTO_TEST_SUITE_END()